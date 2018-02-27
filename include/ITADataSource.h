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

#ifndef INCLUDE_WATCHER_ITA_DATA_SOURCE
#define INCLUDE_WATCHER_ITA_DATA_SOURCE

#include <ITADataSourcesDefinitions.h>

#include <ITAAtomicPrimitives.h>

/*
    Versionshistorie:
    
    CVS    30.11.2005    - Mutex statt CriticalSection in DatasourceRealization

    2.07	 9.5.2005	 - Komplette Umstellung des Projektes auf MS-VC++ 7.0
	                     - Austausch der ISPL gegen das ITAToolkit

	2.06	14.2.2005	 Bugfix zu Bug #001 - siehe KNOWN_BUGS

    2.05	 9.2.2005	 - Neue Funktion in den Utils: WriteFromDatasourceToBuffer

	2.04	 8.2.2005    - Grundgerüst für die Implementierung von eigenen
	                       Datenquellen ausgenommen: ITADatasourceRealization
						 - Fortschrittsausgabe in WriteFromDatasourceToFile

	                     - 3 Bugfixes an der Filedatasource:
						 
						   1. Aktualisierung des Loopmode fehlerhaft (Zuweisung)
	                       2. Knackser beim Wiederholungmodus,
							  weil Puffer nicht mit Nullsamples initialisiert.
						   3. Fehlende Zuweisung im Konstruktor fehlte.
						      Zufälliges Ändern des Loopmodus

	2.03	23.1.2005	 Online-/Offline-Modus in der Funktion WriteFromDatasourceToFile
	                     hinzugefügt.

    2.02	18.1.2005	 Warten auf neue Daten in GetBlockPointer() hinzugefügt,
	                     Dokumentation vervollständigt

    2.01    14.1.2005    Samplerate hinzugefügt, sowie GetSampleRate()

    2.00    12.1.2005    Die neue Klasse ITADatasource als verbesserte,
                         bereinigte Version der alt-eingesessenen DataSource-Klasse.
                         Diese neue Klasse arbeitet jetzt nur noch auf dem Datentyp
                         float. Einige Methoden sind entfernt wurden, zwei kamen hinzu:
                         Die Kanalanzahl und die Blocklänge können nun abgefragt werden.
                         Ganz neu im Programm: Dokumentation! :-)                      
*/                        

/*! \mainpage ITADatasources - Datenquellen für das Audio-Streaming
 *
 * \section intro_sec Einleitung
 *
 * Das Modul ITADatasources stellt Klassen und Funktionen für das Streaming von Audiodaten
 * bereit. Es definiert die abstrakte Basisklasse ITADatasource, welche allgemeine 
 * Datenquellen beschreibt. Hinzu kommt eine konkrete Realisierung der ITADatasource, die
 * sogenannte ITAFileDatasource, welche Audiodaten aus einer Audiodatei freisetzt.
 * Die ITADatasources kommen in vielen anderen Projekten zum Einsatz. Beispielsweise
 * Benutzt das ASIO-Modul ITAsioInterface die Datenquellen von ITADatasources für das
 * Audio-Streaming. Die Hilfsfunktion WriteFromDatasourceToFile erlaubt das Umleiten von
 * Daten einer Datenquelle in eine Audiodatei.
 *
 * \section install_sec Verwendung
 *
 * ITADatasources ist als statische C++-Bibliothek verfügbar. Um Sie zu verwenden benötigen
 * sie die folgenden Verzeichnisse: <b>\\Include</b> (Headerdateien) sowie <b>\\Lib</b> (Bibliothek).
 * Die Bibliothek gibt es in zwei Varianten:
 *
 *  - Mit Debugging-Informationen, Debug ("ITADatasourcesD.lib")
 *  - Ohne Debugging-Informationen, Release ("ITADatasources.lib")
 *
 *
 * \section doc_compile_sec Erstellen der Dokumentation
 *
 * Für diesen Schritt benötigen Sie doxygen. 
 * Führen Sie im Basisverzeichnis die Batchdatei makedoc.bat aus.
 * Die Dokumentation wird erstellt und in das Verzeichnis \\Doc gelegt.
 */

class ITAStreamProperties;
class ITAStreamInfo;

/**
 * \defgroup datasources Datenquellen
 */

//! Abstrakte Basisklasse für Datenquellen
/**
 * Die Klasse ITADatasource definiert die abstrakte Schnittstelle für Audiodatenquellen.
 * Diese liefern Audiodaten von (potentiell) mehreren Kanälen in Form von <b>Blöcken</b>
 * zurück. Blöcke sind eindimensionale float-Arrays deren Felder die Samples repräsentieren.
 * Ihre Länge (Anzahl der Elemente) wird mit <b>Blocklänge</b> bezeichnet.
 * Diese ändert sich zur Lebenszeit einer Datenquelle nicht. Auch die Kanalanzahl und
 * die Samplerate einer Datenquelle bleiben während ihrer Lebenszeit konstant, mit 
 * einer Ausnahme: Es kann vorkommen, das die Kanalanzahl und/oder die Samplerate
 * zum Erzeugungszeitpunkt der Datenquelle noch nicht feststeht
 * (sie erhält/erhalten dann den initialen Wert 0). In diesem Falle kann/muß sie später 
 * festgelegt werden. Nach ihrer Festlegung ändern sie sich dann nicht mehr.
 * 
 * Der Datenaustausch über eine Quelle geschieht Abschnitt- und Kanalweise. Als <b>Abschnitt</b>
 * wird hier der Zeitraum bezeichnet, in dem der Nutzer mit den momentan bereitstehenden
 * Blöcken (der einzelnen Kanäle) arbeitet. Sind diese Arbeitsoperationen auf den
 * Blöcken abgeschlossen, muß der Nutzer den nächsten Abschnitt einleiten, um
 * seitens der Datenquelle die nächsten/folgenden Daten zu erhalten. Dies geschieht
 * durch den Aufruf der Methode IncrementBlockPointer.
 * Innerhalb eines Abschnittes kann der Nutzer mittels der Methode GetBlockPointer Zeiger
 * auf die Blöcke der Kanäle der Datenquelle abrufen. Er darf allerdings die Daten dieser
 * Blöcke <b>nicht verändern</b> (siehe const float*).
 * 
 * Der Vorgehensweise für das kontinuierliche Auslesen von Daten aus der Quelle
 * geschieht nach folgendem Schema (Pseudocode):
 *
 * <pre>
 *   while (Datasource.GetBlockPointer(0) != 0) { // Noch Daten verfügbar?
 *     for i from 0 to Datasource.GetNumberOfChannels()-1 do {
 *       BlockPointer = Datasource.GetBlockPointer(i);
 *       // Arbeite mit den Daten ...
 *     }
 *     
 *     // Nächste Eingabeblöcke bereitstellen
 *     Datasource.IncrementBlockPointer();
 *   }
 * </pre>
 *
 * Die Datenquelle kann sich in zwei Zuständen befinden: 1. Es sind Daten verfügbar
 * und 2. es sind keine Daten verfügbar.
 * Im ersten Fall liefert jeder Aufruf der Methode GetBlockPointer einen gültigen Zeiger
 * ungleich dem Nullzeiger zurück. Tritt der zweite Fall auf, so gibt die Methode Nullzeiger
 * zurück. Dieser Nullzeiger-Fall muss immer von benutzenden Modulen entsprechend behandelt werden.
 *
 * \ingroup datasources
 */
class ITA_DATA_SOURCES_API ITADatasource
{
public:
    //! Destruktor
	virtual ~ITADatasource();

	//! Eigenschaften des Audiostreams zurückgeben
	//virtual const ITAStreamProperties* GetStreamProperties() const=0;

	//! Blocklänge zurückgeben
	/**
	 * \return Blocklänge der Datenquelle in Samples
	 */
	virtual unsigned int GetBlocklength() const=0;
	
	//! Anzahl der Kanäle zurückgeben
	/**
	 * \return Anzahl der Kanäle der Datenquelle
	 * 
	 * \note Falls die Anzahl der Kanäle der Datenquelle noch nicht feststeht,
	 *       wird 0 zurückgegeben (siehe Klassenbeschreibung).
	 */
	virtual unsigned int GetNumberOfChannels() const=0;
	
	//! Samplerate zurückgeben
	/**
	 * \return Samplerate der Datenquelle (in Hertz)
	 *
	 * \note Falls die Samplerate der Datenquelle noch nicht feststeht,
	 *       wird 0 zurückgegeben (siehe Klassenbeschreibung).
     */
	virtual double GetSampleRate() const=0;

	//! Blockzeiger abrufen
	/**
	 * Diese Methode gibt den Zeiger auf das float-Array zurück,
	 * in dem der nächsten Block Daten gespeichert ist. Solch ein
	 * Zeiger wird auch Blockzeiger genannt.
	 *
	 * \param uiChannel Index des Kanals (Wertebereich: [0, Kanäle-1])
	 * \param pStreamInfo Zustandsinformationen des Streams (niemals NULL)
	 *
	 * \return Zeiger auf das float-Array mit den nächsten Daten
	 *         des angegebenen Kanals
	 *
	 * \note Falls momentan keine Daten verfügbar sind,
	 *       wird der <b>Nullzeiger</b> zurückgegeben.
	 *
	 * \note <b>Testen</b> Sie die <b>Verfügbarkeit von Daten</b> durch den Aufruf 
	 *       der Methode mit einem beliebigen gültigen Kanal und bWaitForData==false.
	 *       Erhalten Sie in diesem Falle den Nullzeiger zurück, sind keine 
	 *       Daten verfügbar.
	 *
	 * \warning Der Aufruf der Methode mit einem ungültigen Index (siehe Wertebereich)
	 *          führt <b>auch</b> zur Rückgabe eines <b>Nullzeigers</b>.
	 */
	virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo)=0;	
	
	//! Blockzeiger inkrementieren
	/**
	 * Der Aufruf dieser Methode inkrementiert den internen Blockzeiger der Datenquelle.
	 * Dadurch stellt die Datenquelle die nächsten Daten bereit. Wenn Sie die
	 * Leseoperationen auf den aktuellen Blöcken der Datenquelle
	 * abgeschlossen haben, <b>müssen</b> Sie diese Methode aufrufen, um seitens 
	 * der Datenquelle die nächsten Daten bereitzustellen.
	 */ 
	virtual void IncrementBlockPointer()=0;

	//! Anzahl der Benutzungen der Datenquelle zurückgeben
	virtual int GetNumReferences();

	//! Entfernt alle Referenzen (setzt Referenzzähler auf 0)
	virtual void ClearReferences();

	//! Benutzung vermerken
	/**
	 * Erlaubt Benutzern der Datenquelle ihre Benutzung zu vermerken.
	 * Die Implementierung erfolg mittels Referenzzählern. Auf diese
	 * einfache Weise kann sichergestellt werden, das eine Datenquelle
	 * nicht gelöscht wird, sofern sie noch benutzt wird.
	 *
	 * \return Anzahl der Referenzen nach dem Hinzufügen
	 */
	virtual int AddReference();

	//! Benutzung vermerken
	/**
	 * Erlaubt Benutzern der Datenquelle ihre Benutzung zu vermerken.
	 * Die Implementierung erfolg mittels Referenzzählern. Auf diese
	 * einfache Weise kann sichergestellt werden, das eine Datenquelle
	 * nicht gelöscht wird, sofern sie noch benutzt wird.
	 *
	 * \return Anzahl der Referenzen nach dem Entfernen
	 */
	virtual int RemoveReference();

protected:
	//! Geschützter Standardkonstruktor
	ITADatasource();

private:
	ITAAtomicInt m_iRefCount;
};

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCE
