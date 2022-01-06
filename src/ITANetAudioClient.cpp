#include "ITANetAudioClient.h"

#include <ITAException.h>
#include <ITANetAudioStream.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioClient::CITANetAudioClient( ) : m_pConnection( NULL ) {}

CITANetAudioClient::~CITANetAudioClient( )
{
	delete m_pConnection;
}

bool CITANetAudioClient::Connect( const std::string& sAddress, const int iPort, const bool bUseUDP /* = false */ )
{
	if( GetIsConnected( ) )
		ITA_EXCEPT1( MODAL_EXCEPTION, "This net stream is already connected" );

	// Attempt to connect and check parameters
	const VistaConnectionIP::VistaProtocol iCTProtocol = bUseUDP ? VistaConnectionIP::CT_UDP : VistaConnectionIP::CT_TCP;
	m_pConnection                                      = new VistaConnectionIP( iCTProtocol, sAddress, iPort );

	if( !GetIsConnected( ) )
	{
		delete m_pConnection;
		m_pConnection = NULL;
		return false;
	}

	return true;
}

VistaConnectionIP* CITANetAudioClient::GetConnection( ) const
{
	return m_pConnection;
}

void CITANetAudioClient::Disconnect( )
{
	delete m_pConnection;
	m_pConnection = NULL;
}

bool CITANetAudioClient::GetIsConnected( ) const
{
	if( m_pConnection )
		return m_pConnection->GetIsOpen( );
	else
		return false;
}
