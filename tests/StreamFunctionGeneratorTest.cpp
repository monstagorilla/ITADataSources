#include <iostream>
#include <ITADatasource.h>
#include <ITAFileDatasource.h>
#include <ITAFileDatasink.h>
#include <ITAException.h>
#include <ITADatasourceUtils.h>
#include <ITANumericUtils.h>
#include <ITADatasourceRealization.h>

#include <ITAStreamFunctionGenerator.h>

using namespace std;

void test() {
	const unsigned int uiChannels = 1;
	const double dSamplerate = 44100;
	const unsigned int uiBlocklength = 4096;
	const unsigned int uiNumSamples = (unsigned int)( 5 * dSamplerate );

	ITAStreamFunctionGenerator* fg = NULL;

	// Sine

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
			                            ITAStreamFunctionGenerator::SINE,
										400.0, 1.0, true);
	WriteFromDatasourceToFile(fg, "sine_periodic.wav", uiNumSamples, 0.95, false, true);
	delete fg;

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
			                            ITAStreamFunctionGenerator::SINE,
										400.0, 1.0, false);
	WriteFromDatasourceToFile(fg, "sine_single.wav", uiNumSamples, 0.95, false, true);

	// Triangle

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
		                                ITAStreamFunctionGenerator::TRIANGLE,
										400.0, 1.0, true);
	WriteFromDatasourceToFile(fg, "triangle_periodic.wav", uiNumSamples, 0.95, false, true);
	delete fg;

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
			                            ITAStreamFunctionGenerator::TRIANGLE,
										400.0, 1.0, false);
	WriteFromDatasourceToFile(fg, "triangle_single.wav", uiNumSamples, 0.95, false, true);

	delete fg;

	// Sägezahn

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
		                                ITAStreamFunctionGenerator::SAWTOOTH,
										400.0, 1.0, true);
	WriteFromDatasourceToFile(fg, "sawtooth_periodic.wav", uiNumSamples, 0.95, false, true);
	delete fg;

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
			                            ITAStreamFunctionGenerator::SAWTOOTH,
										400.0, 1.0, false);
	WriteFromDatasourceToFile(fg, "sawtooth_single.wav", uiNumSamples, 0.95, false, true);

	delete fg;

	// Rectangle

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
		                                ITAStreamFunctionGenerator::RECTANGLE,
										400.0, 1.0, true);
	WriteFromDatasourceToFile(fg, "rect_periodic.wav", uiNumSamples, 0.95, false, true);
	delete fg;

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
			                            ITAStreamFunctionGenerator::RECTANGLE,
										400.0, 1.0, false);
	WriteFromDatasourceToFile(fg, "rect_single.wav", uiNumSamples, 0.95, false, true);

	delete fg;

	// Dirac

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
		                                ITAStreamFunctionGenerator::DIRAC,
										400.0, 1.0, true);
	WriteFromDatasourceToFile(fg, "dirac_periodic.wav", uiNumSamples, 0.95, false, true);
	delete fg;

	fg = new ITAStreamFunctionGenerator(uiChannels, dSamplerate, uiBlocklength,
			                            ITAStreamFunctionGenerator::DIRAC,
										400.0, 1.0, false);
	WriteFromDatasourceToFile(fg, "dirac_single.wav", uiNumSamples, 0.95, false, true);

	delete fg;
}

int main(int , char**) {

	try {

		test();	
	
	} catch (ITAException& e) {
		cerr << e << endl;
		return 255;
	}
	
	return 0;
}

