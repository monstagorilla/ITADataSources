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

#ifndef INCLUDE_WATCHER_ITA_PEAK_DETECTOR
#define INCLUDE_WATCHER_ITA_PEAK_DETECTOR

#include <ITADataSourcesDefinitions.h>

#include <ITACriticalSection.h>
#include <ITADataSource.h>
#include <ITAStopWatch.h>

#include <atomic>
#include <string>
#include <vector>

//! Detektor for statistic values in audio stream (i.e. for level metering)
/**
 * Die Klasse ITAStreamDetector wird zwischen eine Datenquelle und einen
 * Konsumenten für die Datenquelle geschaltet und detektiert dabei
 * die Spitzenwerte (peak values) und Mittelwerte (RMS) in den Audiostreams der Kanäle der
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
class ITA_DATA_SOURCES_API ITAStreamDetector : public ITADatasource
{
public:

	//! Mode of detection
	enum Mode
	{
		DEACTIVATED = -1,
		PEAK_AND_RMS,
		PEAK,
		RMS
	};

	//! Konstruktor
	/**
	 * \note Es darf kein Nullzeiger übergeben werden.
	 */
	ITAStreamDetector( ITADatasource* pDatasource, int iMode = PEAK_AND_RMS );

	//! Destruktor
	virtual inline ~ITAStreamDetector() {};

	//! Datenquelle zurückgeben
	inline ITADatasource* GetDatasource() const
	{
		return m_pDataSource;
	};

	//! Messung zurücksetzen
	void Reset();

	void SetMode( const int iMode );

	int GetMode() const;

	//! Spitzenwert über alle Kanäle abrufen
	/**
	 * Diese Methode dient zum Abrufen des Spitzenwertes über alle
	 * Kanäle. Die Ergebnisse werden per Call-by-Reference (über Zeiger)
	 * übergeben. Falls ein Wert uninteressant ist, kann ein Nullzeiger
	 * anstelle dessen übergeben werden. Wird bReset = false gesetzt,
	 * so bleibt der bisherige Spitzenwert und der Kanal im dem er auftrat
	 * für die weitere Analyse erhalten.
	 */
	float GetOverallPeak( int& iPeakChannelIndex, const bool bReset = true );

	//! Spitzenwert über alle Kanäle in Dezibel zurückgeben
	/**
	 * \note Falls der Spitzenwert 0 ist, gibt die Methode die
	 *       symbolische Konstante DECIBEL_MINUS_INFINITY zurück
	 *       (siehe ITANumericUtils.h)
	 */
	double GetOverallPeakDecibel( int& iPeakChannelIndex, const bool bReset = true );

	//! Spitzenwert eines Kanals zurückgeben
	/**
	 * \note Wenn Sie die Spitzenwerte aller Kanäle abrufen möchten,
	 *       so empfiehlt sich die Methode GetChannelPeaks, da diese schneller ist.
	 */
	float GetPeak( const int iChannel, const bool bReset = true );

	//! Spitzenwert eines Kanals in Dezibel zurückgeben
	/**
	 * \note Falls der Spitzenwert 0 ist, gibt die Methode die
	 *       symbolische Konstante DECIBEL_MINUS_INFINITY zurück
	 *       (siehe ITANumericUtils.h)
	 * \note Wenn Sie die Spitzenwerte aller Kanäle abrufen möchten,
	 *       so empfiehlt sich die Methode GetChannelPeaksDecibel, da diese schneller ist.
	 */
	double GetPeakDecibel( const int iChannel, bool bReset = true );

	//! Spitzenwerte aller Kanäle abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kanäle im
	 * angegebenen Vektor.
	 *
	 * \note Falls der Vektor weniger Felder als Kanäle hat,
	 *       so wird er automatisch vergrößert.
	 */
	void GetPeaks( std::vector< float >& vfDest, const bool bReset = true );
	
	//! Spitzenwerte aller Kanäle in Dezibel abrufen
	/**
	 * Diese Methode speichert die Spitzenwerte aller Kanäle
	 * in Dezibel im angegebenen Vektor.
	 *
	 * \note Falls der Vektor weniger Felder als Kanäle hat,
	 *       so wird er automatisch vergrößert.
	 */
	void GetPeaksDecibel( std::vector< double >& vdDestDecibel, const bool bReset = true );

	float GetOverallRMS( const bool bReset = true );
	double GetOverallRMSDecibel( const bool bReset = true );
	float GetRMS( const int iChannel, const bool bReset = true );
	double GetRMSDecibel( const int iChannel, const bool bReset = true );
	void GetRMSs( std::vector< float >& vfDest, const bool bReset = true );
	void GetRMSsDecibel( std::vector< float >& vfDestDecibel, const bool bReset = true );

	inline unsigned int GetBlocklength() const
	{
		return m_iBlocklength;
	};

	inline unsigned int GetNumberOfChannels() const
	{
		return m_iChannels;
	};

	inline double GetSampleRate() const
	{
		return m_dSamplerate;
	};

	virtual const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo );
	virtual void IncrementBlockPointer();

	void SetProfilerEnabled( bool bEnabled );
	bool GetProfilerEnabled() const;

	double GetProfilerMeanCalculationTime( bool bReset = true );
	std::string GetProfilerResult( bool bReset = true );

protected:
	ITADatasource* m_pDataSource;			//!< Angeschlossene Datenquelle
	double m_dSamplerate;					//!< Abtastrate [Hz]
	int m_iChannels;				//!< Anzahl Kanäle
	int m_iBlocklength;			//!< Streaming Puffergröße [Samples]
	std::atomic< int > m_iMode;
	ITACriticalSection m_cs;				//!< Sichert exklusiven Zugriff auf die Daten (s.u.)
	std::vector< float > m_vfPeaks;			//!< Spitzenwerte der einzelnen Kanäle
	float m_fOverallPeak;					//!< Spitzenwert über alle Kanäle
	int m_iOverallPeakChannel;	//!< Kanal in dem der Spitzenwert auftrat
	std::vector< double > m_vdRMSSquaredSums;
	int m_iRMSBlocks;
	bool m_bProfilerEnabled;
	ITAStopWatch m_sw;
};

#endif // INCLUDE_WATCHER_ITA_PEAK_DETECTOR
