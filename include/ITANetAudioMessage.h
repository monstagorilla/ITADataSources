/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2017
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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_MESSAGE
#define INCLUDE_WATCHER_ITA_NET_AUDIO_MESSAGE

#include <ITADataSourcesDefinitions.h>

// ITA includes
#include <ITAException.h>
#include <ITASampleBuffer.h>
#include <ITASampleFrame.h>
#include <ITANetAudioProtocol.h>

// Vista includes
#include <VistaInterProcComm/Connections/VistaByteBufferSerializer.h>
#include <VistaInterProcComm/Connections/VistaByteBufferDeSerializer.h>

// STL includes
#include <string>
#include <vector>

class VistaConnectionIP;

//! Network audio messages
/**
  * Messages consist of a message part and an answer part, each read or written
  * separately. Messages have a three-int-header (SIZE, MSGTYPE, ID), and
  * answers have a three-int header (SIZE, ANSWERTYPE, ID)
  *
  * @todo move to src folder
  */
class ITA_DATA_SOURCES_API CITANetAudioMessage
{
public:
	CITANetAudioMessage( VistaSerializingToolset::ByteOrderSwapBehavior bSwapBuffers );

	void SetConnection( VistaConnectionIP* );
	VistaConnectionIP* GetConnection() const;
	void ClearConnection();

	void WriteMessage();
	// Returns false if no incomming data
	bool ReadMessage( int timeout );

	void ResetMessage();

	int GetIncomingMessageSize() const;
	int GetOutgoingMessageSize() const;
	bool GetOutgoingMessageHasData() const;

	void SetMessageType( int nType );
	int GetMessageType() const;


	void WriteInt( const int );
	void WriteBool( const bool );
	void WriteDouble( const double );
	void WriteException( const ITAException& );
	void WriteFloat( const float );
	void WriteString( const std::string& );
	void WriteIntVector( const std::vector< int > );
	void WriteFloatVector( const std::vector< float > );
	void WriteStreamingParameters(const CITANetAudioProtocol::StreamingParameters &); 
	void WriteRingBufferSize(const int);
	void WriteRingBufferFree(const int);
	void WriteSampleFrame(ITASampleFrame*);

	std::string ReadString();
	int ReadInt();
	bool ReadBool();
	ITAException ReadException();
	float ReadFloat();
	double ReadDouble();
	std::vector< int > ReadIntVector();
	std::vector< float > ReadFloatVector();
	CITANetAudioProtocol::StreamingParameters ReadStreamingParameters();
	int ReadRingBufferSize();
	int ReadRingBufferFree();
	void ReadSampleFrame(ITASampleFrame* pSampleFrame);

private:
	int m_nMessageType;
	int m_nMessageId;
	unsigned long m_iBytesReceivedTotal;

	VistaByteBufferSerializer m_oOutgoing; //!< Serialization buffer for messages
	VistaByteBufferDeSerializer m_oIncoming; //!< Deserialization buffer for messages
	std::vector< VistaType::byte > m_vecIncomingBuffer; // Net IO buffer

	VistaConnectionIP* m_pConnection;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_MESSAGE
