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

// Vorwärtsdeklarationen
class ITADatasourceRealizationEventHandler;

//! Gründgerüst für die Implementierung von Datenquellen
/**
 * Die Klasse ITADatasourceRealization ist ein Grundgerüst für die Implementierung
 * von Datenquellen. Sie bietet dem Entwickler einen Einstiegspunkt eigene
 * Datenquellen zu konstruieren und faßt einige grundlegende Funktionalität
 * zusammen. ITADatasourceRealization realisiert die abstrakten Methoden
 * der Oberklasse ITADatasource und bietet darüber hinaus konkrete Methoden
 * für das Bereitstellen von Daten seitens der Datenquelle.
 *
 * Intern enthält ein ITADatasourceRealization einen Puffer in dem die 
 * freizusetzenden Daten zwischengespeichert werden. Dieser Puffer
 * kann nach beliebig dimensioniert werden (für die Thread-Sicherheit muß er
 * aber mindestens die Größe 2 haben). Die Klasse stellt Methoden bereit
 * um Zeiger auf diesen Puffer abzurufen und diese zu inkrementieren (analog
 * der publizierten Schnittstelle von ITADatasource). Dabei werden komplexe
 * Aspekte der Thread-Sicherheit berücksichtigt und müssen nicht vom Entwickler
 * selbst programmiert werden.
 *
 * \ingroup datasources
 */

class ITA_DATA_SOURCES_API ITADatasourceRealization : public ITADatasource
{
public:
	//! Konstruktor
	/**
	 * \param uiChannels    Anzahl der Kanäle
	 * \param uiBlocklength Blocklänge (in Samples)
	 * \param uiCapacity    Größe des internen Puffers in Anzahl Blöcken
	 *                      (Optional, Standardwert: 2, Minimalwert: 2)
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgelöst.
	 */
	/*
	ITADatasourceRealization(unsigned int uiChannels,
		                     unsigned int uiBlocklength,
				             unsigned int uiCapacity=2);
	*/

	//! Konstruktor
	/**
	 * \param uiChannels    Anzahl der Kanäle
	 * \param dSamplerate	Abtastrate (in Hertz)
	 * \param uiBlocklength Blocklänge (in Samples)
	 * \param uiCapacity    Größe des internen Puffers in Anzahl Blöcken
	 *                      (Optional, Standardwert: 2, Minimalwert: 2)
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgelöst.
	 */
	ITADatasourceRealization(unsigned int uiChannels,
							 double dSamplerate,
		                     unsigned int uiBlocklength,
				             unsigned int uiCapacity=2);

	virtual ~ITADatasourceRealization();

	//! -= Zurücksetzen =-
	/**
	 * \important Darf nur aufgerufen werden, wenn das Streaming nicht läuft!
	 */
	virtual void Reset();

	//! Gibt zurück, ob Fehler während der Stream-Verarbeitung auftraten
	bool HasStreamErrors() const;

	//! Handler für Stream-Event setzen
	/**
	 * \important Darf nur aufgerufen werden, wenn das Streaming nicht läuft!
	 */
	ITADatasourceRealizationEventHandler* GetStreamEventHandler() const;

	//! Handler für Stream-Event setzen
	/**
	 * \important Darf nur aufgerufen werden, wenn das Streaming nicht läuft!
	 */
	void SetStreamEventHandler(ITADatasourceRealizationEventHandler* pHandler);

	// -= Realisierung der abstrakten Methoden von "ITADatasource" =-

	inline const ITAStreamProperties* GetStreamProperties() const { return &m_oStreamProps; }
	inline unsigned int GetBlocklength() const { return m_uiBlocklength; }
	inline unsigned int GetNumberOfChannels() const { return m_uiChannels; }
	inline double GetSampleRate() const { return m_dSampleRate; }

    virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);
	virtual void IncrementBlockPointer();

	//! Schreibzeiger abrufen
	/**
	 * Diese Methode gibt den Zeiger die aktuelle Schreibposition im internen Puffer zurück.
	 * Der Rückgabewert ist niemals NULL.
	 *
	 * \param uiChannel Index des Kanals (Wertebereich: [0, Kanäle-1])
	 * \return Zeiger auf die aktuelle Position des internen Puffers
	 */
	virtual float* GetWritePointer(unsigned int uiChannel);
	
	//! Schreibzeiger inkrementieren
	/**
	 * Der Aufruf dieser Methode inkrementiert den internen Schreibzeiger.
	 * Dadurch stellt die Datenquelle die nächsten Daten bereit. Wenn Sie die
	 * Schreiboperationen auf den Ausgabeblöcken der Datenquelle abgeschlossen
	 * haben, <b>müssen</b> Sie diese Methode aufrufen, um diese Daten durch die
	 * Datenquelle bereitzustellen.
	 */ 
	virtual void IncrementWritePointer();

	//! Nachrichten-Methode
	/*
	 * Wird aufgerufen, wenn GetBlockPointer vom Stream aufgerufen wird,
	 * noch bevor intern Daten verarbeitert werden (also noch vor ProcessStream)
	 */
	inline virtual void PreGetBlockPointer() {};

	//! Nachrichten-Methode
	/*
	 * Wird aufgerufen, nachdem IncrementBlockPointer vom Stream aufgerufen wurde.
	 */
	inline virtual void PostIncrementBlockPointer() {};

	//! Verarbeitungsmethode
	/**
	 * Diese Hook-Methode wird von der Klasse aufgerufen, wenn neue Stream-Daten
	 * produziert werden sollen. Unterklassen sollten diese Methode redefinieren, 
	 * um die Verarbeitung von Samples zu realisieren.
	 */
	inline virtual void ProcessStream(const ITAStreamInfo* ) {};

protected:
	
	/*
	 *  [fwe] Der Einfachheit halber werden diese Variablen hier
	 *        protected zugänglich gemacht. Dies erspart häufige
	 *        Tipparbeit in Unterklassen. Bitte diese Werte aber
	 *        nur lesend [read-only] benutzen!!
	 */
	double m_dSampleRate;				// Abtastrate [Hz]
	unsigned int m_uiChannels;			// Anzahl Kanäle
	unsigned int m_uiBlocklength;		// Streaming Puffergröße [Samples]
	
private:
	ITAStreamProperties m_oStreamProps;	// Audiostream-Parameter

	unsigned int m_uiBufferSize;		// Puffergröße
	unsigned int m_uiReadCursor;		// Leseposition
	unsigned int m_uiWriteCursor;		// Schreibposition

	float* m_pfBuffer;					// Puffer (Kanäle interleaved!)
	ITAAtomicInt m_iGBPEntrances;		// Anzahl paralleler Eintritte in GBP
	ITAAtomicBool m_bGBPFirst;			// Erster Eintritt in GBP seit letztem IBP (=> Daten produzieren)
	int m_iBufferUnderflows;			// DEBUG: Zähler für Buffer-Leerläufe
	int m_iBufferOverflows;				// DEBUG: Zähler für Buffer-Überläufe
	int m_iGBPReentrances;				// DEBUG: Zähler parallele Wiedereintritte in GBP
	int m_iIBPReentrances;				// DEBUG: Zähler parallele Wiedereintritte in IBP
	ITADatasourceRealizationEventHandler* m_pEventHandler;	// Handler für Stream-Events

	void Init(unsigned int uiChannels,
			  unsigned int uiBlocklength,
			  unsigned int uiCapacity);
};

//! Schnittstelle für Nachrichten-Verarbeitung der Klasse ITADatasourceRealization
class ITA_DATA_SOURCES_API ITADatasourceRealizationEventHandler
{
public:
	inline virtual ~ITADatasourceRealizationEventHandler() {};

	virtual void HandlePreGetBlockPointer(ITADatasourceRealization* pSender, unsigned int uiChannel);
	virtual void HandlePostIncrementBlockPointer(ITADatasourceRealization* pSender);
	virtual void HandleProcessStream(ITADatasourceRealization* pSender, const ITAStreamInfo* pStreamInfo);
};

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCE_REALIZATION
