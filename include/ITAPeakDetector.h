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
// $Id: ITAPeakDetector.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_PEAK_DETECTOR
#define INCLUDE_WATCHER_ITA_PEAK_DETECTOR

#include <ITADataSourcesDefinitions.h>
          
#include <ITACriticalSection.h>
#include <ITADataSource.h>
#include <vector>

//! Detektor f�r Spitzwerte (peak values) in Audiostreams
/**
 * Die Klasse ITAPeakDetector wird zwischen eine Datenquelle und einen
 * Konsumenten f�r die Datenquelle geschaltet und detektiert dabei
 * die Spitzenwerte (peak values) in den Audiostreams der Kan�le der
 * Datenquelle. Die Klasse bekommt im Konstruktor ihre Datenquelle gesetzt
 * und stellt selber die Schnittstelle ITADatasource zur Verf�gung.
 * Aufrufe von GetBlockPointer und IncrementBlockPointer werden an die
 * gesetzte Datenquelle delegiert und beim Aufruf von GetBlockPointer dieser
 * Klasse werden wird - je Kanal - Analyse durchgef�hrt.
 *
 * Alle Methoden welche Analysewerte zur�ckgeben besitzen einen Parameter bReset.
 * Ist dieser true (Normalfall), wird mit dem Aufruf der entsprechenden Methode
 * der oder die entsprechenden Analysewerte (der Methode) automatisch zur�ckgesetzt
 * f�r weitere Messungen. Durch Wahl des Parameters bReset = false, wird dieses
 * R�cksetzen unterdr�ckt und die bisherigen Spitzenwerte bleiben erhalten.
 *
 * \note Die Klasse ist Thread-safe
 */
class ITA_DATA_SOURCES_API ITAPeakDetector : public ITADatasource {
public:
	//! Konstruktor
	/**
	 * \note Es darf kein Nullzeiger �bergeben werden.
	 */
	ITAPeakDetector(ITADatasource* pDatasource);	

	//! Destruktor
	virtual ~ITAPeakDetector();

	//! Datenquelle zur�ckgeben
	ITADatasource* GetDatasource() const { return m_pDatasource; }

	//! Messung zur�cksetzen
	void Reset();

	//! Spitzenwert �ber alle Kan�le abrufen
	/**
	 * Diese Methode dient zum Abrufen des Spitzenwertes �ber alle
	 * Kan�le. Die Ergebnisse werden per Call-by-Reference (�ber Zeiger)
	 * �bergeben. Falls ein Wert uninteressant ist, kann ein Nullzeiger
	 * anstelle dessen �bergeben werden. Wird bReset = false gesetzt,
	 * so bleibt der bisherige Spitzenwert und der Kanal im dem er auftrat
	 * f�r die weitere Analyse erhalten.
	 */
	void GetOverallPeak(float* pfPeak, unsigned int* puiChannel=0, bool bReset=true);

	//! Spitzenwert �ber alle Kan�le in Dezibel zur�ckgeben
	/**
	 * \note Falls der Spitzenwert 0 ist, gibt die Methode die
	 *       symbolische Konstante DECIBEL_MINUS_INFINITY zur�ck
	 *       (siehe ITANumericUtils.h)
	 */
	void GetOverallPeakDecibel(double* pdPeakDecibel, unsigned int* puiChannel=0, bool bReset=true);

	//! Spitzenwert eines Kanals zur�ckgeben
	/**
	 * \note Wenn Sie die Spitzenwerte aller Kan�le abrufen m�chten, 
	 *       so empfiehlt sich die Methode GetChannelPeaks, da diese schneller ist.
	 */
	float GetPeak(unsigned int uiChannel, bool bReset=true);

	//! Spitzenwert eines Kanals in Dezibel zur�ckgeben
	/**
	 * \note Falls der Spitzenwert 0 ist, gibt die Methode die
	 *       symbolische Konstante DECIBEL_MINUS_INFINITY zur�ck
	 *       (siehe ITANumericUtils.h)
	 * \note Wenn Sie die Spitzenwerte aller Kan�le abrufen m�chten, 
	 *       so empfiehlt sich die Methode GetChannelPeaksDecibel, da diese schneller ist.
	 */
	double GetPeakDecibel(unsigned int uiChannel, bool bReset=true);

	//! Spitzenwerte aller Kan�le abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kan�le im
	 * angegebenen Zielarray.
	 *
	 * \note Das Zielarray mu� mindestens so viele Felder haben,
	 *       wie die Datenquelle Kan�le hat
	 */
	void GetPeaks(float* pfDest, bool bReset=true);

	//! Spitzenwerte aller Kan�le abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kan�le im
	 * angegebenen Vektor.
	 *
	 * \note Falls der Vektor weniger Felder als Kan�le hat,
	 *       so wird er automatisch vergr��ert.
	 */
	void GetPeaks(std::vector<float>& vfDest, bool bReset=true);

	//! Spitzenwerte aller Kan�le in Dezibel abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kan�le 
	 * in Dezibel im angegebenen Zielarray.
	 *
	 * \note Das Zielarray mu� mindestens so viele Felder haben,
	 *       wie die Datenquelle Kan�le hat
	 */
	void GetPeaksDecibel(double* pdDestDecibel, bool bReset=true);

	//! Spitzenwerte aller Kan�le in Dezibel abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kan�le 
	 * in Dezibel im angegebenen Vektor.
	 *
	 * \note Falls der Vektor weniger Felder als Kan�le hat,
	 *       so wird er automatisch vergr��ert.
	 */
	void GetPeaksDecibel(std::vector<double>& vdDestDecibel, bool bReset=true);

	// -= �berladene Methoden von ITADatasource =-
	
	unsigned int GetBlocklength() const { return m_uiBlocklength; }
	unsigned int GetNumberOfChannels() const { return m_uiChannels; }
	double GetSampleRate() const { return m_dSamplerate; }

	virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);	
	virtual void IncrementBlockPointer();

protected:
	ITADatasource* m_pDatasource;			// Angeschlossene Datenquelle
	double m_dSamplerate;					// Abtastrate [Hz]
	unsigned int m_uiChannels;				// Anzahl Kan�le
	unsigned int m_uiBlocklength;			// Streaming Puffergr��e [Samples]
	ITACriticalSection m_cs;				// Sichert exklusiven Zugriff auf die Daten (s.u.)
	float* m_pfPeaks;						// Spitzenwerte der einzelnen Kan�le
	float m_fOverallPeak;					// Spitzenwert �ber alle Kan�le
	unsigned int m_uiOverallPeakChannel;	// Kanal in dem der Spitzenwert auftrat
};

#endif // INCLUDE_WATCHER_ITA_PEAK_DETECTOR