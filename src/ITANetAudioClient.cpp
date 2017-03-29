#include "ITANetAudioClient.h"

#include <ITANetAudioStream.h>
#include <ITAException.h>

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
	if( m_pConnection )
		return m_pConnection->GetIsOpen();
	else
		return false;
}
