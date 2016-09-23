/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2016
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
// $Id: ITADataSourceRealization.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_DATA_SOURCE_REALIZATION
#define INCLUDE_WATCHER_ITA_DATA_SOURCE_REALIZATION

#include <ITADataSourcesDefinitions.h>

#include <ITADataSource.h>
#include <ITAStreamProperties.h>
#include <ITAAtomicPrimitives.h>

// Vorw�rtsdeklarationen
class ITADatasourceRealizationEventHandler;

//! Gr�ndger�st f�r die Implementierung von Datenquellen
/**
 * Die Klasse ITADatasourceRealization ist ein Grundger�st f�r die Implementierung
 * von Datenquellen. Sie bietet dem Entwickler einen Einstiegspunkt eigene
 * Datenquellen zu konstruieren und fa�t einige grundlegende Funktionalit�t
 * zusammen. ITADatasourceRealization realisiert die abstrakten Methoden
 * der Oberklasse ITADatasource und bietet dar�ber hinaus konkrete Methoden
 * f�r das Bereitstellen von Daten seitens der Datenquelle.
 *
 * Intern enth�lt ein ITADatasourceRealization einen Puffer in dem die 
 * freizusetzenden Daten zwischengespeichert werden. Dieser Puffer
 * kann nach beliebig dimensioniert werden (f�r die Thread-Sicherheit mu� er
 * aber mindestens die Gr��e 2 haben). Die Klasse stellt Methoden bereit
 * um Zeiger auf diesen Puffer abzurufen und diese zu inkrementieren (analog
 * der publizierten Schnittstelle von ITADatasource). Dabei werden komplexe
 * Aspekte der Thread-Sicherheit ber�cksichtigt und m�ssen nicht vom Entwickler
 * selbst programmiert werden.
 *
 * \ingroup datasources
 */

class ITA_DATA_SOURCES_API ITADatasourceRealization : public ITADatasource {
public:
	//! Konstruktor
	/**
	 * \param uiChannels    Anzahl der Kan�le
	 * \param uiBlocklength Blockl�nge (in Samples)
	 * \param uiCapacity    Gr��e des internen Puffers in Anzahl Bl�cken
	 *                      (Optional, Standardwert: 2, Minimalwert: 2)
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgel�st.
	 */
	/*
	ITADatasourceRealization(unsigned int uiChannels,
		                     unsigned int uiBlocklength,
				             unsigned int uiCapacity=2);
	*/

	//! Konstruktor
	/**
	 * \param uiChannels    Anzahl der Kan�le
	 * \param dSamplerate	Abtastrate (in Hertz)
	 * \param uiBlocklength Blockl�nge (in Samples)
	 * \param uiCapacity    Gr��e des internen Puffers in Anzahl Bl�cken
	 *                      (Optional, Standardwert: 2, Minimalwert: 2)
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgel�st.
	 */
	ITADatasourceRealization(unsigned int uiChannels,
							 double dSamplerate,
		                     unsigned int uiBlocklength,
				             unsigned int uiCapacity=2);

	virtual ~ITADatasourceRealization();

	//! -= Zur�cksetzen =-
	/**
	 * \important Darf nur aufgerufen werden, wenn das Streaming nicht l�uft!
	 */
	virtual void Reset();

	//! Gibt zur�ck, ob Fehler w�hrend der Stream-Verarbeitung auftraten
	bool HasStreamErrors() const;

	//! Handler f�r Stream-Event setzen
	/**
	 * \important Darf nur aufgerufen werden, wenn das Streaming nicht l�uft!
	 */
	ITADatasourceRealizationEventHandler* GetStreamEventHandler() const;

	//! Handler f�r Stream-Event setzen
	/**
	 * \important Darf nur aufgerufen werden, wenn das Streaming nicht l�uft!
	 */
	void SetStreamEventHandler(ITADatasourceRealizationEventHandler* pHandler);

	// -= Realisierung der abstrakten Methoden von "ITADatasource" =-

	const ITAStreamProperties* GetStreamProperties() const { return &m_oStreamProps; }
	unsigned int GetBlocklength() const { return m_uiBlocklength; }
	unsigned int GetNumberOfChannels() const { return m_uiChannels; }
	double GetSampleRate() const { return m_dSampleRate; }

    virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);
	virtual void IncrementBlockPointer();

	//! Schreibzeiger abrufen
	/**
	 * Diese Methode gibt den Zeiger die aktuelle Schreibposition im internen Puffer zur�ck.
	 * Der R�ckgabewert ist niemals NULL.
	 *
	 * \param uiChannel Index des Kanals (Wertebereich: [0, Kan�le-1])
	 * \return Zeiger auf die aktuelle Position des internen Puffers
	 */
	virtual float* GetWritePointer(unsigned int uiChannel);
	
	//! Schreibzeiger inkrementieren
	/**
	 * Der Aufruf dieser Methode inkrementiert den internen Schreibzeiger.
	 * Dadurch stellt die Datenquelle die n�chsten Daten bereit. Wenn Sie die
	 * Schreiboperationen auf den Ausgabebl�cken der Datenquelle abgeschlossen
	 * haben, <b>m�ssen</b> Sie diese Methode aufrufen, um diese Daten durch die
	 * Datenquelle bereitzustellen.
	 */ 
	virtual void IncrementWritePointer();

	//! Nachrichten-Methode
	/*
	 * Wird aufgerufen, wenn GetBlockPointer vom Stream aufgerufen wird,
	 * noch bevor intern Daten verarbeitert werden (also noch vor ProcessStream)
	 */
	virtual void PreGetBlockPointer() {};

	//! Nachrichten-Methode
	/*
	 * Wird aufgerufen, nachdem IncrementBlockPointer vom Stream aufgerufen wurde.
	 */
	virtual void PostIncrementBlockPointer() {};

	//! Verarbeitungsmethode
	/**
	 * Diese Hook-Methode wird von der Klasse aufgerufen, wenn neue Stream-Daten
	 * produziert werden sollen. Unterklassen sollten diese Methode redefinieren, 
	 * um die Verarbeitung von Samples zu realisieren.
	 */
	virtual void ProcessStream(const ITAStreamInfo* ) {};

protected:
	
	/*
	 *  [fwe] Der Einfachheit halber werden diese Variablen hier
	 *        protected zug�nglich gemacht. Dies erspart h�ufige
	 *        Tipparbeit in Unterklassen. Bitte diese Werte aber
	 *        nur lesend [read-only] benutzen!!
	 */
	double m_dSampleRate;				// Abtastrate [Hz]
	unsigned int m_uiChannels;			// Anzahl Kan�le
	unsigned int m_uiBlocklength;		// Streaming Puffergr��e [Samples]
	
private:
	ITAStreamProperties m_oStreamProps;	// Audiostream-Parameter

	unsigned int m_uiBufferSize;		// Puffergr��e
	unsigned int m_uiReadCursor;		// Leseposition
	unsigned int m_uiWriteCursor;		// Schreibposition

	float* m_pfBuffer;					// Puffer (Kan�le interleaved!)
	ITAAtomicInt m_iGBPEntrances;		// Anzahl paralleler Eintritte in GBP
	ITAAtomicBool m_bGBPFirst;			// Erster Eintritt in GBP seit letztem IBP (=> Daten produzieren)
	int m_iBufferUnderflows;			// DEBUG: Z�hler f�r Buffer-Leerl�ufe
	int m_iBufferOverflows;				// DEBUG: Z�hler f�r Buffer-�berl�ufe
	int m_iGBPReentrances;				// DEBUG: Z�hler parallele Wiedereintritte in GBP
	int m_iIBPReentrances;				// DEBUG: Z�hler parallele Wiedereintritte in IBP
	ITADatasourceRealizationEventHandler* m_pEventHandler;	// Handler f�r Stream-Events

	void Init(unsigned int uiChannels,
			  unsigned int uiBlocklength,
			  unsigned int uiCapacity);
};

//! Schnittstelle f�r Nachrichten-Verarbeitung der Klasse ITADatasourceRealization
class ITA_DATA_SOURCES_API ITADatasourceRealizationEventHandler {
public:
	virtual ~ITADatasourceRealizationEventHandler() {};

	virtual void HandlePreGetBlockPointer(ITADatasourceRealization* pSender, unsigned int uiChannel);
	virtual void HandlePostIncrementBlockPointer(ITADatasourceRealization* pSender);
	virtual void HandleProcessStream(ITADatasourceRealization* pSender, const ITAStreamInfo* pStreamInfo);
};

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCE_REALIZATION
