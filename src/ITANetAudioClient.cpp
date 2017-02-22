#include <ITANetAudioClient.h>

#include <ITANetAudioMessage.h>
#include <ITANetAudioProtocol.h>
#include <ITANetAudioStream.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioClient::CITANetAudioClient()
	: m_pConnection( NULL )
{
}

CITANetAudioClient::~CITANetAudioClient()
{
	delete m_pConnection;
}

bool CITANetAudioClient::Connect( const std::string& sAddress, int iPort )
{
	if( GetIsConnected() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "This net stream is already connected" );

	// Attempt to connect and check parameters
	m_pConnection = new VistaConnectionIP( VistaConnectionIP::CT_TCP, sAddress, iPort );
	if( !GetIsConnected() )
	{
		delete m_pConnection;
		m_pConnection = NULL;
		return false;
	}

	return true;
}

VistaConnectionIP* CITANetAudioClient::GetConnection() const
{
	return m_pConnection;
}

void CITANetAudioClient::Disconnect()
{
	delete m_pConnection;
	m_pConnection = NULL;
}

bool CITANetAudioClient::GetIsConnected() const
{
	return ( m_pConnection != NULL ) ? true : false;
}
