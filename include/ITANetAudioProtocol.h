/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2016
 *
 * ----------------------------------------------------------------
 *				    ____  __________  _______
 *				   //  / //__   ___/ //  _   |
 *				  //  /    //  /    //  /_|  |
 *				 //  /    //  /    //  ___   |
 *				//__/    //__/    //__/   |__|
 *
 * ----------------------------------------------------------------
 *
 */

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL
#define INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL

#include <ITADataSourcesDefinitions.h>

// ITA includes
#include <ITAException.h>
#include <ITASampleBuffer.h>
#include <ITASampleFrame.h>

// Vista includes
#include <VistaBase/VistaBaseTypes.h>
#include <VistaInterProcComm/Connections/VistaByteBufferSerializer.h>
#include <VistaInterProcComm/Connections/VistaByteBufferDeSerializer.h>

// STL includes
#include <string>
#include <vector>

// Forward declarations
class VistaConnectionIP;

//! Network audio protocol
/**
  * Declaration of message types
  */
struct ITA_DATA_SOURCES_API CITANetAudioProtocol
{
	static const int NET_AUDIO_VERSION = 1;

	static const int NP_INVALID = -1;
	static const int NP_GET_VERSION_INFO = 1;

	static const int NP_CLIENT_OPEN = 100;
	static const int NP_CLIENT_CLOSE = 101;

	static const int NP_SERVER_CLOSE = 200;
	static const int NP_SERVER_GET_RINGBUFFER_SIZE = 201;
	static const int NP_SERVER_GET_RINGBUFFER_FREE = 202;
	static const int NP_SERVER_SEND_SAMPLES = 203;
};


/** Network audio messages 
 *
 * Messages consist of a message part and an answer part, each read or written
 * separately. Messages have a two-int-header (SIZE, MSGTYPE), and
 * answers have a two-int header (SIZE; ANSWERTYPE)
 */
class ITA_DATA_SOURCES_API CITANetAudioMessage
{
public:
	CITANetAudioMessage( VistaSerializingToolset::ByteOrderSwapBehavior bSwapBuffers );

	void ResetMessage();

	void SetConnection( VistaConnectionIP* );

	void WriteMessage();
	void ReadMessage();
	void WriteAnswer();
	void ReadAnswer();

	int GetIncomingMessageSize() const;
	int GetOutgoingMessageSize() const;
	bool GetOutgoingMessageHasData() const;

	void SetMessageType( int nType );
	void SetAnswerType( int nType );
	int GetMessageType() const;
	int GetAnswerType() const;

	// --= Serializing functions =--

	// Basics
	void WriteInt( const int );
	void WriteBool( const bool );
	void WriteDouble( const double );
	void WriteException( const ITAException& );
	void WriteFloat( const float );
	void WriteString( const std::string& );
	void WriteIntVector( const std::vector< int > );
	void WriteFloatVector( const std::vector< float > );


	// --= Reader =--

	std::string ReadString();
	int ReadInt();
	bool ReadBool();
	ITAException ReadException();
	float ReadFloat();
	double ReadDouble();
	std::vector< int > ReadIntVector();
	std::vector< float > ReadFloatVector();
	
	VistaConnectionIP* GetConnection() const;
	void ClearConnection();

private:
	int m_nMessageType;
	int m_nMessageId;
	int m_nAnswerType;
	VistaByteBufferSerializer m_oOutgoing;
	VistaByteBufferDeSerializer m_oIncoming;
	std::vector< VistaType::byte > m_vecIncomingBuffer;

	VistaConnectionIP* m_pConnection;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL
