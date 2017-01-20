#include <ITANetAudioMessage.h>
#include <ITAStringUtils.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaBase/VistaExceptionBase.h>

#include <cassert>
#include <iostream>
#include <iomanip>

static int S_nMessageIds = 0;

CITANetAudioMessage::CITANetAudioMessage( VistaSerializingToolset::ByteOrderSwapBehavior bSwapBuffers )
	: m_vecIncomingBuffer( 2048 )
	, m_oOutgoing( 2048 )
	, m_pConnection( NULL )
{
	m_oOutgoing.SetByteorderSwapFlag( bSwapBuffers );
	m_oIncoming.SetByteorderSwapFlag( bSwapBuffers );
	ResetMessage();
}

void CITANetAudioMessage::ResetMessage()
{
	if( m_oIncoming.GetTailSize() > 0 )
	{
		std::cerr << "CITANetAudioMessage::ResetMessage() called before message was fully processed!" << std::endl;
	}

	// wait till sending is complete -> this prevents us
	// from deleting the buffer while it is still being read
	// by the connection
	//if( m_pConnection )
	//	m_pConnection->WaitForSendFinish();

	m_nMessageId = S_nMessageIds++;

	m_oOutgoing.ClearBuffer();
	m_oOutgoing.WriteInt32( 0 ); // size dummy
	m_oOutgoing.WriteInt32( 0 ); // type dummy
	m_oOutgoing.WriteInt32( 0 ); // ID

	m_oIncoming.SetBuffer( NULL, 0 );

	m_nMessageType = CITANetAudioProtocol::NP_INVALID;
	m_nAnswerType = CITANetAudioProtocol::NP_INVALID;

	m_pConnection = NULL;

#ifdef NET_AUDIO_SHOW_TRAFFIC
	std::cout << "CITANetAudioMessage [Preparing] (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif
}

void CITANetAudioMessage::SetConnection( VistaConnectionIP* pConn )
{
	m_pConnection = pConn;
}

void CITANetAudioMessage::WriteMessage()
{
	VistaType::byte* pBuffer = ( VistaType::byte* ) m_oOutgoing.GetBuffer();
	VistaType::sint32 iSwapDummy;

	// rewrite size dummy
	iSwapDummy = m_oOutgoing.GetBufferSize() - sizeof( VistaType::sint32 );
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	pBuffer += sizeof( VistaType::sint32 );

	// rewrite type dummy
	iSwapDummy = m_nMessageType;
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	pBuffer += sizeof( VistaType::sint32 );

	// rewrite messageid dummy
	iSwapDummy = m_nMessageId;
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

#ifdef NET_AUDIO_SHOW_TRAFFIC
	std::cout << "CITANetAudioMessage [  Writing] " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif

	try
	{
		int iRawBufferSize = m_oOutgoing.GetBufferSize();
		int nRet = m_pConnection->Send( m_oOutgoing.GetBuffer(), iRawBufferSize );

#ifdef NET_AUDIO_SHOW_TRAFFIC
		std::cout << "CITANetAudioMessage [  Writing] " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ") RAW BUFFER DONE" << std::endl;
#endif

		//m_pConnection->WaitForSendFinish();
		//if( nRet != m_oOutgoing.GetBufferSize() )
		//	ITA_EXCEPT1( NETWORK_ERROR, "Could not write the expected number of bytes" );
	}
	catch (VistaExceptionBase& ex)
	{
		ITA_EXCEPT1(NETWORK_ERROR, ex.GetExceptionText());
	}
}


void CITANetAudioMessage::ReadMessage()
{
	std::cout << "CITANetAudioMessage [ Reading ] Waiting for incoming data" << std::endl;
	long nIncomingBytes = m_pConnection->WaitForIncomingData( 0 );
	std::cout << "CITANetAudioMessage [ Reading ] " << nIncomingBytes << " bytes incoming" << std::endl;

	VistaType::sint32 nMessagePayloadSize;
	int nBytesRead = m_pConnection->ReadInt32( nMessagePayloadSize );
	assert( nBytesRead = sizeof( int ) );
	std::cout << "CITANetAudioMessage [ Reading ] Expecting " << nMessagePayloadSize << " bytes message payload" << std::endl;

	// we need at least the two protocol ints
	assert( nMessagePayloadSize >= 2 * sizeof( VistaType::sint32 ) );

	if( nMessagePayloadSize > ( int ) m_vecIncomingBuffer.size() )
		m_vecIncomingBuffer.resize( nMessagePayloadSize );

	int iBytesReceivedTotal = m_pConnection->Receive( &m_vecIncomingBuffer[ 0 ], nMessagePayloadSize );
	std::cout << "CITANetAudioMessage [ Reading ] Received " << iBytesReceivedTotal << " bytes, so far." << std::endl;

	while( nMessagePayloadSize != iBytesReceivedTotal )
	{
		int iBytesReceived = m_pConnection->Receive( &m_vecIncomingBuffer[ iBytesReceivedTotal ], nMessagePayloadSize );
		iBytesReceivedTotal += iBytesReceived;
		std::cout << "CITANetAudioMessage [ Reading ] Further " << iBytesReceived << " bytes incoming" << std::endl;
	}

	// Transfer data into members
	m_oIncoming.SetBuffer( &m_vecIncomingBuffer[ 0 ], nMessagePayloadSize, false );
	m_nMessageType = ReadInt();
	m_nMessageId = ReadInt();

#ifdef NET_AUDIO_SHOW_TRAFFIC
	std::cout << "CITANetAudioMessage [ Reading ] Finished receiving " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif
}

void CITANetAudioMessage::WriteAnswer()
{

#ifdef NET_AUDIO_SHOW_TRAFFIC
	std::cout << "CITANetAudioMessage [ Answering] to " << m_nMessageType << " with " << m_nAnswerType << " (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif

	assert( m_nAnswerType != CITANetAudioProtocol::NP_INVALID );

	VistaType::byte* pBuffer = ( VistaType::byte* )m_oOutgoing.GetBuffer();
	VistaType::sint32 iSwapDummy;

	// rewrite size dummy
	iSwapDummy = m_oOutgoing.GetBufferSize() - sizeof( VistaType::sint32 );
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	pBuffer += sizeof( VistaType::sint32 );

	// rewrite type dummy
	iSwapDummy = m_nAnswerType;
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	pBuffer += sizeof( VistaType::sint32 );

	// rewrite message dummy
	iSwapDummy = m_nMessageId;
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	int nRet = m_pConnection->Send( m_oOutgoing.GetBuffer(), m_oOutgoing.GetBufferSize() );
	//m_pConnection->WaitForSendFinish();
	//f( nRet != m_oOutgoing.GetBufferSize() )
	//	ITA_EXCEPT1( UNKNOWN, "Could not write the expected number of bytes" );
	
}

void CITANetAudioMessage::ReadAnswer()
{
#ifdef NET_AUDIO_SHOW_TRAFFIC
	std::cout << "CITANetAudioMessage [ Reading] yet unkown answer from message " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ") OK" << std::endl;
#endif

	//try
	{
		VistaType::sint32 nMessageSize;
		int nReturn;
		nReturn = m_pConnection->ReadInt32( nMessageSize );

		if( nReturn != sizeof( VistaType::sint32 ) )
			ITA_EXCEPT1( UNKNOWN, "Protokoll error, was expecting 4 bytes to read message size, but received " + std::to_string( nReturn ) );

		// we need at least the two protocol types
		assert( nMessageSize >= 2 * sizeof( VistaType::sint32 ) );

		if( nMessageSize > ( int ) m_vecIncomingBuffer.size() )
			m_vecIncomingBuffer.resize( nMessageSize );

		// jst: hier nicht while( nReturn < nMessageSize) ReadRawBuffer??
		nReturn = m_pConnection->ReadRawBuffer( &m_vecIncomingBuffer[ 0 ], nMessageSize );
		if( nReturn != nMessageSize )
			ITA_EXCEPT1( UNKNOWN, "Protokoll error, Received less bytes than expected" );

		m_oIncoming.SetBuffer( &m_vecIncomingBuffer[ 0 ], nReturn );
	}
	//catch (VistaExceptionBase& ex)
	{
		// Probable connection loss
		//ITA_EXCEPT1( UNKNOWN, ex.GetExceptionText() );
	}

	m_nAnswerType = ReadInt();
	int nMessageID = ReadInt();
	assert( nMessageID == m_nMessageId );
	m_nMessageId = nMessageID;
}


int CITANetAudioMessage::GetMessageType() const
{
	return m_nMessageType;
}

void CITANetAudioMessage::SetMessageType( int nType )
{
	assert( m_nMessageType == CITANetAudioProtocol::NP_INVALID ); // should only be set once
	m_nMessageType = nType;
}

void CITANetAudioMessage::SetAnswerType( int nType )
{
	assert( m_nAnswerType == CITANetAudioProtocol::NP_INVALID ); // should only be set once
	m_nAnswerType = nType;
}

int CITANetAudioMessage::GetAnswerType() const
{
	return m_nAnswerType;
}

int CITANetAudioMessage::GetIncomingMessageSize() const
{
	return m_oIncoming.GetTailSize();
}

int CITANetAudioMessage::GetOutgoingMessageSize() const
{
	return m_oOutgoing.GetBufferSize();
}

bool CITANetAudioMessage::GetOutgoingMessageHasData() const
{
	return ( m_oOutgoing.GetBufferSize() > 4 * sizeof( VistaType::sint32 ) );
}

void CITANetAudioMessage::WriteString( const std::string& sValue )
{
	m_oOutgoing.WriteInt32( ( VistaType::sint32 )sValue.size() );
	if( !sValue.empty() ) m_oOutgoing.WriteString( sValue );
}

void CITANetAudioMessage::WriteInt( const int iValue )
{
	m_oOutgoing.WriteInt32( ( VistaType::sint32 )iValue );
}

void CITANetAudioMessage::WriteBool( const bool bValue )
{
	m_oOutgoing.WriteBool( bValue );
}

void CITANetAudioMessage::WriteFloat( const float fValue )
{
	m_oOutgoing.WriteFloat32( fValue );
}

void CITANetAudioMessage::WriteDouble( const double dValue )
{
	m_oOutgoing.WriteFloat64( dValue );
}

std::string CITANetAudioMessage::ReadString()
{
	VistaType::sint32 nSize;
	int nReturn = m_oIncoming.ReadInt32( nSize );
	assert( nReturn == sizeof( VistaType::sint32 ) );

	// Empty string?
	if( nSize == 0 ) return "";

	std::string sValue;
	nReturn = m_oIncoming.ReadString( sValue, nSize );
	assert( nReturn == nSize );
	return sValue;
}

int CITANetAudioMessage::ReadInt()
{
	VistaType::sint32 nValue;
	int nReturn = m_oIncoming.ReadInt32( nValue );
	if( nReturn == -1 )
		ITA_EXCEPT1( UNKNOWN, "Could not read integer value from incoming message" );
	assert( nReturn == sizeof( VistaType::sint32 ) );
	return nValue;
}

bool CITANetAudioMessage::ReadBool()
{
	bool bValue;
	int nReturn = m_oIncoming.ReadBool( bValue );
	assert( nReturn == sizeof( bool ) );
	return bValue;
}
float CITANetAudioMessage::ReadFloat()
{
	float fValue;
	int nReturn = m_oIncoming.ReadFloat32( fValue );
	assert( nReturn == sizeof( float ) );
	return fValue;
}
double CITANetAudioMessage::ReadDouble()
{
	double dValue;
	int nReturn = m_oIncoming.ReadFloat64( dValue );
	assert( nReturn == sizeof( double ) );
	return dValue;
}


void CITANetAudioMessage::WriteException( const ITAException& oException )
{
	WriteInt( oException.iErrorCode );
	WriteString( oException.sModule );
	WriteString( oException.sReason );
}

ITAException CITANetAudioMessage::ReadException()
{
	int iErrorCode = ReadInt();
	std::string sModule = ReadString();
	std::string sReason = ReadString();
	return ITAException( iErrorCode, sModule, sReason );
}

VistaConnectionIP* CITANetAudioMessage::GetConnection() const
{
	return m_pConnection;
}

void CITANetAudioMessage::ClearConnection() {
	m_pConnection = NULL;
}

void CITANetAudioMessage::WriteIntVector( const std::vector<int> viData )
{
	int iSize = ( int ) viData.size();
	WriteInt( iSize );
	for( int i = 0; i < iSize; i++ )
		WriteInt( viData[ i ] );
}

std::vector<int> CITANetAudioMessage::ReadIntVector()
{
	std::vector<int> viData;
	int iSize = ReadInt();
	for( int i = 0; i < iSize; i++ )
		viData.push_back( ReadInt() );

	return viData;
}

CITANetAudioProtocol::StreamingParameters CITANetAudioMessage::ReadStreamingParameters()
{
	CITANetAudioProtocol::StreamingParameters oParams;

	oParams.iChannels = ReadInt();
	oParams.dSampleRate = ReadDouble();
	oParams.iBlockSize = ReadInt();

	return oParams;
}

void CITANetAudioMessage::WriteStreamingParameters( const CITANetAudioProtocol::StreamingParameters & oParams )
{
	WriteInt( oParams.iChannels );
	WriteDouble( oParams.dSampleRate );
	WriteInt( oParams.iBlockSize );
}

int CITANetAudioMessage::ReadRingBufferSize()
{
	return ReadInt();
}

void CITANetAudioMessage::WriteRingBufferSize( const int iBufferSize )
{
	WriteInt( iBufferSize );
}

int CITANetAudioMessage::ReadRingBufferFree()
{
	return ReadInt();
}

void CITANetAudioMessage::WriteRingBufferFree( const int iBufferFree )
{
	WriteInt( iBufferFree );
}

void CITANetAudioMessage::ReadSampleFrame( ITASampleFrame* pSampleFrame )
{
	int iChannels = ReadInt();
	int iSamples = ReadInt();

	if( pSampleFrame->channels() != iChannels || pSampleFrame->GetLength() != iSamples )
		pSampleFrame->init( iChannels, iSamples, false );

	for( int i = 0; i < iChannels; i++ )
	{
		for( int j = 0; j < iSamples; j++ )
		{
			( *pSampleFrame )[ i ][ j ] = ReadFloat();
		}
	}
}

void CITANetAudioMessage::WriteSampleFrame( ITASampleFrame* pSamples )
{
	WriteInt( pSamples->channels() );
	WriteInt( pSamples->GetLength() );

	for( int i = 0; i < pSamples->channels(); i++ )
	{
		for( int j = 0; j < pSamples->GetLength(); j++ )
		{
			WriteFloat( ( *pSamples )[ i ][ j ] );
		}
	}
}

