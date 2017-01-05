/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2017
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

#ifndef INCLUDE_WATCHER_ITA_BUFFER_DATA_SOURCE
#define INCLUDE_WATCHER_ITA_BUFFER_DATA_SOURCE

#include <ITADataSourcesDefinitions.h>
          
#include <ITADataSource.h>
#include <ITAAtomicPrimitives.h>

#include <vector>

//! Puffer-basierte Datenquelle
/**
 * Die Klasse ITABufferDatasource realisiert Datenquellen, welche
 * Audiodaten aus sich im Speicher befindlichen Puffern freisetzen.
 * Diese Puffer sind ein oder mehrere float-Arrays, welche jeweils die
 * Audiodaten eines Kanals enthalten. Die Verwaltung der Puffer �bernimmt
 * die Klasse NICHT; Sie nutzt diese nur zum Freisetzen von Daten.
 *
 * Da die Audiodaten in Bl�cken einer festen Blockl�nge freigesetzt werden,
 * legt ITABufferDatasource die Kapazit�t als n�chstkleineres Blockl�nge-Vielfaches
 * der Buffergr��e (uiBufferSize) fest.
 *
 * Die Klasse bietet ferner die M�glichkeit einen bestimmten Bereich der Puffer 
 * f�r das Freisetzen der Audiodaten zu benutzen, den sogenannten Arbeitsbereich
 * (<i>region of interest, ROI</i>). Zwei Wiedergabemodi werden unterst�tzt:
 * Einmaliges Freisetzen/Abspielen und Wiederholung.
 *
 * �ber den gesch�tzen Konstruktor und die gesch�tzte Init-Methode k�nnen von
 * ITABufferDatasource abgeleitete Klasse eine verz�gert Initialisierung und damit
 * Festgelegung auf konkrete Puffer und Parameter realisieren. Dies ist notwendig,
 * wenn beim Aufruf des ITABufferDatasource-Konstruktors diese Parameter noch nicht 
 * bekannt sind. Instanzen m�ssen dann mittels der Init-Methode initialisiert werden.
 *
 * \important <b>Thread-Safety</b>: Ist in dieser Implementierung nicht konsequent durchgezogen.
 *            Es gibt einige potentielle Schwachstellen. Durch den Vorrang von Leistungaspekten
 *            und einem speziellen Einsatzschema der Klasse, beidem Thread-safety ausserhalb
 *            der Klasse realisiert wird, wurde auf eine strikte Thread-sichere Implementierung zun�chst
 *            verzichtet. But remember: <b>You have been warned!</b> :-P [fwe: Dezember 2005]
 * 
 * \ingroup datasources
 */
class ITA_DATA_SOURCES_API ITABufferDatasource : public ITADatasource {
public:
	//! Konstruktor (2D-Array)
	/**
	 * \param ppfBuffer Array von float-Zeigern auf die Datenpuffer der einzelnen Kan�le
	 * \param uiChannels Anzahl der Kan�le
	 * \param uiBuffersize Gr��e der Kanalpuffer (Anzahl der Samples)
	 * \param dSamplerate Abtastrate mit der die Datenquelle arbeiten soll
	 * \param uiBlocklength Blockl�nge mit der die Datenquelle arbeiten soll
	 * \param bLoopMode Wiederholungsmodus? [Optional]
	 *
	 * \note Bei ung�ltigen Parametern wird eine Ausnahme vom Typ LLCException ausgel�st
	 */
	ITABufferDatasource(const float** ppfBuffer,
		                unsigned int uiChannels,
					    unsigned int uiBuffersize,
						double dSamplerate,
		                unsigned int uiBlocklength,
						bool bLoopMode=false);	

	//! Konstruktor (Vektor von Arrays)
	/**
	 * \param vpfBuffer Vektor von float-Zeigern auf die Datenpuffer der einzelnen Kan�le
	 * \param uiBuffersize Gr��e der Kanalpuffer (Anzahl der Samples)
	 * \param dSamplerate Abtastrate mit der die Datenquelle arbeiten soll
	 * \param uiBlocklength Blockl�nge mit der die Datenquelle arbeiten soll
	 * \param bLoopMode Wiederholungsmodus? [Optional]
	 *
	 * \note Die Kanalanzahl wird aus der Anzahl der Elemente im Vektor bestimmt
	 * \note Bei ung�ltigen Parametern wird eine Ausnahme vom Typ LLCException ausgel�st
	 */
	ITABufferDatasource(const std::vector<float*>& vpfBuffer,
					    unsigned int uiBuffersize,
						double dSamplerate,
		                unsigned int uiBlocklength,
						bool bLoopMode=false);	

	//! Spezieller Konstruktor (Einkanal-Betrieb)
	/**
	 * Dies ist ein spezieller Konstruktor f�r Datenquellen mit nur einem Kanal.
	 *
	 * \param pfBuffer Zeiger auf den Datenpuffer
	 * \param uiBuffersize Gr��e des Datenpuffers (Anzahl der Samples)
	 * \param dSamplerate Abtastrate mit der die Datenquelle arbeiten soll
	 * \param uiBlocklength Blockl�nge mit der die Datenquelle arbeiten soll
	 * \param bLoopMode Wiederholungsmodus? [Optional]
	 *
	 * \note Bei ung�ltigen Parametern wird eine Ausnahme vom Typ LLCException ausgel�st
	 */
	ITABufferDatasource(const float* pfBuffer,
					    unsigned int uiBuffersize,
						double dSamplerate,
		                unsigned int uiBlocklength,
						bool bLoopMode=false);	

	//! Kapazit�t der Datenquelle zur�ckgeben
	/** 
	 * Gibt die Anzahl der Samples der Daten der Quelle zur�ck.
	 * Dieser Wert ist das n�chstkleinere Blockl�ngen-Vielfache der 
	 * Pufferl�nge uiBuffersize und mu� somit nicht unbedingt dem
	 * wert uiBuffersize entsprechen. Er ist unabh�ngig von der
	 * Wahl des Start- und Endcursors und kann als Obergrenze f�r
	 * den Endcursor verwendet werden.
	 */
	unsigned int GetCapacity();

	//! Aktuelle Wiedergabeposition bezogen auf die gesamten Quellendaten zur�ckgeben
	/**
	 * Gibt den relativen Cursor zum Beginn der Quellendaten (Offset 0) zur�ck.
	 *
	 * \note Dieser Wert ist NICHT IMMER ein Vielfaches der Blockl�nge.
	 *       Dies h�ngt davon ab, ob der Startgrenze der ROI ein Vielfaches
	 *       der Blockl�nge ist.
	 */
	unsigned int GetAbsoluteCursor();

	//! Relative Wiedergabeposition bezogen auf den Arbeitsbereich zur�ckgeben
	unsigned int GetCursor();

	//! Wiedergabeposition setzen bezogen auf den Arbeitsbereich
	void SetCursor(unsigned int uiNewCursor);

	//! Wiedergabe pausiert?
	bool IsPaused() const;
	
	//! Pausierung f�r Wiedergabe ein-/ausschalten
	void SetPaused(bool bPaused);

	//! Zur�ckgeben ob die Wiederholung eingeschaltet ist
	bool GetLoopMode();

	//! Wiederholung Ein-/Ausschalten
	void SetLoopMode(bool bLoopMode);

	//! Arbeitsbereich (region of interest) festlegen
	/**
	 * Legt den Arbeitsbereich fest, d.h. das Interval in den Quellendaten, aus dem die
	 * Quelle ihre Daten freisetzt. Dabei stellt uiStartOffset die Nummer des Samples
	 * dar, an dem der Arbeitsbereich beginnt. Dieser Wert mu� NICHT unbedingt ein Vielfaches der
	 * Bl�ckl�nge sein und darf frei gew�hlt werden. Der Wert uiEndOffset ist die Nummer 
	 * des ersten Samples, welches nicht mehr im Arbeitsbereich enthalten ist.
	 * Somit ist definieren beide Werte das HALBOFFENE Interval [uiStartOffset, uiEndOffset). 
	 * Die Datenquelle setzt also Audiodaten aus dem Bereich [uiStartOffset, uiEndOffset-1]
	 * frei. Das Ende des Arbeitsbereiches uiEndOffset wird von der Methode so
	 * angepasst, das die Differenz uiEndOffset-uiStartOffset das n�chstkleinere Vielfache
	 * der Blockl�nge ergibt.
	 */
	void SetROI(unsigned int uiStartOffset, unsigned int uiEndOffset);

	//! Startposition des Arbeitsbereiches (region of interest) zur�ckgeben
	unsigned int GetROIStart();

	//! Endposition des Arbeitsbereiches (region of interest) zur�ckgeben
	unsigned int GetROIEnd();

	//! L�nge der Arbeitsbereiches (region of interest) zur�ckgeben
	/**
	 * Gibt die L�nge des Arbeitsbereiches (region of interest) zur�ck.
	 * Dies ist die Differenz von End- und Startcursor. Der Wert ist
	 * immer ein Vielfaches der Blockl�nge.
	 */
	unsigned int GetROILength();

	//! Zur�ckspulen
	/**
	 * Spult die Datenquelle an den Beginn der Daten zur�ck.
	 * Der als n�chstes freigesetzten Daten sind wieder die Daten
	 * vom Anfang der Puffer.
	 */
	void Rewind();

	// -= Realisierte Methoden von ITADatasource =-

	unsigned int GetBlocklength() const;
	unsigned int GetNumberOfChannels() const;
	double GetSampleRate() const;

    const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);	
	void IncrementBlockPointer();

protected:
	//! Gesch�tzer Konstruktor
	/**
	 * Dieser Konstruktor kann von abgeleiteten Klassen benutzt werden um eine
	 * Instanz von ITABufferDatasource zu erzeugen, welche noch nicht auf einen
	 * konkreten Puffer und konkrete Parameter festgelegt ist (z.B. f�r die
	 * Spezialisierung ITAFileDatasource ist dies notwendig).
	 *
	 * \important Auf jedenfall muss die Instanz sp�ter mit der Methode Init
	 *            initialisiert werden.
	 */
	ITABufferDatasource();

	//! Initialisieren
	/**
	 * Zu benutzen im Verbund mit dem gesch�tzen Konstruktor.
	 */
	void Init(const std::vector<float*>& vpfBuffer,
		      unsigned int uiBuffersize,
			  double dSamplerate,
			  unsigned int uiBlocklength,
			  bool bLoopMode);

private:
	unsigned int m_uiBuffersize;		// Gr��e der einzelnen Kanalpuffer
	std::vector<const float*> m_vpfBuffer;	// Vektor der Kanalpuffer

	double m_dSamplerate;				// Abtastrate [Hz]
	unsigned int m_uiChannels;			// Anzahl Kan�le
	unsigned int m_uiBlocklength;		// Streaming Puffergr��e [Samples]

	unsigned int m_uiCapacity;			// Kapazit�t der Quelle
	ITAAtomicInt m_iCursor;				// Leseposition

	ITAAtomicBool m_bPaused;			// Wiedergabezustand
	ITAAtomicBool m_bPausedInternal;	// Wiedergabezustand (interne Variable)
	bool m_bLoopMode, m_bNewLoopMode;	// Wiederholungsmodusm, neuer Wiederholungsmodus
	bool m_bChangeLoopMode;				// true = Wiederholungsmodus soll ge�ndert werden
	bool m_bRewind;						// Als n�chstes Zur�ckspulen?
	bool m_bChangeROI;					// true = Start-/Endcursor soll ge�ndert werden
	bool m_bInit;						// Instanz initialisiert?
	bool m_bGetBlockPointerTouched;		// Seit letztem IncrementBlockPointer bereits ein GetBlockPointer aufgerufen?
	unsigned int m_uiROIStart, m_uiROIEnd;
	unsigned int m_uiNewROIStart, m_uiNewROIEnd;

	// Zustands�nderungen verarbeiten ggf. Cursor anpassen
	// (R�ckgabewert: false falls neuer Cursorstand bereits gesetzt und
	//                keine weitere Bearbeitung des Cursors mehr notwendig)
	bool Update();
};

#endif // INCLUDE_WATCHER_ITA_BUFFER_DATA_SOURCE