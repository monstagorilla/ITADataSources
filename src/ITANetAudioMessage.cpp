#include "ITANetAudioMessage.h"

#include <ITAClock.h>
#include <ITADataLog.h>
#include <ITAStringUtils.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaBase/VistaExceptionBase.h>
#include <VistaBase/VistaStreamUtils.h>

#include <algorithm>
#include <cstring>
#include <cassert>
#include <iostream>
#include <iomanip>

static int S_nMessageIds = 0;

struct ITANetAudioMessageLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "MessageType";
		os << "\t" << "Action";
		os << "\t" << "InternalProcessingTime";
		os << "\t" << "PayloadSize";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << sMessageType;
		os << "\t" << sAction;
		os << "\t" << std::setprecision( 12 ) << dInternalProcessingTime;
		os << "\t" << nMessagePayloadSize;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp; //!< Time stamp at beginning of logged message process
	std::string sMessageType; //!< Protocol message type
	std::string sAction; //!< Triggered action
	double dInternalProcessingTime; //!< Processing within message class
	VistaType::sint32 nMessagePayloadSize; //!< Data

};

class ITABufferedDataLoggerImplProtocol : public ITABufferedDataLogger < ITANetAudioMessageLog > {};

CITANetAudioMessage::CITANetAudioMessage( VistaSerializingToolset::ByteOrderSwapBehavior bSwapBuffers )
	: m_vecIncomingBuffer( 2048 )
	, m_oOutgoing( 2048 )
	, m_pConnection( NULL )
	, m_iBytesReceivedTotal( 0 )
	, m_sMessageLoggerBaseName( "ITANetAudioMessage" )
	, m_bDebuggingEnabled( false )
{
	m_pMessageLogger = new ITABufferedDataLoggerImplProtocol();
	m_pMessageLogger->setOutputFile( m_sMessageLoggerBaseName + ".log" );

	m_nMessageId = 0;
	m_oOutgoing.SetByteorderSwapFlag( bSwapBuffers );
	m_oIncoming.SetByteorderSwapFlag( bSwapBuffers );

	ResetMessage();
}

void CITANetAudioMessage::ResetMessage()
{
	const double dInTime = ITAClock::getDefaultClock()->getTime();

	ITANetAudioMessageLog oLog;
	oLog.uiBlockId = m_nMessageId;
	oLog.sMessageType = "RESET_MESSAGE";
	oLog.nMessagePayloadSize = 0;
	oLog.dWorldTimeStamp = dInTime;

	oLog.sAction = "reset_message";

	if( m_oIncoming.GetTailSize() > 0 )
	{
		vstr::err() << "CITANetAudioMessage::ResetMessage() called before message was fully processed!" << std::endl;
		oLog.sAction = "reset_failed";
	}

	// wait till sending is complete -> this prevents us
	// from deleting the buffer while it is still being read
	// by the connection
	if( m_pConnection != NULL )
		m_pConnection->WaitForSendFinish(); // can be time-costly

	m_nMessageId = S_nMessageIds++;

	m_oOutgoing.ClearBuffer();
	m_oOutgoing.WriteInt32( 0 ); // Payload size
	m_oOutgoing.WriteInt32( 0 ); // Message Type
	m_oOutgoing.WriteInt32( 0 ); // Identifier

	m_oIncoming.SetBuffer( NULL, 0 );

	m_nMessageType = -1;

	oLog.dInternalProcessingTime = ITAClock::getDefaultClock()->getTime() - dInTime;
	m_pMessageLogger->log( oLog );

#if NET_AUDIO_SHOW_TRAFFIC
	vstr::out() << "CITANetAudioMessage [Preparing] (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif
}

void CITANetAudioMessage::SetConnection( VistaConnectionIP* pConn )
{
	m_pConnection = pConn;
}

void CITANetAudioMessage::WriteMessage()
{
	const double dInTime = ITAClock::getDefaultClock()->getTime();
	ITANetAudioMessageLog oLog;
	oLog.dWorldTimeStamp = dInTime;
	VistaType::byte* pBuffer = ( VistaType::byte* ) m_oOutgoing.GetBuffer();
	VistaType::sint32 iSwapDummy;

	// rewrite size dummy
	iSwapDummy = m_oOutgoing.GetBufferSize() - sizeof( VistaType::sint32 );
	oLog.nMessagePayloadSize = iSwapDummy;

	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	std::memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	pBuffer += sizeof( VistaType::sint32 );

	// rewrite type dummy
	iSwapDummy = m_nMessageType;
	oLog.sMessageType = CITANetAudioProtocol::GetNPMessageID( m_nMessageType );
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	std::memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );

	pBuffer += sizeof( VistaType::sint32 );

	// rewrite messageid dummy
	iSwapDummy = m_nMessageId;
	oLog.uiBlockId = m_nMessageId;
	if( m_oOutgoing.GetByteorderSwapFlag() )
		VistaSerializingToolset::Swap4( &iSwapDummy );
	std::memcpy( pBuffer, &iSwapDummy, sizeof( VistaType::sint32 ) );
	oLog.sAction = "write_message";
	oLog.dInternalProcessingTime = ITAClock::getDefaultClock()->getTime() - dInTime;
	m_pMessageLogger->log( oLog );

	//std::cout << GetMessageLoggerBaseName() << " Write: Groesse:" << oLog.nMessagePayloadSize << " MsgType:" << oLog.sMessageType << " MsgID: " << oLog.uiBlockId << std::endl;

#if NET_AUDIO_SHOW_TRAFFIC
	vstr::out() << "CITANetAudioMessage [  Writing] " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif

	try
	{
		// It appears safe to send even very big data payload, so we will send at once
		int iRawBufferSize = m_oOutgoing.GetBufferSize();
		int nRet = m_pConnection->Send( m_oOutgoing.GetBuffer(), iRawBufferSize );

#if NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "CITANetAudioMessage [  Writing] " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ") RAW BUFFER DONE" << std::endl;
#endif

		m_pConnection->WaitForSendFinish();
		if( nRet != m_oOutgoing.GetBufferSize() )
			VISTA_THROW( "ITANetAudioMessage: could not send all data from output buffer via network connection", 255 );
	}
	catch( VistaExceptionBase& ex )
	{
		ITA_EXCEPT1( NETWORK_ERROR, ex.GetExceptionText() );
	}
}


bool CITANetAudioMessage::ReadMessage( int timeout )
{
	ITANetAudioMessageLog oLog;
	const double dInTime = ITAClock::getDefaultClock()->getTime();
	oLog.dWorldTimeStamp = dInTime;

#if NET_AUDIO_SHOW_TRAFFIC
	vstr::out() << "CITANetAudioMessage [ Reading ] Waiting for incoming data" << std::endl;
#endif
	// WaitForIncomming Data int in ca ms
	long nIncomingBytes = m_pConnection->WaitForIncomingData( timeout );
	// TODO Timer entfernen
	if( nIncomingBytes == -1 )
		return false;

	//if (timeout != 0)
	//nIncomingBytes = m_pConnection->WaitForIncomingData( 0 );
#if NET_AUDIO_SHOW_TRAFFIC
	vstr::out() << "CITANetAudioMessage [ Reading ] " << nIncomingBytes << " bytes incoming" << std::endl;
#endif

	VistaType::sint32 nMessagePayloadSize;
	int nBytesRead = m_pConnection->ReadInt32( nMessagePayloadSize );
	assert( nBytesRead == sizeof( VistaType::sint32 ) );
	oLog.nMessagePayloadSize = nMessagePayloadSize;
#if NET_AUDIO_SHOW_TRAFFIC
	vstr::out() << "CITANetAudioMessage [ Reading ] Expecting " << nMessagePayloadSize << " bytes message payload" << std::endl;
#endif
	if( nMessagePayloadSize <= 0 )
		return false;
	// we need at least the two protocol ints
	//assert( nMessagePayloadSize >= 2 * sizeof( VistaType::sint32 ) );

	//std::cout << GetMessageLoggerBaseName() << " Read: " << nMessagePayloadSize << std::endl;

	if( nMessagePayloadSize > ( int ) m_vecIncomingBuffer.size() )
		m_vecIncomingBuffer.resize( nMessagePayloadSize );

	// Receive all incoming data (potentially splitted)

	while( ( unsigned long ) nMessagePayloadSize > m_iBytesReceivedTotal )
	{
		int iIncommingBytes = m_pConnection->WaitForIncomingData( 0 );
		int iBytesReceived;
		if( nMessagePayloadSize < iIncommingBytes )
			iBytesReceived = m_pConnection->Receive( &m_vecIncomingBuffer[ m_iBytesReceivedTotal ], nMessagePayloadSize - m_iBytesReceivedTotal );
		else
			iBytesReceived = m_pConnection->Receive( &m_vecIncomingBuffer[ m_iBytesReceivedTotal ], iIncommingBytes );
		m_iBytesReceivedTotal += iBytesReceived;
#if NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ CITANetAudioMessage ] " << std::setw( 3 ) << std::floor( iBytesReceivedTotal / float( nMessagePayloadSize ) * 100.0f ) << "% transmitted" << std::endl;
#endif
	}
	m_iBytesReceivedTotal = 0;

	oLog.sAction = "read_message";
	// Transfer data into members
	m_oIncoming.SetBuffer( &m_vecIncomingBuffer[ 0 ], nMessagePayloadSize, false );
	m_nMessageType = ReadInt();
	m_nMessageId = ReadInt();

	//std::cout << GetMessageLoggerBaseName() << " Read: MsgType:" << m_nMessageType << " MsgID: " << m_nMessageId << std::endl;

	oLog.sMessageType = CITANetAudioProtocol::GetNPMessageID( m_nMessageType );
	oLog.uiBlockId = m_nMessageId;
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock()->getTime() - dInTime;
	m_pMessageLogger->log( oLog );

#if NET_AUDIO_SHOW_TRAFFIC
	vstr::out() << "CITANetAudioMessage [ Reading ] Finished receiving " << m_nMessageType << " (id=" << std::setw( 4 ) << m_nMessageId << ")" << std::endl;
#endif
	return true;
}

int CITANetAudioMessage::GetMessageType() const
{
	return m_nMessageType;
}

void CITANetAudioMessage::SetMessageType( int nType )
{
	m_nMessageType = nType;
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

void CITANetAudioMessage::ClearConnection()
{
	m_pConnection = NULL;
	if( GetIsDebuggingEnabled() == false )
		m_pMessageLogger->setOutputFile( "" ); // disable output
	delete m_pMessageLogger;
}

void CITANetAudioMessage::WriteIntVector( const std::vector< int > viData )
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
	oParams.iRingBufferSize = ReadInt();

	return oParams;
}

void CITANetAudioMessage::WriteStreamingParameters( const CITANetAudioProtocol::StreamingParameters & oParams )
{
	WriteInt( oParams.iChannels );
	WriteDouble( oParams.dSampleRate );
	WriteInt( oParams.iBlockSize );
	WriteInt( oParams.iRingBufferSize );
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
		for( int j = 0; j < iSamples; j++ )
			( *pSampleFrame )[ i ][ j ] = ReadFloat();
}

void CITANetAudioMessage::WriteSampleFrame( ITASampleFrame* pSamples )
{
	WriteInt( pSamples->channels() );
	WriteInt( pSamples->GetLength() );

	for( int i = 0; i < pSamples->channels(); i++ )
		for( int j = 0; j < pSamples->GetLength(); j++ )
			WriteFloat( ( *pSamples )[ i ][ j ] );
}

void CITANetAudioMessage::SetMessageLoggerBaseName( const std::string& sBaseName )
{
	assert( !sBaseName.empty() );
	if( m_pMessageLogger )
		m_pMessageLogger->setOutputFile( sBaseName + ".log" );
	m_sMessageLoggerBaseName = sBaseName;
}

std::string CITANetAudioMessage::GetMessageLoggerBaseName() const
{
	return m_sMessageLoggerBaseName;
}

void CITANetAudioMessage::SetDebuggingEnabled( bool bEnabled )
{
	m_bDebuggingEnabled = bEnabled;
}

bool CITANetAudioMessage::GetIsDebuggingEnabled() const
{
	return m_bDebuggingEnabled;
}
