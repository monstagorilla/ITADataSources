#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITADataSourceUtils.h>
#include <stdio.h>
#include <iostream>

using namespace std;

/*
   Hinweis: Dieses Programm erstellt quasi eine Kopie einer Audiodatei über eine Dateiquelle
*/

int main(int argc, char* argv[]) {
	
	if (argc != 3) {
		fprintf(stderr, "Syntax: FileSourceTest INPUTFILE OUTPUTFILE\n");
		return 255;
	}
	
	try {
		ITAFileDatasource source(argv[1], 1024);

		printf("Input file:  \"%s\"\n", argv[1]);
	
		printf("Channels:       %d\n", source.GetNumberOfChannels());
		printf("Capacity:       %d Samples\n", source.GetCapacity());
		printf("File capacity:  %d Samples\n", source.GetFileCapacity());
		printf("Samplerate:     %0.3f KHz\n", source.GetSampleRate()/1000);

		printf("\nOutput file: \"%s\"\n", argv[2]);

		WriteFromDatasourceToFile(&source, argv[2], source.GetFileCapacity());

		printf("\nDone.\n");

	}
	catch (ITAException& e) {
		cerr << e << endl;
	}

	return 0;
}
