#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITANetAudioMessage.h>

// ITA includes
#include <ITADataSource.h>
#include <ITANetAudioMessage.h>
#include <ITAException.h>
#include <ITAStreamInfo.h>

// Vista includes
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>
#include <VistaBase/VistaStreamUtils.h>

// STL
#include <cmath>
#include <cassert>

CITANetAudioStreamingServer::CITANetAudioStreamingServer( )
: m_pInputStream( NULL )
, m_iUpdateStrategy( AUTO )
, m_pConnection( NULL )
, m_pNetAudioServer( new CITANetAudioServer( ) )
{
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort )
{
	if ( !m_pInputStream )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not start server without a valid input stream" );

	if ( !m_pNetAudioServer->Start( sAddress, iPort ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection( );

	m_pMessage = new CITANetAudioMessage( m_pConnection->GetByteorderSwapFlag( ) );
	m_pMessage->ResetMessage( );
	m_pMessage->SetConnection( m_pConnection );
	while ( !m_pMessage->ReadMessage( ) ); //blocking

	assert( m_pMessage->GetMessageType( ) == CITANetAudioProtocol::NP_CLIENT_OPEN );
	CITANetAudioProtocol::StreamingParameters oClientParams = m_pMessage->ReadStreamingParameters( );

	bool bOK = false;
	m_oServerParams.iBufferSize = oClientParams.iBufferSize;
	if ( m_oServerParams == oClientParams )
	{
		bOK = true;
#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters matched. Will resume with streaming" << std::endl;
	}
	else
	{
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters mismatch detected. Will notify client and stop." << std::endl;
#endif
	}

	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_OPEN );
	m_pMessage->WriteBool( bOK );
	m_pMessage->WriteMessage( );

	if ( bOK )
		Run( );

	return bOK;
}

bool CITANetAudioStreamingServer::LoopBody( )
{
	m_pMessage->ResetMessage( );
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->ReadMessage( );

	if ( m_pMessage->ReadMessage( ) )
	{
		int iMsgType = m_pMessage->GetMessageType( );
		switch ( iMsgType )
		{
			case CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES:
			{
					int iFreeSamples = m_pMessage->ReadInt( );
					if ( iFreeSamples >= int( m_pInputStream->GetBlocklength( ) ) )
					{
						// Send Samples
						for ( int i = 0; i < int( m_pInputStream->GetNumberOfChannels( ) ); i++ )
						{
							ITAStreamInfo oStreamInfo;
							oStreamInfo.nSamples = m_sfTempTransmitBuffer.GetLength( );
							const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
							if ( pfData != 0 )
							m_sfTempTransmitBuffer[ i ].write( pfData, m_sfTempTransmitBuffer.GetLength( ) );
						}
					m_pInputStream->IncrementBlockPointer( );
					m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES );
					m_pMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
					m_pMessage->WriteMessage( );

#ifdef NET_AUDIO_SHOW_TRAFFIC
					vstr::out() << "[ITANetAudioStreamingServer] Transmitted "<< m_sfTempTransmitBuffer.GetLength() << " samples for " 
					<< m_pInputStream->GetNumberOfChannels() << " channels" << std::endl;
#endif
				}
				else
				{
					// Waiting for Trigger
					m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_WAITING_FOR_TRIGGER );
					m_pMessage->WriteMessage( );

#ifdef NET_AUDIO_SHOW_TRAFFIC
					vstr::out() << "[ITANetAudioStreamingServer] Not enough free samples in client buffer, requesting a trigger when more free samples available" << std::endl;
#endif
				}
				break;
			}
			case CITANetAudioProtocol::NP_CLIENT_CLOSE:
			{
				//m_pMessage->SetAnswerType( CITANetAudioProtocol::NP_SERVER_CLOSE );
				//m_pMessage->WriteAnswer();
				StopGently( false );
				//m_pConnection = NULL;
				Stop( );
				return false;
			}
			default:
			{
				vstr::out( ) << "[ITANetAudioStreamingServer] Unkown protocol type : " << iMsgType << std::endl;
				break;
			}
		}

	}

	return true;
}

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if ( VistaThreadLoop::IsRunning( ) )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels( ), m_pInputStream->GetBlocklength( ), true );
	m_oServerParams.dSampleRate = m_pInputStream->GetSampleRate( );
	m_oServerParams.iBlockSize = m_pInputStream->GetBlocklength( );
	m_oServerParams.iChannels = m_pInputStream->GetNumberOfChannels( );
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream( ) const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::GetNetStreamBlocklength( ) const
{
	return m_sfTempTransmitBuffer.GetLength( );
}

int CITANetAudioStreamingServer::GetNetStreamNumberOfChannels( ) const
{
	return m_sfTempTransmitBuffer.channels( );
}

void CITANetAudioStreamingServer::SetAutomaticUpdateRate( )
{
	m_iUpdateStrategy = AUTO;
}

bool CITANetAudioStreamingServer::IsClientConnected( ) const
{
	return m_pNetAudioServer->IsConnected( );
}

std::string CITANetAudioStreamingServer::GetNetworkAddress( ) const
{
	return m_pNetAudioServer->GetServerAddress( );
}

int CITANetAudioStreamingServer::GetNetworkPort( ) const
{
	return m_pNetAudioServer->GetNetworkPort( );
}

void CITANetAudioStreamingServer::Stop( )
{
	m_pNetAudioServer->Stop( );
}
