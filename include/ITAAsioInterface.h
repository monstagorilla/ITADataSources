/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2018
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

#ifndef INCLUDE_WATCHER_ITA_ASIO_INTERFACE
#define INCLUDE_WATCHER_ITA_ASIO_INTERFACE

#include <ITADataSourcesDefinitions.h>

// ASIO-Includes
//#include <asiosys.h>
#include <common/asio.h>

#include <string>

#define ITASIO_API ITA_DATA_SOURCES_API

/*
    Versionshistorie:
    
    CVS     24.9.2008    - Anpassung Buildkonfiguration: Jetzt standardmäßig Static Lib
	                       Zusätzlich spezielle Projektdatei für alte DLL-Konfiguration
					     - Wichtiges Bugfix in einer Konvertierungsroutine für 32 LSB.

	CVS		1.8.2005	 - Erweiterung: Abspielen von Nullblöcken nach bei ITAsioStop()
	                     - Bugfix: SetPlaybackDatasource(NULL) repariert (Nullzeigerproblem)

    CVS     30.11.2005   - Bugfix: Anzahl an Eingangsblöcken wird verworfen
	                     - Bugfix: Reset in ASIOSource

	2.1		9.6.2005	 - Umstellung auf MS-VC7
	                     - Nun statische Bibliothek - Keine DLL mehr
						 - std::strings wieder gegen const char*
						   getauscht, wegen Problem mit std::string & DLL

    X.XX	18.1.2005	 ...
      
		
    Todo: - Wie werden Parameter-Konflikte zwischen den Einstellungen im ITAsioInterface
	        (Samplerate, Anzahl der Kanäle, Puffergröße) und der ausgewählten Datenquelle
			behandelt? Hier fehlt noch eine Systematik.

*/                        

/*! \mainpage ITAsioInterface
 *
 * \section intro_sec Einleitung
 *
 * Das ITAsioInterface ist eine Schnittstelle zwischen ASIO und den ITADatasources.
 * Es stellt Funktionen bereit um ASIO-Streaming über ASIO-Hardware mittels der
 * ITADatasources zu realisieren.
 *
 * \section diffs_sec Unterschiede zum alten ITAsioInterface (<=2004)
 * 
 *  - Microsoft Visual Studio 7.0 Portierung
 *  - Dokumentation der Funktionen auf doxygen umgestellt
 *  - Einstige Erweiterung "BufferswitchCallback" entfernt
 *  - Die Funktion "ITAsioSetWriteDataSource" wurde entfernt,
 *    da ab sofort Eingangsdatenquellen seitens des ITAsioInterface
 *	  bereitgestellt werden.
 *
 *  - Neue Funktionen: ITAsioGetDriverName (Einzelnamen ermitteln)
 *  
 *  - Folgende Funktionen sind in Folge der Umstrukturierung entfernt wurden
 *
 *    ITAsioPreInit (Initialisierung geschieht nun automatisch)
 * 
 * \section install_sec Verwendung
 *
 * Das ITAsioInterface ist als dynamische C++-Bibliothek (DLL) verfügbar.
 * Um Sie zu verwenden benötigen sie die folgenden Verzeichnisse: 
 * <b>\\Include</b> (Headerdateien) sowie <b>\\Lib</b> (DLL und Bibliothek).
 *
 * Die Bibliothek gibt es in zwei Varianten:
 *
 *  - Mit Debugging-Informationen, Debug ("ITAsioInterface.lib")
 *  - Ohne Debugging-Informationen, Release ("ITAsioInterfaceD.lib")
 *
 * Das Laufzeitmodell ist Multithreaded-DLL (MD) bzw. Multithreaded-DLL Debuggen (MDd).
 * Die Dokumentation des Moduls befindet sich im Verzeichnis \\Include.
 *
 * \section compile_sec Erstellen
 *
 * Um das ITAsioInterface selbst aus den Quellen zu Erstellen (Compilieren) 
 * benötigen Sie die vollständigen Quellen. Dies beeinhaltet folgende Verzeichnisse:
 *
 *  - Doc (Dokumentation)
 *  - Include (Headerdateien)
 *  - Lib (DLL und Bibliothek)
 *  - Src (Quellcodes)
 *  - Temp (Temporäres Build-Verzeichnis)
 *  - Tests (Testprogramme)
 *
 * Im Verzeichnis Sources\\ITAsioInterface befinden sich die Quellen für die Bibliothek 
 * (Verwenden Sie den Visual C++ Arbeitbeitsbereich ITAsioInterface.sln im Basisverzeichnis).
 *
 * \section test_sec Testen
 *
 * Im Verzeichnis Sources\\Testsuite befinden sich die Komponententests für das Modul
 * (Verwenden Sie den Visual C++ Arbeitbeitsbereich Testsuite.dsw). Diese sind jeweils
 * Kommandozeilenprogramme welche zum manuellen Testen (d.h. Überprüfung der Testergebnisse
 * durch den Tester selbst) verwendet werden können.
 *
 * \section doc_compile_sec Erstellen der Dokumentation
 *
 * Für diesen Schritt benötigen Sie doxygen. Führen Sie im Doc-Verzeichnis
 * die Batchdatei makedoc.bat aus.
 * Die Dokumentation wird erstellt und in das Verzeichnis Doc\\HTML gelegt.
 *
 * Wenn Sie Fragen zum ITAsioInterface haben, können Sie jederzeit die
 * Autoren per eMail kontaktieren:
 *
 *   Tobias Lentz (tle@akustik.rwth-aachen.de)
 *   Frank Wefers (Frank.Wefers@web.de)
 *
 */

// Vorwärtsdeklarationen
class ITADatasource;

/**
 * \defgroup main ITAsioInterface
 *
 * Das ITAsioInterface hat den selben Zustandsautomat wie ASIO:
 * \image html states.png "ASIO-Zustandsautomat"
 * Nach dem Laden der DLL durch Windows befindet sich das ITAsioInterface im Zustand LOADED.
 */
/*@{*/

//! ASIO-Bibliothek initialisieren 
/**
 * Initialisiert die Bibliothek. Muss als erstes aufgerufen werden.
 */
ITASIO_API void ITAsioInitializeLibrary(void);

//! ASIO-Bibliothek finalisieren 
/**
 * Finalisiert die Bibliothek. Muss als erstes aufgerufen werden.
 */
ITASIO_API void ITAsioFinalizeLibrary(void);

//! Anzahl der verfügbaren ASIO-Treiber zurückgeben
/**
 * \return Anzahl der verfügbaren ASIO-Treiber
 */
ITASIO_API long ITAsioGetNumDrivers(void);

//! Name eines ASIO-Treibers zurückgeben
/**
 * Diese Funktion gibt den Namen eines ASIO-Treibers zurück
 *
 * \param lDriverNr Nummer des Treibers (Gültige Werte: 0..N-1 bei N Treibern im System)
 *
 * \return Name des Treiber
 *
 * \note Wenn Sie einen ungültigen Index übergeben,
 *       wird eine leere Zeichenkette zurückgegeben
 */
ITASIO_API const char* ITAsioGetDriverName(long lDriverNr);

//! Zeichenkette eines ASIO-Fehlers zurückgeben
/**
 * Diese Funktion gibt den Namen eines ASIO-Fehlers zurück
 *
 * \param 
 *
 * \return Name des Treiber
 *
 * \note Wenn Sie einen ungültigen Index übergeben,
 *       wird eine leere Zeichenkette zurückgegeben
 */
ITASIO_API const char* ITAsioGetErrorStr(ASIOError ae);

//! ASIO-Treiber initialisieren
/**
 * Initialisiert einen ASIO-Treiber für die Benutzung.
 *
 * \param lDriverNr Nummer zu initialisierenden Treibers (Gültige Werte: 0..N-1 bei N Treibern im System)
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioInitializeDriver(long lDriverNr);

//! ASIO-Treiber initialisieren
/**
 * Initialisiert einen ASIO-Treiber für die Benutzung.
 *
 * \param pszDriverName Namen des zu initialisierenden Treibers
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioInitializeDriver( const char* pszDriverName );
inline ASIOError ITAsioInitializeDriver( const std::string& sDriverName )
{
	return ITAsioInitializeDriver( sDriverName.c_str() );
};

//! ASIO-Treiber freigeben
/**
 * Gibt einen mittels ITAsioInit initialisierten ASIO-Treiber wieder frei.
 *
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Abhängig vom aktuellen Zustand impliziert der Aufruf der Funktionen
 *       den Aufruf der Funktion(en) ASIOStop und/oder ASIODisposeBuffers.
 */
ITASIO_API ASIOError ITAsioFinalizeDriver(void);

//! Anzahl der Ein-/Ausgabekanäle bestimmen
/**
 * \param numInputChannels   Zeiger auf die Variable, in der Anzahl der Eingabekanäle abgelegt wird
 * \param numOutputChannels  Zeiger auf die Variable, in der Anzahl der Ausgabekanäle abgelegt wird
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioGetChannels(long *numInputChannels, long *numOutputChannels);

//! Puffergrößen des ASIO-Treibers bestimmen
/**
 * Diese Funktion ermittelt Informationen über die Puffergrößen des ASIO-Treibers und
 * speichert diese in die durch Zeiger angegebenen Zielvariablen.
 *
 * \param minSize        Zeiger auf die Variable, in der die minimale Puffergröße abgelegt wird
 * \param maxSize        Zeiger auf die Variable, in der die maximale Puffergröße abgelegt wird
 * \param preferredSize  Zeiger auf die Variable, in der die bevorzugte Puffergröße abgelegt wird
 * \param granularity    Zeiger auf die Variable, in der die Granularität abgelegt wird
 *
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Überlicherweise ist die Puffergröße eine Potenz zur Basis 2 (128, 256, ...).
 *       In diesem Falle ist die Granularität -1. Gültige Puffergrößen sind dann
 *       aller Zweierpotenzen im Bereich minSize .. maxSize.
 *
 * \note Wenn die minimale und maximale Puffergröße identisch sind, so muß die
 *       bevorzugte Puffergröße den selben Wert haben. In diesem Falle ist die
 *       Granularität meist 0.
 */
ITASIO_API ASIOError ITAsioGetBufferSize(long *minSize,
										 long *maxSize,
										 long *preferredSize,
										 long *granularity);

//! Ein- und Ausgabelatenzen bestimmen
/**
 * Diese Funktion ermittelt Ein- und Ausgabelatenzzeiten des ASIO-Treibers/-Geräts und
 * speichert diese in die durch Zeiger angegebenen Zielvariablen. Die Latenzen enthalten
 * sowohl die Verzögerung durch die Pufferung (Puffergröße) als auch Geräte-interne
 * Latenzen (durch Datenübertragung, Elektronik, u.A.). Die Latenzen werden in Samples
 * angegeben. Es ist allerdings Hersteller-Sache wie präzise diese Angaben sind. Man sollte
 * sich also nicht unbedingt darauf verlassen.
 *
 * \param inputLatency   Zeiger auf die Variable, in der die Eingabelatenzzeit [in Samples] abgelegt wird
 * \param outputLatency  Zeiger auf die Variable, in der die Ausgabelatenzzeit [in Samples] abgelegt wird
 *
 * \return ASE wenn kein Fehler auftrat
 *
 * \note Die Funktion darf erst nach ITAsioCreateBuffers aufgerufen werden.
 */
ITASIO_API ASIOError ITAsioGetLatencies(long *inputLatency, long *outputLatency);

//!	Testen ob eine Samplerate unterstützt wird
/**
 * Testet ob der ASIO-Treiber eine Samplerate unterstützt.
 *
 * \param sampleRate Samplerate (in Hertz)
 * \return ASE_OK, falls die Samplerate unterstützt wird,
 *         ASE_NoClock, falls die Samplerate nicht unterstützt wird
 *
 */
ITASIO_API ASIOError ITAsioCanSampleRate(ASIOSampleRate sampleRate);

//!	Aktuelle Samplerate zurückgeben
/**
 * Gibt die aktuelle Samplerate des ASIO-Treibers zurück.
 *
 * \param currentRate Zeiger auf die Variable, in der die Samplerate abgelegt wird
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Falls die Samplerate unbekannt ist, gibt die Funktion ASE_NoClock und
 *       in currentRate wird der Wert 0 abgelegt.
 */
ITASIO_API ASIOError ITAsioGetSampleRate(ASIOSampleRate *currentRate);

//! Samplerate setzen
/**
 * Setzt die Samplerate des ASIO-Treibers.
 *
 * \param sampleRate Samplerate (in Hertz)
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Der Parameter sampleRate == 0 schaltet auf externe Synchronisation
 */
ITASIO_API ASIOError ITAsioSetSampleRate(ASIOSampleRate sampleRate);

//! Ausgabeverstärkung setzen
/**
 * Setzt die Ausgabeverstärkung (Standardwert: +1.0)
 *
 * \param dGain Ausgabeverstärkungsfaktor
 */
ITASIO_API void ITAsioSetGain(double dGain);

//! Ausgabedatenquelle festlegen
/**
 * Mit dieser Funktion wird die Datenquelle festgelegt, von welcher die
 * Audiodaten für die Ausgabe (Wiedergabe) bezogen werden.
 *
 * \param pidsDatasource Zeiger auf die Datenquelle
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Die Übergabe eines Nullzeigers als Parameter ist erlaubt und
 *       bewirkt das keine Ausgabedatenquelle festgelegt wird.
 *
 * \note Die Parameter der Datenquelle (Anzahl der Kanäle, Samplerate, Blocklänge)
 *       müssen zu den beim ITAsioInterface eingestellten Parametern passen.
 *       Bei Konflikten scheitert die Funktion und gibt ASE_InvalidParameter zurück.
 *
 * \note Diese Funktion kann erst genutzt werden, wenn ITAsioCreateBuffers durchgeführt
 *       wurde, d.h. mindestens der Laufzeitzustand PREPARED vorliegt.
 */
ITASIO_API ASIOError ITAsioSetPlaybackDatasource(ITADatasource* pidsDatasource);

//! Eingabedatenquelle zurückgeben
/**
 * Diese Funktion gibt einen Zeiger auf die Eingabedatenquelle zurück,
 * mittels welcher das Modul die aufgenommenen Audiodaten bereitstellt.
 *
 * \return Zeiger auf die Eingabedatenquelle
 *
 * \note Sind mittels ITAsioCreateBuffers keine Eingabekanäle konfiguriert
 *       wurden, gibt die Funktion den Nullzeiger zurück
 * 
 * \note Diese Funktion kann erst genutzt werden, wenn ITAsioCreateBuffers durchgeführt
 *       wurde, d.h. mindestens der Laufzeitzustand PREPARED vorliegt.
 */
ITASIO_API ITADatasource* ITAsioGetRecordDatasource();

//!ASIO-Puffer erzeugen
/**
 * Erzeugt die ASIO-Puffer.
 *
 * \param lNumberInputChannels Anzahl der Eingabekanäle
 * \param lNumberOutputChannels Anzahl der Ausgabekanäle
 * \param lBufferSize Puffergröße in Samples
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \sa ITAsioGetChannels
 * \sa ITAsioGetBufferSize
 */
ITASIO_API ASIOError ITAsioCreateBuffers(long lNumberInputChannels,
										 long lNumberOutputChannels,
										 long lBufferSize);

//! ASIO-Puffer freigeben
/**
 * Gibt alle mittels ITAsioCreateBuffers erzeugten ASIO-Puffer wieder frei.
 *
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Der Aufruf dieser Funktion bei laufenem
 *       ASIO-Streaming impliziert ITAsioStop()
 */
ITASIO_API ASIOError ITAsioDisposeBuffers(void);

//! ASIO-Streaming starten
/**
 * Startet das Streaming der Ein- und Ausgabekanäle synchron.
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioStart(void);

//! ASIO-Streaming stopppen
/**
 * Stoppt das Streaming der Ein- und Ausgabekanäle synchron.
 *
 * \return ASE_OK if ok.
 */
ITASIO_API ASIOError ITAsioStop(void);

//! ASIO-Kontrollfeld anzeigen
/**
 * Diese Funktion zeigt das ASIO-Kontrollfeld an.
 */
ITASIO_API ASIOError ITAsioControlPanel(void);

#endif // INCLUDE_WATCHER_ITA_ASIO_INTERFACE
