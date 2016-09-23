#include <ITAStreamPatchBay.h>

#include <algorithm>
#include <float.h>
#include <iostream>

#include <ITAFastMath.h>
#include <ITAException.h>
#include <ITAFunctors.h>

ITAStreamPatchbay::ITAStreamPatchbay( const double dSamplerate,
									  const int iBlockLength,
									  const int iFadelength,
									  const int iGainAdaption )
: m_dSamplerate( dSamplerate )
, m_iBlockLength( iBlockLength )
, m_iGainFadeLength( iFadelength )
, m_fTempGain( 0.0f )
{
	SetGainAdaption( iGainAdaption );
	Reset();
}

ITAStreamPatchbay::~ITAStreamPatchbay()
{
	// Tidy up ...
	Clear();

	return;
}

void ITAStreamPatchbay::Clear()
{
	std::for_each( m_vpInputs.begin(), m_vpInputs.end(), deleteFunctor< InputDesc > );
	m_vpInputs.clear();
	std::for_each( m_vpOutputs.begin(), m_vpOutputs.end(), deleteFunctor< OutputDesc > );
	m_vpOutputs.clear();

	return;
}

void ITAStreamPatchbay::Reset()
{
	for( int i=0; i<GetNumOutputs(); i++ )
		m_vpOutputs[i]->Reset();

	m_bProcessData = true;
	m_bProcessIncrement = false;

	return;
}

int ITAStreamPatchbay::GetNumInputs() const
{
	return int( m_vpInputs.size() );
}

int ITAStreamPatchbay::GetNumOutputs() const
{
	return int( m_vpOutputs.size() );
}

int ITAStreamPatchbay::GetInputNumChannels( const int iInput ) const
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input channel index out of range");

	return m_vpInputs[iInput]->iChannels;
}

int ITAStreamPatchbay::GetOutputNumChannels( const int iOutput ) const
{
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output channel index out of range");

	return m_vpOutputs[iOutput]->iChannels;
}

int ITAStreamPatchbay::AddInput( const int iNumChannels )
{
	if( iNumChannels < 1 )
		ITA_EXCEPT1( INVALID_PARAMETER, "Inputs must have at least one channel" );

	unsigned int uiIndex =int( m_vpInputs.size() );
	m_vpInputs.push_back( new InputDesc( iNumChannels, m_iBlockLength ) );
	return uiIndex;
}

int ITAStreamPatchbay::AddInput( ITADatasource* pIn )
{
	int iInput = AddInput( pIn->GetNumberOfChannels() );
	SetInputDatasource( iInput, pIn );
	return iInput;
}

bool ITAStreamPatchbay::RemoveInput( int iInput )
{
	if( iInput >= GetNumInputs() )
		ITA_EXCEPT1( INVALID_PARAMETER, "Input channel index out of range" );

	// reduce all input indices by 1 of all inputs with an equal or greater index than the one to be deleted
	for( int i=0; i<GetNumOutputs(); i++ )
	{
		for( int j=0; j<GetOutputNumChannels(i); j++ )
		{
			std::set<Connection, CompareConnection>& dest = m_vpOutputs[i]->conns[j];
			for (std::set<Connection, CompareConnection>::iterator cit=dest.begin(); cit!=dest.end(); ++cit) 
			{
				if (cit->iFirst >= iInput) 
				{
					Connection& conn = *const_cast<Connection*>( &(*cit) );
					conn.iFirst--;
				}
			}
		}
	}

	m_vpInputs.erase( m_vpInputs.begin() + iInput );

	return true;
}

int ITAStreamPatchbay::AddOutput( const int iNumChannels )
{
	if (iNumChannels < 1)
		ITA_EXCEPT1(INVALID_PARAMETER, "Outputs must have at least one channel");

	int iIndex = int( m_vpOutputs.size() );
	m_vpOutputs.push_back( new OutputDesc( this, iNumChannels, m_dSamplerate, m_iBlockLength ) );
	return iIndex;
}

ITADatasource* ITAStreamPatchbay::GetInputDatasource( const int iInput ) const
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input channel index out of range");

	return m_vpInputs[iInput]->pDatasource;
}

void ITAStreamPatchbay::SetInputDatasource( const int iInput, ITADatasource* pdsDatasource )
{
	if( iInput >= GetNumInputs() )
		ITA_EXCEPT1( INVALID_PARAMETER, "Input channel index out of range" );

	if (pdsDatasource != NULL) {
		// Validate the stream properties of the source
		int channels = pdsDatasource->GetNumberOfChannels();
		if (pdsDatasource->GetNumberOfChannels() != m_vpInputs[iInput]->iChannels)
			ITA_EXCEPT1(INVALID_PARAMETER, "Datasource properties do not match the input properties (num channel missmatch");
		if (pdsDatasource->GetSampleRate() != m_dSamplerate)
			ITA_EXCEPT1(INVALID_PARAMETER, "Datasource properties do not match the input properties (samplerate missmatch");
		if (pdsDatasource->GetBlocklength() != m_iBlockLength)
			ITA_EXCEPT1(INVALID_PARAMETER, "Datasource properties do not match the input properties (blocklength missmatch)");
	}

	m_vpInputs[iInput]->pDatasource = pdsDatasource;
}

ITADatasource* ITAStreamPatchbay::GetOutputDatasource( const int uOutput ) const
{
	if (uOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output channel index out of range");

	return m_vpOutputs[uOutput];
}

void ITAStreamPatchbay::ConnectChannels( const int iInput,
		                                 const int iInputChannel,
				                         const int iOutput,
										 const int iOutputChannel )
{
	// Delegate with default gain
	ConnectChannels( iInput, iInputChannel, iOutput, iOutputChannel, 1.0f );
}

void ITAStreamPatchbay::ConnectChannels( const int iInput,
		                                 const int iInputChannel,
				                         const int iOutput,
										 const int iOutputChannel,
										 const double dGain )
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range (in conn channel)");
	if (iInputChannel >= m_vpInputs[iInput]->iChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Input channel index out of range (in conn channel)");
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");
	if (iOutputChannel >= m_vpOutputs[iOutput]->iChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Output channel index out of range");

	m_vpOutputs[iOutput]->conns[iOutputChannel].insert( Connection(iInput, iInputChannel, (float) dGain) );
}

void ITAStreamPatchbay::Disconnect( const int iInput, const int iOutput )
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range");
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");

	const Connections& conns = m_vpOutputs[iOutput]->conns;
	std::set<Connection, CompareConnection>::const_iterator cit;
	for( int i=0; i < int( conns.size() ); i++ )
	{
		for( int j=0; j<m_vpInputs[iInput]->iChannels; j++ )
		{
			cit = conns[i].find(  Connection(iInput, j) );
			if( cit != conns[j].end() )
			{
				m_vpOutputs[iOutput]->conns[j].erase( cit );
			}
		}
	}
}

void ITAStreamPatchbay::DisconnectOutput( const int iOutput )
{
	if( iOutput >= GetNumOutputs() )
		ITA_EXCEPT1( INVALID_PARAMETER, "Output index out of range" );

	m_vpOutputs[iOutput]->conns.clear();

	return;
}

void ITAStreamPatchbay::DisconnectAllOutputs()
{
	for( int i = 0; i < GetNumOutputs(); ++i )
		DisconnectOutput( i );

	return;
}


void ITAStreamPatchbay::DisconnectOutputChannel( const int iOutput,  const int iOutputChannel )
{
	if( iOutput >= GetNumOutputs() )
		ITA_EXCEPT1( INVALID_PARAMETER, "Output index out of range" );
	if( iOutputChannel >= m_vpOutputs[iOutput]->iChannels )
		ITA_EXCEPT1( INVALID_PARAMETER, "Output channel index out of range" );

	m_vpOutputs[iOutput]->conns[iOutputChannel].clear();

	return;
}


bool ITAStreamPatchbay::IsInputMuted( const int iInput )const
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range");
	return m_vpInputs[iInput]->bMuted;
}

void ITAStreamPatchbay::SetInputMuted( const int iInput,  const bool bMuted )
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range");

	m_vpInputs[iInput]->bMuted = bMuted;
}

bool ITAStreamPatchbay::IsOutputMuted( const int iOutput) const
{
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");

	return m_vpOutputs[iOutput]->bMuted;
}


void ITAStreamPatchbay::SetOutputMuted( const int iOutput,  const bool bResult)
{
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");

	m_vpOutputs[iOutput]->bMuted = bResult;
}

double ITAStreamPatchbay::GetInputGain( const int iInput ) const
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range");

	return m_vpInputs[iInput]->fCurrentGain;
}

void ITAStreamPatchbay::SetInputGain( const int iInput,  const double dGain )
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range");
	
	/* [fwe] For Ambisonics we also allow negative gains 
	if (dGain < 0)
		ITA_EXCEPT1(INVALID_PARAMETER, "Gain must not be negative");
	*/

	m_vpInputs[iInput]->fNewGain = (float) dGain;
}

double ITAStreamPatchbay::GetOutputGain( const int iOutput ) const
{
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");

	return m_vpOutputs[iOutput]->fCurrentGain;
}

void ITAStreamPatchbay::SetOutputGain( const int iOutput,  const double dGain )
{
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");
		
	/* [fwe] For Ambisonics we also allow negative gains 
	if (dGain < 0)
		ITA_EXCEPT1(INVALID_PARAMETER, "Gain must not be negative");
	*/

	m_vpOutputs[iOutput]->fNewGain = (float) dGain;
}

double ITAStreamPatchbay::GetConnectionGain( const int iInput,
	                                         const int iInputChannel,
			                                 const int iOutput,
											 const int iOutputChannel ) const
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range (in conn channel)");
	if (iInputChannel >= m_vpInputs[iInput]->iChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Input channel index out of range (in conn channel)");
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");
	if (iOutputChannel >= m_vpOutputs[iOutput]->iChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Output channel index out of range");

	const std::set<Connection, CompareConnection>& conns = m_vpOutputs[iOutput]->conns[iOutputChannel];
	const std::set<Connection, CompareConnection>::const_iterator cit = conns.find(  Connection(iInput, iInputChannel) );
	if (cit == conns.end())
		ITA_EXCEPT1(INVALID_PARAMETER, "No such connection");
	return cit->fCurrentGain;
}

void ITAStreamPatchbay::SetConnectionGain( const int iInput,
	                                       const int iInputChannel,
			                               const int iOutput,
	                                       const int iOutputChannel,
										   const double dGain )
{
	if (iInput >= GetNumInputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Input index out of range (in conn channel)");
	if (iInputChannel >= m_vpInputs[iInput]->iChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Input channel index out of range (in conn channel)");
	if (iOutput >= GetNumOutputs())
		ITA_EXCEPT1(INVALID_PARAMETER, "Output index out of range");
	if (iOutputChannel >= m_vpOutputs[iOutput]->iChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Output channel index out of range");
		
	/* [fwe] For Ambisonics we also allow negative gains 
	if (dGain < 0)
		ITA_EXCEPT1(INVALID_PARAMETER, "Gain must not be negative");
	*/

	std::set<Connection, CompareConnection>& conns = m_vpOutputs[iOutput]->conns[iOutputChannel];
	std::set<Connection, CompareConnection>::iterator it = conns.find(  Connection(iInput, iInputChannel) );
	if (it == conns.end())
		ITA_EXCEPT1(INVALID_PARAMETER, "No such connection");

	// [fwe] Note: Visual Studio 2010 is more strict with the STL (which is correct).
	// We need to remove the const-modifier from the set element. This is okay because
	// the change of the gain parameter does not change the ordering in the set!
	// Looks nasty but works and is okay.
	Connection& conn = *const_cast<Connection*>( &(*it) );
	conn.fNewGain = (float) dGain;
}

unsigned int ITAStreamPatchbay::GetGainAdaption() const
{
	return m_iGainAdaption;
}

void ITAStreamPatchbay::SetGainAdaption( const int iGainAdaption )
{
	if (iGainAdaption > GAIN_LINEAR_FADE)
		ITA_EXCEPT1(INVALID_PARAMETER, "Invalid gain adaption");

	m_iGainAdaption = iGainAdaption;
}

unsigned int ITAStreamPatchbay::GetGainFadeLength() const
{
	return m_iGainFadeLength;
}

void ITAStreamPatchbay::SetGainFadeLength( const int iFadeLength )
{
	m_iGainFadeLength = (std::min)(iFadeLength, m_iBlockLength);
}

void ITAStreamPatchbay::PrintConnections() const
{
	for( int i=0; i<GetNumOutputs(); i++ )
	{
		for( int j=0; j<GetOutputNumChannels(i); j++ )
		{
			std::set< Connection, CompareConnection >& dest = m_vpOutputs[i]->conns[j];
			for (std::set< Connection, CompareConnection >::const_iterator cit=dest.begin(); cit!=dest.end(); ++cit)
			{
				std::cout << "output[" << i << "," << j << "] <== input[" << cit->iFirst << "," << cit->iSecond << "]" << std::endl;
			}
		}
	}
}

void ITAStreamPatchbay::HandlePreGetBlockPointer( ITADatasourceRealization* pSender, unsigned int uiChannel )
{
	// Do nothing ...
}

void ITAStreamPatchbay::HandlePostIncrementBlockPointer( ITADatasourceRealization* pSender )
{
	// Ignore, if already called for another output
	if (!m_bProcessIncrement) return;
	
	// Turn data producion on again, when the first output's block pointer is incremented
	m_bProcessData = true;
	m_bProcessIncrement = false;

	// Increment block pointers on all attached inputs
	// [Bugfix fwe 2014-11-24] Only increment one time for each source!

	// Build a unique list of sources
	std::set<ITADatasource*> vSources;
	for( int i=0; i<GetNumInputs(); i++) {
		ITADatasource* pdsInputSource = m_vpInputs[i]->pDatasource;
		if (pdsInputSource) vSources.insert(pdsInputSource);
	}
	
	// Increment each data source only once
	for (std::set<ITADatasource*>::iterator it=vSources.begin(); it!=vSources.end(); ++it)
		(*it)->IncrementBlockPointer();
}

void ITAStreamPatchbay::HandleProcessStream( ITADatasourceRealization* pSender, const ITAStreamInfo* pStreamInfo )
{
	// Output X wants data
	OutputDesc* pOutput = dynamic_cast<OutputDesc*>( pSender );

	// Produce all data?
	if (m_bProcessData) {
		ProcessData(pStreamInfo);
		m_bProcessData = false;
		m_bProcessIncrement = true;
	}
}

void ITAStreamPatchbay::ProcessData( const ITAStreamInfo* pStreamInfo )
{
	// Fetch current block/read pointer (or set to NULL if not connected)
	int iNumInputs = GetNumInputs();
	for( int i=0; i<iNumInputs; i++ )
	{
		InputDesc* pInput( m_vpInputs[i] );
		int iNumInChannels = GetInputNumChannels( i );
		if( pInput->pDatasource )
		{
			for( int j=0; j<iNumInChannels; j++ )
				pInput->vpfInputData[j] = pInput->pDatasource->GetBlockPointer( j, pStreamInfo );
		}
		else
		{
			for( int j=0; j<iNumInChannels; j++ )
				pInput->vpfInputData[j] = NULL;
		}
	}

	// Process in/out data
	int iNumOutputs = GetNumOutputs();
	for( int i=0; i< iNumOutputs; i++ )
	{
		 OutputDesc* pOutput( m_vpOutputs[i] );
		for( int j=0; j<GetOutputNumChannels( i ); j++ )
		{
			// Determine the given connections from this output ant it's channel
			std::set< Connection, CompareConnection >& dest( pOutput->conns[j] );

			// Get the destination buffer
			float* pfOutChannelData = pOutput->GetWritePointer( j );

			// First zero the output
			fm_zero( pfOutChannelData, m_iBlockLength );

			for( std::set< Connection, CompareConnection >::iterator it=dest.begin(); it!=dest.end(); ++it )
			{
				unsigned int iInput = it->iFirst;
				unsigned int iInputChannel = it->iSecond;

				InputDesc* pConnectedInChannel( m_vpInputs[iInput] );

				// Fetch the input samples
				const float* pfInChannelData = pConnectedInChannel->vpfInputData[iInputChannel];

				// No input channel connected => Proceed with next channel ...
				if( pfInChannelData == nullptr )
				{
					// Update connection gains
					Connection& conn = *const_cast<Connection*>( &(*it) );
					conn.fCurrentGain = conn.fNewGain;
					continue;
				}

				bool bMuted = ( pOutput->bMuted || pConnectedInChannel->bMuted );

				// Calculate the effective gains (input gain * output gain * connection gain)
				float fCurrentGain = pOutput->fCurrentGain *
					                 pConnectedInChannel->fCurrentGain *
									 it->fCurrentGain;
				float fNewGain = pOutput->fNewGain *
					             pConnectedInChannel->fNewGain *
								 it->fNewGain;

				// TODO [fwe]: Nur eine Notlösung. So knackst das. Da muss Fading rein...
				if( bMuted )
				{
					fCurrentGain = fNewGain = 0;
				}

				if( ( m_iGainAdaption == GAIN_LINEAR_FADE ) && ( fabs( fCurrentGain - fNewGain ) > FLT_MIN ) )
				{
					// Smooth gain adaptation
					float fScale = ( fNewGain - fCurrentGain ) / float( m_iGainFadeLength - 1 );

					for( int k=0; k<m_iGainFadeLength; k++)
						pfOutChannelData[k] += ( pfInChannelData[k] * ( fCurrentGain + ( k*fScale ) ) );
					for( int k=m_iGainFadeLength; k<m_iBlockLength; k++ )
						pfOutChannelData[k] += ( pfInChannelData[k] * fNewGain );
				}
				else
				{
					// Use new gain instantly
					for( int k=0; k<m_iBlockLength; k++ )
						pfOutChannelData[k] += pfInChannelData[k] * fNewGain;
				}

				// Update connection gains, do a dirty const removal
				const Connection& cconn( *it );
				Connection& conn = const_cast< Connection& >( cconn );
				conn.fCurrentGain.set( conn.fNewGain.get() );
			}
		}
		
		// Update output gains (after processing data)
		pOutput->fCurrentGain = pOutput->fNewGain;
		pOutput->IncrementWritePointer();
	}

	// Update input gains (can first be done after all outputs are processed!)
	for( int i=0; i<GetNumInputs(); i++ )
		m_vpInputs[i]->fCurrentGain = m_vpInputs[i]->fNewGain;

	return;
}
