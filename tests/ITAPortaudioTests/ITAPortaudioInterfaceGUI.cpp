#include <fx.h>
#include <ITAPortaudioInterface.h>
#include <ITAFileDatasource.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamPatchbay.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAStringUtils.h>
#include <vector>

// Static settings used to instantiate ITAPortaudioInterface to fetch information
static double dSampleRate = 44.1e3;
static int iBlockSize = 256;

class TestWindow : public FXMainWindow {
FXDECLARE(TestWindow)
public:
	// Messages
	enum {
		ID_TESTOUTPUTDEVICE = FXMainWindow::ID_LAST,
		ID_SELECTOUTPUTDEVICE,
		ID_SELECTINPUTDEVICE,
		ID_TAB,
		ID_LAST
	};

	TestWindow(FXApp* app);
	~TestWindow() {};

	void create() {
		FXMainWindow::create();
		show(PLACEMENT_SCREEN);
	}

	// Events
	long onTestOutputDevice(FXObject*, FXSelector, void*);
	long onSelectOutputDevice(FXObject*, FXSelector, void*);

private:
	TestWindow(){};

	// Other
	std::vector<int> m_viOutputDevices;
	std::vector<int> m_viInputDevices;
	ITAPortaudioInterface* m_pITAPA;
	
	// Gui
	FXComboBox* m_pOutputDevicesCombobox;
	FXComboBox* m_pInputDevicesCombobox;
	FXVerticalFrame* m_pITAPAOutputDevicesFrame;
	FXVerticalFrame* m_pITAPAOutputDeviceInfo;
	FXTextField* m_pInputDeviceInfoNameTextField;
	FXTextField* m_pInputDeviceIdentifierTextField;
	FXTextField* m_pInputDeviceInfoSampleRateTextField;

};

// Message map for main window
FXDEFMAP(TestWindow) TestWindowMap[]= {
	// Initialization
	FXMAPFUNC(SEL_COMMAND,	TestWindow::ID_TESTOUTPUTDEVICE,	TestWindow::onTestOutputDevice),
	FXMAPFUNC(SEL_COMMAND,	TestWindow::ID_SELECTOUTPUTDEVICE,	TestWindow::onSelectOutputDevice)
};

FXIMPLEMENT(TestWindow, FXMainWindow, TestWindowMap, ARRAYNUMBER(TestWindowMap));

// Construct the GUI
TestWindow::TestWindow(FXApp* app)
: FXMainWindow(app, "ITAPortaudioInterfaceGUI", NULL, NULL, DECOR_ALL | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0,0,500,380, 10,10,10,10, 10,10),
m_pITAPAOutputDeviceInfo(NULL), m_pITAPA(NULL)
{

	// --= GUI =--

	// Tab book
	FXTabBook* m_pTabBook = new FXTabBook(this, this, ID_TAB, LAYOUT_FILL, 0,0,0,0, 0,0,DEFAULT_SPACING,DEFAULT_SPACING);

	// Output devices tab
	FXTabItem* pITAPAOutputDevicesTab = new FXTabItem(m_pTabBook, "&Output devices", NULL, TAB_TOP_NORMAL, 0,0,0,0, 10,10,2,5);
	m_pITAPAOutputDevicesFrame = new FXVerticalFrame(m_pTabBook, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL);
	m_pOutputDevicesCombobox = new FXComboBox(m_pITAPAOutputDevicesFrame, 70, this, ID_SELECTOUTPUTDEVICE, FRAME_GROOVE);
	m_pITAPAOutputDeviceInfo = new FXVerticalFrame(m_pITAPAOutputDevicesFrame, LAYOUT_FILL);
	m_pInputDeviceIdentifierTextField = new FXTextField(m_pITAPAOutputDeviceInfo, 60, this, ID_SELECTINPUTDEVICE, FRAME_RAISED);
	m_pInputDeviceInfoNameTextField = new FXTextField(m_pITAPAOutputDeviceInfo, 60, this, ID_SELECTINPUTDEVICE, FRAME_RAISED);
	m_pInputDeviceInfoSampleRateTextField = new FXTextField(m_pITAPAOutputDeviceInfo, 60, this, ID_SELECTINPUTDEVICE, FRAME_RAISED);

	// Input devices tab
	FXTabItem* pITAPAInputDevicesTab = new FXTabItem(m_pTabBook, "&Input devices", NULL, TAB_TOP_NORMAL, 0,0,0,0, 10,10,2,5);
	FXVerticalFrame* pITAPAInputDevicesFrame = new FXVerticalFrame(m_pTabBook, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL);
	

	// Initialize ITAPortaudioInterface to fetch information on devices
	m_pITAPA = new ITAPortaudioInterface(dSampleRate, iBlockSize);
	ITAPortaudioInterface::ITA_PA_ERRORCODE iError = m_pITAPA->Initialize();

	m_viOutputDevices.clear();
	for (int i=0; i<m_pITAPA->GetNumDevices(); i++) {
		if (m_pITAPA->GetNumOutputChannels(i) > 0)
			m_viOutputDevices.push_back(i);
		if (m_pITAPA->GetNumInputChannels(i) > 0)
			m_viInputDevices.push_back(i);
	}

	int iDefaultOutputDevice = m_pITAPA->GetDefaultOutputDevice();
	int iDefaultInputDevice = m_pITAPA->GetDefaultInputDevice();

	// Outputs
	m_pOutputDevicesCombobox->setNumVisible(std::min((int) m_viOutputDevices.size(), 12));

	int iDefaultOutputComboboxItem;
	for (int i=0; i<(int) m_viOutputDevices.size(); i++) {
		int iDeviceID = m_viOutputDevices[i];
		std::string sDeviceItem = "[" + IntToString(iDeviceID) + "] ";
		if (iDeviceID == iDefaultOutputDevice) {
			sDeviceItem += "* ";
			iDefaultOutputComboboxItem = i;
		}
		sDeviceItem += m_pITAPA->GetDeviceName(iDeviceID);
		sDeviceItem += " [channels: " + IntToString(m_pITAPA->GetNumOutputChannels(iDeviceID)) + "]";
		m_pOutputDevicesCombobox->appendItem(sDeviceItem.c_str());
	}

	FXButton* pTestOutputDecviceButton = new FXButton(m_pITAPAOutputDevicesFrame, "Playback test ...", NULL, this, ID_TESTOUTPUTDEVICE);

	// Inputs
	m_pInputDevicesCombobox = new FXComboBox(pITAPAInputDevicesFrame, 70, this, ID_SELECTINPUTDEVICE, FRAME_GROOVE);
	m_pInputDevicesCombobox->setNumVisible(std::min((int) m_viInputDevices.size(), 12));

	int iDefaultInputComboboxItem;
	for (int i=0; i<(int) m_viInputDevices.size(); i++) {
		int iDeviceID = m_viInputDevices[i];
		std::string sDeviceItem = "";
		if (iDeviceID == iDefaultInputDevice) {
			sDeviceItem += "* ";
			iDefaultInputComboboxItem = i;
		}
		sDeviceItem += IntToString(iDeviceID) + ": '";
		sDeviceItem += m_pITAPA->GetDeviceName(iDeviceID);
		sDeviceItem += "' [channels: " + IntToString(m_pITAPA->GetNumInputChannels(iDeviceID)) + "]";
		m_pInputDevicesCombobox->appendItem(sDeviceItem.c_str());
	}
	
	m_pITAPA->Finalize();
	m_pITAPA = NULL;

	// Send messages
	m_pOutputDevicesCombobox->setCurrentItem(iDefaultInputComboboxItem, true);
	m_pInputDevicesCombobox->setCurrentItem(iDefaultOutputComboboxItem, true);
}

long TestWindow::onSelectOutputDevice(FXObject*, FXSelector, void*) {
	int iCurrentItem = m_pOutputDevicesCombobox->getCurrentItem();
	int iDriverID = m_viOutputDevices[iCurrentItem];

	m_pITAPA = new ITAPortaudioInterface(dSampleRate, iBlockSize);
	m_pITAPA->Initialize(iDriverID);

	std::string sDeviceID = "Device identifier: " + IntToString(iDriverID);
	m_pInputDeviceIdentifierTextField->setText(sDeviceID.c_str(), true);

	std::string sDeviceName = "Device name: " + m_pITAPA->GetDeviceName(iDriverID);
	m_pInputDeviceInfoNameTextField->setText(sDeviceName.c_str(), true);

	std::string sSampleRate = "Current sample rate: " + DoubleToString(m_pITAPA->GetSampleRate());
	m_pInputDeviceInfoSampleRateTextField->setText(sSampleRate.c_str(), true);

	m_pITAPA->Finalize();

	return 1;
}

long TestWindow::onTestOutputDevice(FXObject*, FXSelector, void*) {

	int iCurrentItem = m_pOutputDevicesCombobox->getCurrentItem();
	int iDriverID = m_viOutputDevices[iCurrentItem];

	m_pITAPA->Initialize(iDriverID);
	int iNumInputChannels = m_pITAPA->GetNumOutputChannels(iDriverID);
	int iNumOutputChannels = m_pITAPA->GetNumOutputChannels(iDriverID);

	if (iNumOutputChannels <= 0)
		return -1;
		
	std::vector<ITADatasource*> vInputSignals;
	ITAStreamPatchbay patchbay(dSampleRate, iBlockSize);
	patchbay.AddOutput(iNumOutputChannels);

	double fFundamentalFrequency = 161;
	for (int i=0; i<iNumOutputChannels; i++) {
		patchbay.AddInput(1);
		vInputSignals.push_back(new ITAStreamFunctionGenerator(1, dSampleRate, iBlockSize, ITAStreamFunctionGenerator::SAWTOOTH, (2*0+1)*fFundamentalFrequency, 1, true));
		patchbay.SetInputDatasource(i, vInputSignals[i]);
		patchbay.ConnectChannels(i, 0, 0, i);
	}

	ITAStreamMultiplier1N multiplier(vInputSignals[0], iNumOutputChannels);
	
	ITADatasource* pOutputTail = &multiplier; // patchbay.GetOutputDatasource(0);

	ITAPortaudioInterface::ITA_PA_ERRORCODE err;
	if ((err = m_ITAPA->SetPlaybackDatasource(pOutputTail) != ITAPortaudioInterface::ITA_PA_NO_ERROR) {
		FXMessageBox msg(this->getApp(), "Error", m_pITAPA->GetErrorCodeString(err).c_str(), NULL);
		msg.execute();
	}	

	if ((err = m_pITAPA->Open()) != ITAPortaudioInterface::ITA_PA_NO_ERROR) {
		FXMessageBox msg(this->getApp(), "Error", m_pITAPA->GetErrorCodeString(err).c_str(), NULL);
		msg.execute();
	} else {
		if ((err = m_pITAPA->Start()) != ITAPortaudioInterface::ITA_PA_NO_ERROR) {
			FXMessageBox msg(this->getApp(), "Error", m_pITAPA->GetErrorCodeString(err).c_str(), NULL);
			msg.execute();
		} else {
			m_pITAPA->Sleep(3);
			m_pITAPA->Stop();
		}
	}
	m_pITAPA->Close();
	m_pITAPA->Finalize();

	return 1;
}

int main(int argc,char *argv[]) {
	FXApp application(L"ITAPortaudioInterfaceGUI", L"ITAPortaudioInterfaceGUI");
	application.init(argc,argv);
	new TestWindow(&application);
	application.create();
	application.run();

	return 0;
}