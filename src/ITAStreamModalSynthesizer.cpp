#include <ITAStreamModalSynthesizer.h>

ITAStreamModalSynthesizer::ITAStreamModalSynthesizer( double dSamplerate, unsigned int uiBlocklength ) : ITADatasourceRealization( 1, dSamplerate, uiBlocklength ) {}


void ITAStreamModalSynthesizer::Clear( ) {}

void ITAStreamModalSynthesizer::SetModes( const ModeData* pModeData, int iNumModes ) {}

void ITAStreamModalSynthesizer::SetModes( std::vector<ModeData> vModeData ) {}
