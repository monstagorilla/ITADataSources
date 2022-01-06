/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2021
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

#include <ITADataSource.h>
#include <ITADataSourcesDefinitions.h>
#include <vector>

//! Puffer-basierte Datenquelle
/**
 * Die Klasse ITABufferDatasource realisiert Datenquellen, welche
 * Audiodaten aus sich im Speicher befindlichen Puffern freisetzen.
 * Diese Puffer sind ein oder mehrere float-Arrays, welche jeweils die
 * Audiodaten eines Kanals enthalten. Die Verwaltung der Puffer übernimmt
 * die Klasse NICHT; Sie nutzt diese nur zum Freisetzen von Daten.
 *
 * Da die Audiodaten in Blöcken einer festen Blocklänge freigesetzt werden,
 * legt ITABufferDatasource die Kapazität als nächstkleineres Blocklänge-Vielfaches
 * der Buffergröße (uiBufferSize) fest.
 *
 * Die Klasse bietet ferner die Möglichkeit einen bestimmten Bereich der Puffer
 * für das Freisetzen der Audiodaten zu benutzen, den sogenannten Arbeitsbereich
 * (<i>region of interest, ROI</i>). Zwei Wiedergabemodi werden unterstützt:
 * Einmaliges Freisetzen/Abspielen und Wiederholung.
 *
 * Über den geschützen Konstruktor und die geschützte Init-Methode können von
 * ITABufferDatasource abgeleitete Klasse eine verzögert Initialisierung und damit
 * Festgelegung auf konkrete Puffer und Parameter realisieren. Dies ist notwendig,
 * wenn beim Aufruf des ITABufferDatasource-Konstruktors diese Parameter noch nicht
 * bekannt sind. Instanzen müssen dann mittels der Init-Methode initialisiert werden.
 *
 * \important <b>Thread-Safety</b>: Ist in dieser Implementierung nicht konsequent durchgezogen.
 *            Es gibt einige potentielle Schwachstellen. Durch den Vorrang von Leistungaspekten
 *            und einem speziellen Einsatzschema der Klasse, beidem Thread-safety ausserhalb
 *            der Klasse realisiert wird, wurde auf eine strikte Thread-sichere Implementierung zunächst
 *            verzichtet. But remember: <b>You have been warned!</b> :-P [fwe: Dezember 2005]
 *
 * \ingroup datasources
 */
class ITA_DATA_SOURCES_API ITABufferDatasource : public ITADatasource
{
public:
	//! Konstruktor (2D-Array)
	/**
	 * \param ppfBuffer Array von float-Zeigern auf die Datenpuffer der einzelnen Kanäle
	 * \param uiChannels Anzahl der Kanäle
	 * \param uiBuffersize Größe der Kanalpuffer (Anzahl der Samples)
	 * \param dSamplerate Abtastrate mit der die Datenquelle arbeiten soll
	 * \param uiBlocklength Blocklänge mit der die Datenquelle arbeiten soll
	 * \param bLoopMode Wiederholungsmodus? [Optional]
	 *
	 * \note Bei ungültigen Parametern wird eine Ausnahme vom Typ ITAException ausgelöst
	 */
	ITABufferDatasource( const float** ppfBuffer, unsigned int uiChannels, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength,
	                     bool bLoopMode = false );

	//! Konstruktor (Vektor von Arrays)
	/**
	 * \param vpfBuffer Vektor von float-Zeigern auf die Datenpuffer der einzelnen Kanäle
	 * \param uiBuffersize Größe der Kanalpuffer (Anzahl der Samples)
	 * \param dSamplerate Abtastrate mit der die Datenquelle arbeiten soll
	 * \param uiBlocklength Blocklänge mit der die Datenquelle arbeiten soll
	 * \param bLoopMode Wiederholungsmodus? [Optional]
	 *
	 * \note Die Kanalanzahl wird aus der Anzahl der Elemente im Vektor bestimmt
	 * \note Bei ungültigen Parametern wird eine Ausnahme vom Typ CIAException ausgelöst
	 */
	ITABufferDatasource( const std::vector<float*>& vpfBuffer, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode = false );

	//! Spezieller Konstruktor (Einkanal-Betrieb)
	/**
	 * Dies ist ein spezieller Konstruktor für Datenquellen mit nur einem Kanal.
	 *
	 * \param pfBuffer Zeiger auf den Datenpuffer
	 * \param uiBuffersize Größe des Datenpuffers (Anzahl der Samples)
	 * \param dSamplerate Abtastrate mit der die Datenquelle arbeiten soll
	 * \param uiBlocklength Blocklänge mit der die Datenquelle arbeiten soll
	 * \param bLoopMode Wiederholungsmodus? [Optional]
	 *
	 * \note Bei ungültigen Parametern wird eine Ausnahme vom Typ CITAException ausgelöst
	 */
	ITABufferDatasource( const float* pfBuffer, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode = false );

	//! Kapazität der Datenquelle zurückgeben
	/**
	 * Gibt die Anzahl der Samples der Daten der Quelle zurück.
	 * Dieser Wert ist das nächstkleinere Blocklängen-Vielfache der
	 * Pufferlänge uiBuffersize und muß somit nicht unbedingt dem
	 * wert uiBuffersize entsprechen. Er ist unabhängig von der
	 * Wahl des Start- und Endcursors und kann als Obergrenze für
	 * den Endcursor verwendet werden.
	 */
	unsigned int GetCapacity( );

	//! Aktuelle Wiedergabeposition bezogen auf die gesamten Quellendaten zurückgeben
	/**
	 * Gibt den relativen Cursor zum Beginn der Quellendaten (Offset 0) zurück.
	 *
	 * \note Dieser Wert ist NICHT IMMER ein Vielfaches der Blocklänge.
	 *       Dies hängt davon ab, ob der Startgrenze der ROI ein Vielfaches
	 *       der Blocklänge ist.
	 */
	unsigned int GetAbsoluteCursor( );

	//! Relative Wiedergabeposition bezogen auf den Arbeitsbereich zurückgeben
	unsigned int GetCursor( );

	//! Wiedergabeposition setzen bezogen auf den Arbeitsbereich
	void SetCursor( unsigned int uiNewCursor );

	//! Wiedergabe pausiert?
	bool IsPaused( ) const;

	//! Pausierung für Wiedergabe ein-/ausschalten
	void SetPaused( bool bPaused );

	//! Zurückgeben ob die Wiederholung eingeschaltet ist
	bool GetLoopMode( );

	//! Wiederholung Ein-/Ausschalten
	void SetLoopMode( bool bLoopMode );

	//! Sets the looping mode
	/**
	 * @param[in] bLoopingEnabled True means looping, false will play until EOF
	 */
	void SetIsLooping( bool bLoopingEnabled );

	//! Looping mode getter
	/**
	 * @return True means looping, false will play until EOF
	 */
	bool GetIsLooping( );

	//! Arbeitsbereich (region of interest) festlegen
	/**
	 * Legt den Arbeitsbereich fest, d.h. das Interval in den Quellendaten, aus dem die
	 * Quelle ihre Daten freisetzt. Dabei stellt uiStartOffset die Nummer des Samples
	 * dar, an dem der Arbeitsbereich beginnt. Dieser Wert muß NICHT unbedingt ein Vielfaches der
	 * Blöcklänge sein und darf frei gewählt werden. Der Wert uiEndOffset ist die Nummer
	 * des ersten Samples, welches nicht mehr im Arbeitsbereich enthalten ist.
	 * Somit ist definieren beide Werte das HALBOFFENE Interval [uiStartOffset, uiEndOffset).
	 * Die Datenquelle setzt also Audiodaten aus dem Bereich [uiStartOffset, uiEndOffset-1]
	 * frei. Das Ende des Arbeitsbereiches uiEndOffset wird von der Methode so
	 * angepasst, das die Differenz uiEndOffset-uiStartOffset das nächstkleinere Vielfache
	 * der Blocklänge ergibt.
	 */
	void SetROI( unsigned int uiStartOffset, unsigned int uiEndOffset );

	//! Startposition des Arbeitsbereiches (region of interest) zurückgeben
	unsigned int GetROIStart( );

	//! Endposition des Arbeitsbereiches (region of interest) zurückgeben
	unsigned int GetROIEnd( );

	//! Länge der Arbeitsbereiches (region of interest) zurückgeben
	/**
	 * Gibt die Länge des Arbeitsbereiches (region of interest) zurück.
	 * Dies ist die Differenz von End- und Startcursor. Der Wert ist
	 * immer ein Vielfaches der Blocklänge.
	 */
	unsigned int GetROILength( );

	//! Zurückspulen
	/**
	 * Spult die Datenquelle an den Beginn der Daten zurück.
	 * Der als nächstes freigesetzten Daten sind wieder die Daten
	 * vom Anfang der Puffer.
	 */
	void Rewind( );

	// -= Realisierte Methoden von ITADatasource =-

	unsigned int GetBlocklength( ) const;
	unsigned int GetNumberOfChannels( ) const;
	double GetSampleRate( ) const;

	const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo );
	void IncrementBlockPointer( );

protected:
	//! Geschützer Konstruktor
	/**
	 * Dieser Konstruktor kann von abgeleiteten Klassen benutzt werden um eine
	 * Instanz von ITABufferDatasource zu erzeugen, welche noch nicht auf einen
	 * konkreten Puffer und konkrete Parameter festgelegt ist (z.B. für die
	 * Spezialisierung ITAFileDatasource ist dies notwendig).
	 *
	 * \important Auf jedenfall muss die Instanz später mit der Methode Init
	 *            initialisiert werden.
	 */
	ITABufferDatasource( );

	//! Initialisieren
	/**
	 * Zu benutzen im Verbund mit dem geschützen Konstruktor.
	 */
	void Init( const std::vector<float*>& vpfBuffer, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode );

private:
	unsigned int m_uiBuffersize;           // Größe der einzelnen Kanalpuffer
	std::vector<const float*> m_vpfBuffer; // Vektor der Kanalpuffer

	double m_dSamplerate;         // Abtastrate [Hz]
	unsigned int m_uiChannels;    // Anzahl Kanäle
	unsigned int m_uiBlocklength; // Streaming Puffergröße [Samples]

	unsigned int m_uiCapacity;  // Kapazität der Quelle
	std::atomic<int> m_iCursor; // Leseposition

	std::atomic<bool> m_bPaused;         // Wiedergabezustand
	std::atomic<bool> m_bPausedInternal; // Wiedergabezustand (interne Variable)
	bool m_bLoopMode, m_bNewLoopMode;    // Wiederholungsmodusm, neuer Wiederholungsmodus
	bool m_bChangeLoopMode;              // true = Wiederholungsmodus soll geändert werden
	bool m_bRewind;                      // Als nächstes Zurückspulen?
	bool m_bChangeROI;                   // true = Start-/Endcursor soll geändert werden
	bool m_bInit;                        // Instanz initialisiert?
	bool m_bGetBlockPointerTouched;      // Seit letztem IncrementBlockPointer bereits ein GetBlockPointer aufgerufen?
	unsigned int m_uiROIStart, m_uiROIEnd;
	unsigned int m_uiNewROIStart, m_uiNewROIEnd;

	// Zustandsänderungen verarbeiten ggf. Cursor anpassen
	// (Rückgabewert: false falls neuer Cursorstand bereits gesetzt und
	//                keine weitere Bearbeitung des Cursors mehr notwendig)
	bool Update( );
};

#endif // INCLUDE_WATCHER_ITA_BUFFER_DATA_SOURCE