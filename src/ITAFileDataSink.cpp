#include <ITAFileDataSink.h>

#include <ITAFastMath.h>
#include <ITADataSource.h>
#include <ITAAudiofileWriter.h>
#include <ITANumericUtils.h>
     
ITAFileDatasink::ITAFileDatasink(std::string sFilename, ITADatasource* pdsSource, ITAQuantization eQuantization)
: m_pfSilence(NULL) {
	m_pdsSource = pdsSource;
	m_pFileWriter = NULL;

	if (pdsSource) {
		m_vpfData.resize(pdsSource->GetNumberOfChannels());

		ITAAudiofileProperties props;
		props.uiChannels = pdsSource->GetNumberOfChannels();
		props.dSamplerate = pdsSource->GetSamplerate();
		props.eDomain = ITA_TIME_DOMAIN;
		props.eQuantization = eQuantization;

		m_pFileWriter = ITAAudiofileWriter::create(sFilename, props);
		m_pfSilence = fm_falloc(pdsSource->GetBlocklength(), true);
	}
}

ITAFileDatasink::~ITAFileDatasink() {
	delete m_pFileWriter;
	fm_free(m_pfSilence);
}

void ITAFileDatasink::Transfer(unsigned int uiSamples) {
	if (m_pdsSource) {
		// Anzahl der zu transferrierenden Blöcke bestimmen
		unsigned int b = m_pdsSource->GetBlocklength();
		unsigned int n = uprdivu(uiSamples, b);

		for (unsigned int i=0; i<n; i++) {
			for (unsigned int j=0; j<m_pdsSource->GetNumberOfChannels(); j++) {
				const float* pfSrc = m_pdsSource->GetBlockPointer(j, &m_siState);
				if (pfSrc)
					m_vpfData[j] = (float*) pfSrc;
				else
					m_vpfData[j] = m_pfSilence;
			}
			m_pdsSource->IncrementBlockPointer();
			m_siState.nSamples += b;
			m_siState.dTimecode = (double) (m_siState.nSamples) / m_pdsSource->GetSamplerate();

			m_pFileWriter->write(b, m_vpfData);	
		}
	}
}
