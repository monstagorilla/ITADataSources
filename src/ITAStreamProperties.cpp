#include <ITAStreamProperties.h>

ITAStreamProperties::ITAStreamProperties( ) : dSamplerate( 0 ), uiChannels( 0 ), uiBlocklength( 0 ) {}

ITAStreamProperties::ITAStreamProperties( double dSamplerate, unsigned int uiChannels, unsigned int uiBlocklength )
{
	this->dSamplerate   = dSamplerate;
	this->uiChannels    = uiChannels;
	this->uiBlocklength = uiBlocklength;
}

ITAStreamProperties::~ITAStreamProperties( ) {}

bool ITAStreamProperties::operator==( const ITAStreamProperties& rhs ) const
{
	return ( ( dSamplerate == rhs.dSamplerate ) && ( uiChannels == rhs.uiChannels ) && ( uiBlocklength == rhs.uiBlocklength ) );
}

bool ITAStreamProperties::operator!=( const ITAStreamProperties& rhs ) const
{
	return ( ( dSamplerate != rhs.dSamplerate ) || ( uiChannels != rhs.uiChannels ) || ( uiBlocklength != rhs.uiBlocklength ) );
}
