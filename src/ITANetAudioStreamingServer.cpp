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

// STL
#include <cmath>
#include <cassert>

CITANetAudioStreamingServer::CITANetAudioStreamingServer()
	: m_pInputStream( NULL )
	, m_iUpdateStrategy( AUTO )
	, m_pConnection( NULL )
{
	m_pNetAudioServer = new CITANetAudioServer();
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort )
{
	if( !m_pInputStream )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not start server without a valid input stream" );

	// TODO: vorr�ckgabe noch anfangen zu senden (Samples)
	if( !m_pNetAudioServer->Start( sAddress, iPort ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection();

	m_pMessage = new CITANetAudioMessage( m_pConnection->GetByteorderSwapFlag() );
	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->ReadMessage(); // blocking

	assert( m_pMessage->GetMessageType() == CITANetAudioProtocol::NP_CLIENT_OPEN );
	CITANetAudioProtocol::StreamingParameters oClientParams = m_pMessage->ReadStreamingParameters();

	bool bOK = false;
	if (m_pInputStream->GetNumberOfChannels() == oClientParams.iChannels &&
		m_pInputStream->GetSampleRate() == oClientParams.dSampleRate &&
		m_pInputStream->GetBlocklength() == oClientParams.iBlockSize)
	{
		bOK = true;
	}

	std::cout << " Client Data: \nAnzahl Channel: " << oClientParams.iChannels << std::endl;
	std::cout << "SampleRate: " << oClientParams.dSampleRate << std::endl;
	std::cout << "Blockgroesse: " << oClientParams.iBlockSize << std::endl;

	std::cout << " Server Data: \nAnzahl Channel: " << m_pInputStream->GetNumberOfChannels() << std::endl;
	std::cout << "SampleRate: " << m_pInputStream->GetSampleRate() << std::endl;
	std::cout << "Blockgroesse: " << m_pInputStream->GetBlocklength() << std::endl;

	m_pMessage->SetAnswerType( CITANetAudioProtocol::NP_SERVER_OPEN );
	m_pMessage->WriteBool( bOK );
	m_pMessage->WriteAnswer();

	Run();

	return true;
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

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if( VistaThreadLoop::IsRunning() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), m_pInputStream->GetBlocklength(), true );

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

bool CITANetAudioStreamingServer::LoopBody()
{
	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->ReadMessage(); // blocking

	int iMsgType = m_pMessage->GetMessageType();
	switch( iMsgType )
	{
	case CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES:
	{
		int iFreeSamples = m_pMessage->ReadInt();
		if( iFreeSamples >= m_pInputStream->GetBlocklength() )
		{
			// Send Samples
			for( int i = 0; i < m_pInputStream->GetNumberOfChannels(); i++ )
			{
				ITAStreamInfo oStreamInfo;
				oStreamInfo.nSamples = m_sfTempTransmitBuffer.GetLength();
				const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
				if( pfData != 0 )
					m_sfTempTransmitBuffer[ i ].write( pfData, m_sfTempTransmitBuffer.GetLength() );
			}
			m_pInputStream->IncrementBlockPointer();
			m_pMessage->SetAnswerType( CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES );
			m_pMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
			m_pMessage->WriteAnswer();
			//std::cout << "Transmitted " << m_pInputStream->GetBlocklength() << " samples, because there where " << iFreeSamples << " free samples on client side" << std::endl;
		}
		else
		{
			//std::cout << "Could not transmitt, because there where only " << iFreeSamples << " free samples on client side" << std::endl;

			// Waiting for Trigger
			m_pMessage->SetAnswerType( CITANetAudioProtocol::NP_SERVER_WAITING_FOR_TRIGGER );
			m_pMessage->WriteAnswer();
			break;
		}


		float fTimeOut = m_pInputStream->GetBlocklength() / m_pInputStream->GetSampleRate();
		//VistaTimeUtils::Sleep( (int) ( 1 * 100 ) );
		break;
	}
	case CITANetAudioProtocol::NP_CLIENT_CLOSE:
	{
		m_pMessage->WriteAnswer();
		m_pConnection = NULL;
		StopGently( true );
		Stop();

		return false;
	}
	default:
	{
		std::cout << "Unkown protocol type: " << iMsgType << std::endl;
		break;
	}
	}

	return true;
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::Transmit(const ITASampleFrame& sfNewSamples, int iNumSamples)
{
	return 0;
}
