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

CITANetAudioStreamingServer::CITANetAudioStreamingServer()
	: m_pInputStream( NULL )
	, m_iUpdateStrategy( AUTO )
	, m_pConnection( NULL )
	, m_pNetAudioServer( new CITANetAudioServer() )
{
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort )
{
	if( !m_pInputStream )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not start server without a valid input stream" );

	if( !m_pNetAudioServer->Start( sAddress, iPort ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection();

	m_pIncomingMessage = new CITANetAudioMessage( m_pConnection );
	m_pOutgoingMessage = new CITANetAudioMessage( m_pConnection );

	Run();

	return true;
}

bool CITANetAudioStreamingServer::LoopBody()
{
	m_pIncomingMessage->ResetMessage();

	if( m_pIncomingMessage->TryReadMessage() )
	{
		int iMsgType = m_pIncomingMessage->GetMessageType();
		switch( iMsgType )
		{
		case CITANetAudioProtocol::NP_CLIENT_OPEN:
			vstr::out() << m_pIncomingMessage->ReadStreamingParameters().dSampleRate << std::endl;
			break;

		case CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES:
		{
			int iFreeSamples = m_pIncomingMessage->ReadInt();

			if( iFreeSamples >= int( m_pInputStream->GetBlocklength() ) )
			{
				// Send Samples
				for( int i = 0; i < int( m_pInputStream->GetNumberOfChannels() ); i++ )
				{
					ITAStreamInfo oStreamInfo;
					oStreamInfo.nSamples = m_sfTempTransmitBuffer.GetLength();
					const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
					if( pfData != 0 )
						m_sfTempTransmitBuffer[ i ].write( pfData, m_sfTempTransmitBuffer.GetLength() );
				}
				m_pInputStream->IncrementBlockPointer();
				
				m_pOutgoingMessage->ResetMessage();
				m_pOutgoingMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES );
				m_pIncomingMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
				m_pIncomingMessage->WriteMessage();

#ifdef NET_AUDIO_SHOW_TRAFFIC
				vstr::out() << "[ITANetAudioStreamingServer] Transmitted "<< m_sfTempTransmitBuffer.GetLength() << " samples for " 
					<< m_pInputStream->GetNumberOfChannels() << " channels" << std::endl;
#endif
			}
			else
			{

#ifdef NET_AUDIO_SHOW_TRAFFIC
				vstr::out() << "[ITANetAudioStreamingServer] Not enough free samples in client buffer, continuing without sending samples" << std::endl;
#endif
				break;
			}

			break;
		}
		case CITANetAudioProtocol::NP_CLIENT_CLOSE:
		{
			Stop();
			return false;
		}
		default:
		{
			vstr::out() << "[ITANetAudioStreamingServer] Unkown protocol type : " << iMsgType << std::endl;
			break;
		}
		}
	}
	else
	{
		// Request ringbuffer free sample size
		m_pOutgoingMessage->ResetMessage();
		m_pOutgoingMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE );
		m_pOutgoingMessage->WriteMessage();
	}

	return false;
}

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if( VistaThreadLoop::IsRunning() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), m_pInputStream->GetBlocklength(), true );
	m_oServerParams.dSampleRate = m_pInputStream->GetSampleRate();
	m_oServerParams.iBlockSize = m_pInputStream->GetBlocklength();
	m_oServerParams.iChannels = m_pInputStream->GetNumberOfChannels();
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::GetNetStreamBlocklength() const
{
	return m_sfTempTransmitBuffer.GetLength();
}

int CITANetAudioStreamingServer::GetNetStreamNumberOfChannels() const
{
	return m_sfTempTransmitBuffer.channels();
}

void CITANetAudioStreamingServer::SetAutomaticUpdateRate()
{
	m_iUpdateStrategy = AUTO;
}

bool CITANetAudioStreamingServer::IsClientConnected() const
{
	return m_pNetAudioServer->IsConnected();
}

std::string CITANetAudioStreamingServer::GetNetworkAddress() const
{
	return m_pNetAudioServer->GetServerAddress();
}

int CITANetAudioStreamingServer::GetNetworkPort() const
{
	return m_pNetAudioServer->GetNetworkPort();
}

void CITANetAudioStreamingServer::Stop()
{
	m_pNetAudioServer->Stop();
}
