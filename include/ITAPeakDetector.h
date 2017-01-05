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
// $Id: ITAPeakDetector.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_PEAK_DETECTOR
#define INCLUDE_WATCHER_ITA_PEAK_DETECTOR

#include <ITADataSourcesDefinitions.h>
          
#include <ITACriticalSection.h>
#include <ITADataSource.h>
#include <vector>

//! Detektor für Spitzwerte (peak values) in Audiostreams
/**
 * Die Klasse ITAPeakDetector wird zwischen eine Datenquelle und einen
 * Konsumenten für die Datenquelle geschaltet und detektiert dabei
 * die Spitzenwerte (peak values) in den Audiostreams der Kanäle der
 * Datenquelle. Die Klasse bekommt im Konstruktor ihre Datenquelle gesetzt
 * und stellt selber die Schnittstelle ITADatasource zur Verfügung.
 * Aufrufe von GetBlockPointer und IncrementBlockPointer werden an die
 * gesetzte Datenquelle delegiert und beim Aufruf von GetBlockPointer dieser
 * Klasse werden wird - je Kanal - Analyse durchgeführt.
 *
 * Alle Methoden welche Analysewerte zurückgeben besitzen einen Parameter bReset.
 * Ist dieser true (Normalfall), wird mit dem Aufruf der entsprechenden Methode
 * der oder die entsprechenden Analysewerte (der Methode) automatisch zurückgesetzt
 * für weitere Messungen. Durch Wahl des Parameters bReset = false, wird dieses
 * Rücksetzen unterdrückt und die bisherigen Spitzenwerte bleiben erhalten.
 *
 * \note Die Klasse ist Thread-safe
 */
class ITA_DATA_SOURCES_API ITAPeakDetector : public ITADatasource {
public:
	//! Konstruktor
	/**
	 * \note Es darf kein Nullzeiger übergeben werden.
	 */
	ITAPeakDetector(ITADatasource* pDatasource);	

	//! Destruktor
	virtual ~ITAPeakDetector();

	//! Datenquelle zurückgeben
	ITADatasource* GetDatasource() const { return m_pDataSource; }

	//! Messung zurücksetzen
	void Reset();

	//! Spitzenwert über alle Kanäle abrufen
	/**
	 * Diese Methode dient zum Abrufen des Spitzenwertes über alle
	 * Kanäle. Die Ergebnisse werden per Call-by-Reference (über Zeiger)
	 * übergeben. Falls ein Wert uninteressant ist, kann ein Nullzeiger
	 * anstelle dessen übergeben werden. Wird bReset = false gesetzt,
	 * so bleibt der bisherige Spitzenwert und der Kanal im dem er auftrat
	 * für die weitere Analyse erhalten.
	 */
	void GetOverallPeak(float* pfPeak, unsigned int* puiChannel=0, bool bReset=true);

	//! Spitzenwert über alle Kanäle in Dezibel zurückgeben
	/**
	 * \note Falls der Spitzenwert 0 ist, gibt die Methode die
	 *       symbolische Konstante DECIBEL_MINUS_INFINITY zurück
	 *       (siehe ITANumericUtils.h)
	 */
	void GetOverallPeakDecibel(double* pdPeakDecibel, unsigned int* puiChannel=0, bool bReset=true);

	//! Spitzenwert eines Kanals zurückgeben
	/**
	 * \note Wenn Sie die Spitzenwerte aller Kanäle abrufen möchten, 
	 *       so empfiehlt sich die Methode GetChannelPeaks, da diese schneller ist.
	 */
	float GetPeak(unsigned int uiChannel, bool bReset=true);

	//! Spitzenwert eines Kanals in Dezibel zurückgeben
	/**
	 * \note Falls der Spitzenwert 0 ist, gibt die Methode die
	 *       symbolische Konstante DECIBEL_MINUS_INFINITY zurück
	 *       (siehe ITANumericUtils.h)
	 * \note Wenn Sie die Spitzenwerte aller Kanäle abrufen möchten, 
	 *       so empfiehlt sich die Methode GetChannelPeaksDecibel, da diese schneller ist.
	 */
	double GetPeakDecibel(unsigned int uiChannel, bool bReset=true);

	//! Spitzenwerte aller Kanäle abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kanäle im
	 * angegebenen Zielarray.
	 *
	 * \note Das Zielarray muß mindestens so viele Felder haben,
	 *       wie die Datenquelle Kanäle hat
	 */
	void GetPeaks(float* pfDest, bool bReset=true);

	//! Spitzenwerte aller Kanäle abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kanäle im
	 * angegebenen Vektor.
	 *
	 * \note Falls der Vektor weniger Felder als Kanäle hat,
	 *       so wird er automatisch vergrößert.
	 */
	void GetPeaks(std::vector<float>& vfDest, bool bReset=true);

	//! Spitzenwerte aller Kanäle in Dezibel abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kanäle 
	 * in Dezibel im angegebenen Zielarray.
	 *
	 * \note Das Zielarray muß mindestens so viele Felder haben,
	 *       wie die Datenquelle Kanäle hat
	 */
	void GetPeaksDecibel(double* pdDestDecibel, bool bReset=true);

	//! Spitzenwerte aller Kanäle in Dezibel abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kanäle 
	 * in Dezibel im angegebenen Vektor.
	 *
	 * \note Falls der Vektor weniger Felder als Kanäle hat,
	 *       so wird er automatisch vergrößert.
	 */
	void GetPeaksDecibel(std::vector<double>& vdDestDecibel, bool bReset=true);

	// -= Überladene Methoden von ITADatasource =-
	
	unsigned int GetBlocklength() const { return m_uiBlocklength; }
	unsigned int GetNumberOfChannels() const { return m_uiChannels; }
	double GetSampleRate() const { return m_dSamplerate; }

	virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);	
	virtual void IncrementBlockPointer();

protected:
	ITADatasource* m_pDataSource;			// Angeschlossene Datenquelle
	double m_dSamplerate;					// Abtastrate [Hz]
	unsigned int m_uiChannels;				// Anzahl Kanäle
	unsigned int m_uiBlocklength;			// Streaming Puffergröße [Samples]
	ITACriticalSection m_cs;				// Sichert exklusiven Zugriff auf die Daten (s.u.)
	float* m_pfPeaks;						// Spitzenwerte der einzelnen Kanäle
	float m_fOverallPeak;					// Spitzenwert über alle Kanäle
	unsigned int m_uiOverallPeakChannel;	// Kanal in dem der Spitzenwert auftrat
};

#endif // INCLUDE_WATCHER_ITA_PEAK_DETECTOR