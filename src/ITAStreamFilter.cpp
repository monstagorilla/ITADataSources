/*
   +----------------------------------------------------------+
   |                                                          |
   |  ITAStreamFilter.cpp                                     |
   |  Eine abstrakte Basisklasse für Filter welche            |
   |  auf Audiostreams arbeiten                               |
   |                                                          |
   |  Autoren: Frank Wefers                                   |
   |                                                          |
   |  (c) Copyright Institut für technische Akustik (ITA)     |
   |      Aachen university of technology (RWTH), 2008        |
   |                                                          |
   +----------------------------------------------------------+
                                                                */
// $Id: ITAStreamFilter.cpp,v 1.2 2008-09-02 08:53:35 fwefers Exp $

#include "ITAStreamFilter.h"

#include <ITAException.h>

ITAStreamFilter::ITAStreamFilter(ITADatasource* pdsInput)
: ITADatasource(pdsInput->GetBlocklength(), pdsInput->GetNumberOfChannels(), pdsInput->GetSamplerate()),
  m_pdsInput(pdsInput),
  m_vpfInputData(pdsInput->GetNumberOfChannels()), 
  m_vpfOutputBuffer(pdsInput->GetNumberOfChannels(), NULL) {
	
	// Ausgabepuffer allozieren (SIMD-aligned!)
	for (unsigned int i=0; i<m_uiChannels; i++)
		m_vpfOutputBuffer[i] = fm_falloc(m_uiBlocklength);

	m_bGBPCalled = false;

	if (pdsInput != NULL) SetInputDatasource(pdsInput);
}

ITAStreamFilter::~ITAStreamFilter() {
	delete[] m_ppfInputData;
	if (m_ppfOutputBuffer) 
		for (unsigned int i=0; i<m_uiChannels; i++) delete[] m_ppfOutputBuffer[i];
	delete[] m_ppfOutputBuffer;
}

ITADatasource* ITAStreamFilter::GetInputDatasource() const { return m_pdsInput; }

void ITAStreamFilter::SetInputDatasource(ITADatasource* pdsInput) {
	/* Hinweis: Sicherstellen, das sobald erstmal eine Datenquelle zugeordnet wurde,
                die Zuordnung zu einer Datenquelle nicht mehr aufgehoben werden kann (Nullzeiger).
				Also: Es dürfen andere Datenquellen gesetzt werden, diese müssen aber
				die selben Eigenschaften ausweisen, wie die erstmalig zugeordnete Datenquelle. */

	if (pdsInput == NULL)
		ITA_EXCEPT1(INVALID_PARAMETER, "Nullpointer not allowed as parameter");
	
	// Wurde schon erstmalig eine Datenquelle zuordnet? Dann auf gleiche Eigenschaften prüfen!
	if (m_pdsInput != NULL) {
		if ((pdsInput->GetBlocklength() != m_uiBlocklength) ||
			(pdsInput->GetNumberOfChannels() != m_uiChannels) ||
			(pdsInput->GetSamplerate() != m_dSamplerate))
			ITA_EXCEPT1(INVALID_PARAMETER, "The new datasources' properties do not match the previous datasources' properties");

		// TODO: Bei alter Datenquelle aufräumen? z.B. IncrementBlockPointer aufrufen?
		for (unsigned int i=0; i<m_uiChannels; i++)
			for (unsigned int j=0; j<m_uiBlocklength; j++)
				m_ppfOutputBuffer[i][j] = 0;
	} else {
		// Erstmalige Zuordnung. Sicherstellen das alle Datenquellen-Eigenschaften bereits festgelegt sind
		if ((pdsInput->GetBlocklength() == 0) ||
		    (pdsInput->GetNumberOfChannels() == 0) ||
			(pdsInput->GetSamplerate() == 0))
			ITA_EXCEPT1(INVALID_PARAMETER, "The input datasources' properties are not (completely) defined yet");

		m_pdsInput = pdsInput;
		m_uiChannels = m_pdsInput->GetNumberOfChannels();
		m_uiBlocklength = m_pdsInput->GetBlocklength();
		m_dSamplerate = m_pdsInput->GetSamplerate();

		m_ppfOutputBuffer = new float*[m_uiChannels];
		memset(m_ppfOutputBuffer, 0, m_uiChannels * sizeof(float*));
		for (unsigned int i=0; i<m_uiChannels; i++) m_ppfOutputBuffer[i] = new float[m_uiBlocklength];
		for (unsigned int i=0; i<m_uiChannels; i++)
			for (unsigned int j=0; j<m_uiBlocklength; j++)
				m_ppfOutputBuffer[i][j] = 0;

		m_ppfInputData = new const float*[m_uiChannels];
	}

	m_bGBPCalled = false;
}
	
const float* ITAStreamFilter::GetBlockPointer(unsigned int uiChannel, bool bWaitForData) {
	if ((m_pdsInput != NULL) && !m_bGBPCalled) {
		// Daten holen
		for (unsigned int i=0; i<m_uiChannels; i++)
			m_ppfInputData[i] = m_pdsInput->GetBlockPointer(i, bWaitForData);

		// Filter berechnen
		ProcessAllChannels(m_ppfInputData, m_ppfOutputBuffer);

		m_bGBPCalled = true;
	}		


	return m_ppfOutputBuffer[uiChannel];
}

void ITAStreamFilter::IncrementBlockPointer() {
	if (m_pdsInput) m_pdsInput->IncrementBlockPointer();
	m_bGBPCalled = false;
}