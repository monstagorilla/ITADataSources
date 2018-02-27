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
    
    CVS     24.9.2008    - Anpassung Buildkonfiguration: Jetzt standardm��ig Static Lib
	                       Zus�tzlich spezielle Projektdatei f�r alte DLL-Konfiguration
					     - Wichtiges Bugfix in einer Konvertierungsroutine f�r 32 LSB.

	CVS		1.8.2005	 - Erweiterung: Abspielen von Nullbl�cken nach bei ITAsioStop()
	                     - Bugfix: SetPlaybackDatasource(NULL) repariert (Nullzeigerproblem)

    CVS     30.11.2005   - Bugfix: Anzahl an Eingangsbl�cken wird verworfen
	                     - Bugfix: Reset in ASIOSource

	2.1		9.6.2005	 - Umstellung auf MS-VC7
	                     - Nun statische Bibliothek - Keine DLL mehr
						 - std::strings wieder gegen const char*
						   getauscht, wegen Problem mit std::string & DLL

    X.XX	18.1.2005	 ...
      
		
    Todo: - Wie werden Parameter-Konflikte zwischen den Einstellungen im ITAsioInterface
	        (Samplerate, Anzahl der Kan�le, Puffergr��e) und der ausgew�hlten Datenquelle
			behandelt? Hier fehlt noch eine Systematik.

*/                        

/*! \mainpage ITAsioInterface
 *
 * \section intro_sec Einleitung
 *
 * Das ITAsioInterface ist eine Schnittstelle zwischen ASIO und den ITADatasources.
 * Es stellt Funktionen bereit um ASIO-Streaming �ber ASIO-Hardware mittels der
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
 * Das ITAsioInterface ist als dynamische C++-Bibliothek (DLL) verf�gbar.
 * Um Sie zu verwenden ben�tigen sie die folgenden Verzeichnisse: 
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
 * ben�tigen Sie die vollst�ndigen Quellen. Dies beeinhaltet folgende Verzeichnisse:
 *
 *  - Doc (Dokumentation)
 *  - Include (Headerdateien)
 *  - Lib (DLL und Bibliothek)
 *  - Src (Quellcodes)
 *  - Temp (Tempor�res Build-Verzeichnis)
 *  - Tests (Testprogramme)
 *
 * Im Verzeichnis Sources\\ITAsioInterface befinden sich die Quellen f�r die Bibliothek 
 * (Verwenden Sie den Visual C++ Arbeitbeitsbereich ITAsioInterface.sln im Basisverzeichnis).
 *
 * \section test_sec Testen
 *
 * Im Verzeichnis Sources\\Testsuite befinden sich die Komponententests f�r das Modul
 * (Verwenden Sie den Visual C++ Arbeitbeitsbereich Testsuite.dsw). Diese sind jeweils
 * Kommandozeilenprogramme welche zum manuellen Testen (d.h. �berpr�fung der Testergebnisse
 * durch den Tester selbst) verwendet werden k�nnen.
 *
 * \section doc_compile_sec Erstellen der Dokumentation
 *
 * F�r diesen Schritt ben�tigen Sie doxygen. F�hren Sie im Doc-Verzeichnis
 * die Batchdatei makedoc.bat aus.
 * Die Dokumentation wird erstellt und in das Verzeichnis Doc\\HTML gelegt.
 *
 * Wenn Sie Fragen zum ITAsioInterface haben, k�nnen Sie jederzeit die
 * Autoren per eMail kontaktieren:
 *
 *   Tobias Lentz (tle@akustik.rwth-aachen.de)
 *   Frank Wefers (Frank.Wefers@web.de)
 *
 */

// Vorw�rtsdeklarationen
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

//! Anzahl der verf�gbaren ASIO-Treiber zur�ckgeben
/**
 * \return Anzahl der verf�gbaren ASIO-Treiber
 */
ITASIO_API long ITAsioGetNumDrivers(void);

//! Name eines ASIO-Treibers zur�ckgeben
/**
 * Diese Funktion gibt den Namen eines ASIO-Treibers zur�ck
 *
 * \param lDriverNr Nummer des Treibers (G�ltige Werte: 0..N-1 bei N Treibern im System)
 *
 * \return Name des Treiber
 *
 * \note Wenn Sie einen ung�ltigen Index �bergeben,
 *       wird eine leere Zeichenkette zur�ckgegeben
 */
ITASIO_API const char* ITAsioGetDriverName(long lDriverNr);

//! Zeichenkette eines ASIO-Fehlers zur�ckgeben
/**
 * Diese Funktion gibt den Namen eines ASIO-Fehlers zur�ck
 *
 * \param 
 *
 * \return Name des Treiber
 *
 * \note Wenn Sie einen ung�ltigen Index �bergeben,
 *       wird eine leere Zeichenkette zur�ckgegeben
 */
ITASIO_API const char* ITAsioGetErrorStr(ASIOError ae);

//! ASIO-Treiber initialisieren
/**
 * Initialisiert einen ASIO-Treiber f�r die Benutzung.
 *
 * \param lDriverNr Nummer zu initialisierenden Treibers (G�ltige Werte: 0..N-1 bei N Treibern im System)
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioInitializeDriver(long lDriverNr);

//! ASIO-Treiber initialisieren
/**
 * Initialisiert einen ASIO-Treiber f�r die Benutzung.
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
 * \note Abh�ngig vom aktuellen Zustand impliziert der Aufruf der Funktionen
 *       den Aufruf der Funktion(en) ASIOStop und/oder ASIODisposeBuffers.
 */
ITASIO_API ASIOError ITAsioFinalizeDriver(void);

//! Anzahl der Ein-/Ausgabekan�le bestimmen
/**
 * \param numInputChannels   Zeiger auf die Variable, in der Anzahl der Eingabekan�le abgelegt wird
 * \param numOutputChannels  Zeiger auf die Variable, in der Anzahl der Ausgabekan�le abgelegt wird
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioGetChannels(long *numInputChannels, long *numOutputChannels);

//! Puffergr��en des ASIO-Treibers bestimmen
/**
 * Diese Funktion ermittelt Informationen �ber die Puffergr��en des ASIO-Treibers und
 * speichert diese in die durch Zeiger angegebenen Zielvariablen.
 *
 * \param minSize        Zeiger auf die Variable, in der die minimale Puffergr��e abgelegt wird
 * \param maxSize        Zeiger auf die Variable, in der die maximale Puffergr��e abgelegt wird
 * \param preferredSize  Zeiger auf die Variable, in der die bevorzugte Puffergr��e abgelegt wird
 * \param granularity    Zeiger auf die Variable, in der die Granularit�t abgelegt wird
 *
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note �berlicherweise ist die Puffergr��e eine Potenz zur Basis 2 (128, 256, ...).
 *       In diesem Falle ist die Granularit�t -1. G�ltige Puffergr��en sind dann
 *       aller Zweierpotenzen im Bereich minSize .. maxSize.
 *
 * \note Wenn die minimale und maximale Puffergr��e identisch sind, so mu� die
 *       bevorzugte Puffergr��e den selben Wert haben. In diesem Falle ist die
 *       Granularit�t meist 0.
 */
ITASIO_API ASIOError ITAsioGetBufferSize(long *minSize,
										 long *maxSize,
										 long *preferredSize,
										 long *granularity);

//! Ein- und Ausgabelatenzen bestimmen
/**
 * Diese Funktion ermittelt Ein- und Ausgabelatenzzeiten des ASIO-Treibers/-Ger�ts und
 * speichert diese in die durch Zeiger angegebenen Zielvariablen. Die Latenzen enthalten
 * sowohl die Verz�gerung durch die Pufferung (Puffergr��e) als auch Ger�te-interne
 * Latenzen (durch Daten�bertragung, Elektronik, u.A.). Die Latenzen werden in Samples
 * angegeben. Es ist allerdings Hersteller-Sache wie pr�zise diese Angaben sind. Man sollte
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

//!	Testen ob eine Samplerate unterst�tzt wird
/**
 * Testet ob der ASIO-Treiber eine Samplerate unterst�tzt.
 *
 * \param sampleRate Samplerate (in Hertz)
 * \return ASE_OK, falls die Samplerate unterst�tzt wird,
 *         ASE_NoClock, falls die Samplerate nicht unterst�tzt wird
 *
 */
ITASIO_API ASIOError ITAsioCanSampleRate(ASIOSampleRate sampleRate);

//!	Aktuelle Samplerate zur�ckgeben
/**
 * Gibt die aktuelle Samplerate des ASIO-Treibers zur�ck.
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

//! Ausgabeverst�rkung setzen
/**
 * Setzt die Ausgabeverst�rkung (Standardwert: +1.0)
 *
 * \param dGain Ausgabeverst�rkungsfaktor
 */
ITASIO_API void ITAsioSetGain(double dGain);

//! Ausgabedatenquelle festlegen
/**
 * Mit dieser Funktion wird die Datenquelle festgelegt, von welcher die
 * Audiodaten f�r die Ausgabe (Wiedergabe) bezogen werden.
 *
 * \param pidsDatasource Zeiger auf die Datenquelle
 * \return ASE_OK wenn kein Fehler auftrat
 *
 * \note Die �bergabe eines Nullzeigers als Parameter ist erlaubt und
 *       bewirkt das keine Ausgabedatenquelle festgelegt wird.
 *
 * \note Die Parameter der Datenquelle (Anzahl der Kan�le, Samplerate, Blockl�nge)
 *       m�ssen zu den beim ITAsioInterface eingestellten Parametern passen.
 *       Bei Konflikten scheitert die Funktion und gibt ASE_InvalidParameter zur�ck.
 *
 * \note Diese Funktion kann erst genutzt werden, wenn ITAsioCreateBuffers durchgef�hrt
 *       wurde, d.h. mindestens der Laufzeitzustand PREPARED vorliegt.
 */
ITASIO_API ASIOError ITAsioSetPlaybackDatasource(ITADatasource* pidsDatasource);

//! Eingabedatenquelle zur�ckgeben
/**
 * Diese Funktion gibt einen Zeiger auf die Eingabedatenquelle zur�ck,
 * mittels welcher das Modul die aufgenommenen Audiodaten bereitstellt.
 *
 * \return Zeiger auf die Eingabedatenquelle
 *
 * \note Sind mittels ITAsioCreateBuffers keine Eingabekan�le konfiguriert
 *       wurden, gibt die Funktion den Nullzeiger zur�ck
 * 
 * \note Diese Funktion kann erst genutzt werden, wenn ITAsioCreateBuffers durchgef�hrt
 *       wurde, d.h. mindestens der Laufzeitzustand PREPARED vorliegt.
 */
ITASIO_API ITADatasource* ITAsioGetRecordDatasource();

//!ASIO-Puffer erzeugen
/**
 * Erzeugt die ASIO-Puffer.
 *
 * \param lNumberInputChannels Anzahl der Eingabekan�le
 * \param lNumberOutputChannels Anzahl der Ausgabekan�le
 * \param lBufferSize Puffergr��e in Samples
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
 * Startet das Streaming der Ein- und Ausgabekan�le synchron.
 *
 * \return ASE_OK wenn kein Fehler auftrat
 */
ITASIO_API ASIOError ITAsioStart(void);

//! ASIO-Streaming stopppen
/**
 * Stoppt das Streaming der Ein- und Ausgabekan�le synchron.
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
