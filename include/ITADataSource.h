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

#include <atomic>

/*
    Versionshistorie:
    
    CVS    30.11.2005    - Mutex statt CriticalSection in DatasourceRealization

    2.07	 9.5.2005	 - Komplette Umstellung des Projektes auf MS-VC++ 7.0
	                     - Austausch der ISPL gegen das ITAToolkit

	2.06	14.2.2005	 Bugfix zu Bug #001 - siehe KNOWN_BUGS

    2.05	 9.2.2005	 - Neue Funktion in den Utils: WriteFromDatasourceToBuffer

	2.04	 8.2.2005    - Grundger�st f�r die Implementierung von eigenen
	                       Datenquellen ausgenommen: ITADatasourceRealization
						 - Fortschrittsausgabe in WriteFromDatasourceToFile

	                     - 3 Bugfixes an der Filedatasource:
						 
						   1. Aktualisierung des Loopmode fehlerhaft (Zuweisung)
	                       2. Knackser beim Wiederholungmodus,
							  weil Puffer nicht mit Nullsamples initialisiert.
						   3. Fehlende Zuweisung im Konstruktor fehlte.
						      Zuf�lliges �ndern des Loopmodus

	2.03	23.1.2005	 Online-/Offline-Modus in der Funktion WriteFromDatasourceToFile
	                     hinzugef�gt.

    2.02	18.1.2005	 Warten auf neue Daten in GetBlockPointer() hinzugef�gt,
	                     Dokumentation vervollst�ndigt

    2.01    14.1.2005    Samplerate hinzugef�gt, sowie GetSampleRate()

    2.00    12.1.2005    Die neue Klasse ITADatasource als verbesserte,
                         bereinigte Version der alt-eingesessenen DataSource-Klasse.
                         Diese neue Klasse arbeitet jetzt nur noch auf dem Datentyp
                         float. Einige Methoden sind entfernt wurden, zwei kamen hinzu:
                         Die Kanalanzahl und die Blockl�nge k�nnen nun abgefragt werden.
                         Ganz neu im Programm: Dokumentation! :-)                      
*/                        

/*! \mainpage ITADatasources - Datenquellen f�r das Audio-Streaming
 *
 * \section intro_sec Einleitung
 *
 * Das Modul ITADatasources stellt Klassen und Funktionen f�r das Streaming von Audiodaten
 * bereit. Es definiert die abstrakte Basisklasse ITADatasource, welche allgemeine 
 * Datenquellen beschreibt. Hinzu kommt eine konkrete Realisierung der ITADatasource, die
 * sogenannte ITAFileDatasource, welche Audiodaten aus einer Audiodatei freisetzt.
 * Die ITADatasources kommen in vielen anderen Projekten zum Einsatz. Beispielsweise
 * Benutzt das ASIO-Modul ITAsioInterface die Datenquellen von ITADatasources f�r das
 * Audio-Streaming. Die Hilfsfunktion WriteFromDatasourceToFile erlaubt das Umleiten von
 * Daten einer Datenquelle in eine Audiodatei.
 *
 * \section install_sec Verwendung
 *
 * ITADatasources ist als statische C++-Bibliothek verf�gbar. Um Sie zu verwenden ben�tigen
 * sie die folgenden Verzeichnisse: <b>\\Include</b> (Headerdateien) sowie <b>\\Lib</b> (Bibliothek).
 * Die Bibliothek gibt es in zwei Varianten:
 *
 *  - Mit Debugging-Informationen, Debug ("ITADatasourcesD.lib")
 *  - Ohne Debugging-Informationen, Release ("ITADatasources.lib")
 *
 *
 * \section doc_compile_sec Erstellen der Dokumentation
 *
 * F�r diesen Schritt ben�tigen Sie doxygen. 
 * F�hren Sie im Basisverzeichnis die Batchdatei makedoc.bat aus.
 * Die Dokumentation wird erstellt und in das Verzeichnis \\Doc gelegt.
 */

class ITAStreamProperties;
class ITAStreamInfo;

/**
 * \defgroup datasources Datenquellen
 */

//! Abstrakte Basisklasse f�r Datenquellen
/**
 * Die Klasse ITADatasource definiert die abstrakte Schnittstelle f�r Audiodatenquellen.
 * Diese liefern Audiodaten von (potentiell) mehreren Kan�len in Form von <b>Bl�cken</b>
 * zur�ck. Bl�cke sind eindimensionale float-Arrays deren Felder die Samples repr�sentieren.
 * Ihre L�nge (Anzahl der Elemente) wird mit <b>Blockl�nge</b> bezeichnet.
 * Diese �ndert sich zur Lebenszeit einer Datenquelle nicht. Auch die Kanalanzahl und
 * die Samplerate einer Datenquelle bleiben w�hrend ihrer Lebenszeit konstant, mit 
 * einer Ausnahme: Es kann vorkommen, das die Kanalanzahl und/oder die Samplerate
 * zum Erzeugungszeitpunkt der Datenquelle noch nicht feststeht
 * (sie erh�lt/erhalten dann den initialen Wert 0). In diesem Falle kann/mu� sie sp�ter 
 * festgelegt werden. Nach ihrer Festlegung �ndern sie sich dann nicht mehr.
 * 
 * Der Datenaustausch �ber eine Quelle geschieht Abschnitt- und Kanalweise. Als <b>Abschnitt</b>
 * wird hier der Zeitraum bezeichnet, in dem der Nutzer mit den momentan bereitstehenden
 * Bl�cken (der einzelnen Kan�le) arbeitet. Sind diese Arbeitsoperationen auf den
 * Bl�cken abgeschlossen, mu� der Nutzer den n�chsten Abschnitt einleiten, um
 * seitens der Datenquelle die n�chsten/folgenden Daten zu erhalten. Dies geschieht
 * durch den Aufruf der Methode IncrementBlockPointer.
 * Innerhalb eines Abschnittes kann der Nutzer mittels der Methode GetBlockPointer Zeiger
 * auf die Bl�cke der Kan�le der Datenquelle abrufen. Er darf allerdings die Daten dieser
 * Bl�cke <b>nicht ver�ndern</b> (siehe const float*).
 * 
 * Der Vorgehensweise f�r das kontinuierliche Auslesen von Daten aus der Quelle
 * geschieht nach folgendem Schema (Pseudocode):
 *
 * <pre>
 *   while (Datasource.GetBlockPointer(0) != 0) { // Noch Daten verf�gbar?
 *     for i from 0 to Datasource.GetNumberOfChannels()-1 do {
 *       BlockPointer = Datasource.GetBlockPointer(i);
 *       // Arbeite mit den Daten ...
 *     }
 *     
 *     // N�chste Eingabebl�cke bereitstellen
 *     Datasource.IncrementBlockPointer();
 *   }
 * </pre>
 *
 * Die Datenquelle kann sich in zwei Zust�nden befinden: 1. Es sind Daten verf�gbar
 * und 2. es sind keine Daten verf�gbar.
 * Im ersten Fall liefert jeder Aufruf der Methode GetBlockPointer einen g�ltigen Zeiger
 * ungleich dem Nullzeiger zur�ck. Tritt der zweite Fall auf, so gibt die Methode Nullzeiger
 * zur�ck. Dieser Nullzeiger-Fall muss immer von benutzenden Modulen entsprechend behandelt werden.
 *
 * \ingroup datasources
 */
class ITA_DATA_SOURCES_API ITADatasource
{
public:
    //! Destruktor
	virtual ~ITADatasource();

	//! Eigenschaften des Audiostreams zur�ckgeben
	//virtual const ITAStreamProperties* GetStreamProperties() const=0;

	//! Blockl�nge zur�ckgeben
	/**
	 * \return Blockl�nge der Datenquelle in Samples
	 */
	virtual unsigned int GetBlocklength() const=0;
	
	//! Anzahl der Kan�le zur�ckgeben
	/**
	 * \return Anzahl der Kan�le der Datenquelle
	 * 
	 * \note Falls die Anzahl der Kan�le der Datenquelle noch nicht feststeht,
	 *       wird 0 zur�ckgegeben (siehe Klassenbeschreibung).
	 */
	virtual unsigned int GetNumberOfChannels() const=0;
	
	//! Samplerate zur�ckgeben
	/**
	 * \return Samplerate der Datenquelle (in Hertz)
	 *
	 * \note Falls die Samplerate der Datenquelle noch nicht feststeht,
	 *       wird 0 zur�ckgegeben (siehe Klassenbeschreibung).
     */
	virtual double GetSampleRate() const=0;

	//! Blockzeiger abrufen
	/**
	 * Diese Methode gibt den Zeiger auf das float-Array zur�ck,
	 * in dem der n�chsten Block Daten gespeichert ist. Solch ein
	 * Zeiger wird auch Blockzeiger genannt.
	 *
	 * \param uiChannel Index des Kanals (Wertebereich: [0, Kan�le-1])
	 * \param pStreamInfo Zustandsinformationen des Streams (niemals NULL)
	 *
	 * \return Zeiger auf das float-Array mit den n�chsten Daten
	 *         des angegebenen Kanals
	 *
	 * \note Falls momentan keine Daten verf�gbar sind,
	 *       wird der <b>Nullzeiger</b> zur�ckgegeben.
	 *
	 * \note <b>Testen</b> Sie die <b>Verf�gbarkeit von Daten</b> durch den Aufruf 
	 *       der Methode mit einem beliebigen g�ltigen Kanal und bWaitForData==false.
	 *       Erhalten Sie in diesem Falle den Nullzeiger zur�ck, sind keine 
	 *       Daten verf�gbar.
	 *
	 * \warning Der Aufruf der Methode mit einem ung�ltigen Index (siehe Wertebereich)
	 *          f�hrt <b>auch</b> zur R�ckgabe eines <b>Nullzeigers</b>.
	 */
	virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo)=0;	
	
	//! Blockzeiger inkrementieren
	/**
	 * Der Aufruf dieser Methode inkrementiert den internen Blockzeiger der Datenquelle.
	 * Dadurch stellt die Datenquelle die n�chsten Daten bereit. Wenn Sie die
	 * Leseoperationen auf den aktuellen Bl�cken der Datenquelle
	 * abgeschlossen haben, <b>m�ssen</b> Sie diese Methode aufrufen, um seitens 
	 * der Datenquelle die n�chsten Daten bereitzustellen.
	 */ 
	virtual void IncrementBlockPointer()=0;

	//! Anzahl der Benutzungen der Datenquelle zur�ckgeben
	virtual int GetNumReferences();

	//! Entfernt alle Referenzen (setzt Referenzz�hler auf 0)
	virtual void ClearReferences();

	//! Benutzung vermerken
	/**
	 * Erlaubt Benutzern der Datenquelle ihre Benutzung zu vermerken.
	 * Die Implementierung erfolg mittels Referenzz�hlern. Auf diese
	 * einfache Weise kann sichergestellt werden, das eine Datenquelle
	 * nicht gel�scht wird, sofern sie noch benutzt wird.
	 *
	 * \return Anzahl der Referenzen nach dem Hinzuf�gen
	 */
	virtual int AddReference();

	//! Benutzung vermerken
	/**
	 * Erlaubt Benutzern der Datenquelle ihre Benutzung zu vermerken.
	 * Die Implementierung erfolg mittels Referenzz�hlern. Auf diese
	 * einfache Weise kann sichergestellt werden, das eine Datenquelle
	 * nicht gel�scht wird, sofern sie noch benutzt wird.
	 *
	 * \return Anzahl der Referenzen nach dem Entfernen
	 */
	virtual int RemoveReference();

protected:
	//! Gesch�tzter Standardkonstruktor
	ITADatasource();

private:
	std::atomic< int > m_iRefCount;
};

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCE
