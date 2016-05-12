#include "ITAStreamYJunction.h"

#include <ITAException.h>
#include <ITAStreamPatchbay.h>

ITAStreamYJunction::ITAStreamYJunction(unsigned int uiOutputs, ITADatasource* pdsInput)
: m_pImpl(NULL) {
	if (uiOutputs == 0)
		ITA_EXCEPT1(INVALID_PARAMETER, "The number of outputs must at least be one");

	m_pImpl = new ITAStreamPatchbay(pdsInput->GetSamplerate(), pdsInput->GetBlocklength());
	m_pImpl->AddInput(pdsInput->GetNumberOfChannels());
	m_pImpl->SetInputDatasource(0, pdsInput);
	for (unsigned int i=0; i<uiOutputs; i++) {
		m_pImpl->AddOutput(pdsInput->GetNumberOfChannels());
		for (unsigned int j=0; j<pdsInput->GetNumberOfChannels(); j++)
			m_pImpl->ConnectChannels(0, j, i, j);
	}
}

ITAStreamYJunction::~ITAStreamYJunction() {
	delete m_pImpl;
}

ITADatasource* ITAStreamYJunction::GetInputDatasource() {
	return m_pImpl->GetInputDatasource(0);
}

void ITAStreamYJunction::SetInputDatasource(ITADatasource* pdsInput) {
	m_pImpl->SetInputDatasource(0, pdsInput);
}

unsigned int ITAStreamYJunction::GetNumberOfOutputs() {
	return m_pImpl->GetNumOutputs();
}

ITADatasource* ITAStreamYJunction::GetOutputDatasource(unsigned int uiIndex) {
	return m_pImpl->GetOutputDatasource(uiIndex);
}
