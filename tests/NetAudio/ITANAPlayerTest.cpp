#include <ITADataSource.h>
#include <ITAPortaudioInterface.h>
#include <ITASampleFrame.h>
#include <ITAStreamMultiplier1N.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort    = 12480;
static double g_dSampleRate = 44.1e3;
static int g_iBufferSize    = 256;

class CITANAStream : public ITADatasource
{
public:
	inline CITANAStream( int iChannels, double dSamplingRate, int iBufferSize )
	    : m_iBufferSize( iBufferSize )
	    , m_dSampleRate( dSamplingRate )
	    , m_iNumChannels( iChannels )
	    , m_sfOutBuffer( iChannels, iBufferSize, true )
	    , m_pConnection( NULL )
	{
		m_pConnection = new VistaConnectionIP( VistaConnectionIP::CT_TCP, g_sServerName, g_iServerPort );
		if( m_pConnection->GetIsConnected( ) )
		{
			cout << "Connection established" << endl;
		}
	};

	inline ~CITANAStream( ) { delete m_pConnection; };

	inline unsigned int GetBlocklength( ) const { return m_iBufferSize; };

	inline unsigned int GetNumberOfChannels( ) const { return m_iNumChannels; };

	inline double GetSampleRate( ) const { return m_dSampleRate; };

	inline const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* )
	{
		// get buffer from net audio stream
		if( m_pConnection->GetIsConnected( ) )
		{
			m_pConnection->Send( &uiChannel, sizeof( int ) );
			m_pConnection->Send( &m_iBufferSize, sizeof( int ) );

			void* pDestination       = (void*)m_sfOutBuffer[uiChannel].data( );
			int iReceiveBufferLength = sizeof( float ) * m_iBufferSize;
			int l                    = m_pConnection->Receive( pDestination, iReceiveBufferLength );
			assert( l == iReceiveBufferLength );
			// cout << "Data received: " << l << endl;
		}
		else
		{
			m_sfOutBuffer.zero( );
		}

		return m_sfOutBuffer[uiChannel].GetData( );
	};

	void IncrementBlockPointer( ) {
		// cout << "Block pointer increment" << endl;
	};


private:
	int m_iBufferSize;
	int m_iNumChannels;
	double m_dSampleRate;
	ITASampleFrame m_sfOutBuffer;
	VistaConnectionIP* m_pConnection;
};

int main( int, char** )
{
	CITANAStream oNetAudioStream( 1, g_dSampleRate, g_iBufferSize );
	ITAStreamMultiplier1N oMultiplier( &oNetAudioStream, 2 );

	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBufferSize );
	ITAPA.Initialize( );
	ITAPA.SetPlaybackDatasource( &oMultiplier );
	ITAPA.Open( );
	ITAPA.Start( );

	// Playback
	float fSeconds = 10.0f;
	cout << "Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	cout << "Done." << endl;

	ITAPA.Stop( );
	ITAPA.Close( );
	ITAPA.Finalize( );

	return 0;
}
