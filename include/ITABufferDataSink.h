/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2019
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

#ifndef INCLUDE_WATCHER_ITA_BUFFER_DATA_SINK
#define INCLUDE_WATCHER_ITA_BUFFER_DATA_SINK

#include <ITADataSourcesDefinitions.h>

#include <ITAAudiofileCommon.h>
#include <ITAStreamInfo.h>

// STL-Includes
#include <string> 
#include <vector>

// Vorw�rtsdeklarationen
class ITAAudiofileWriter;
class ITADatasource;

//! Audiodatei-Datensenke
/**
 * Die Klasse ITAFileDatasink realisiert eine Datensenke f�r Audiostreams,
 * welche die Audiodaten in eine Datei schreibt. Der Datentransfer muss
 * selbst initiiert werden.
 */
class ITA_DATA_SOURCES_API ITABufferDataSink
{
public:
	//! Konstruktor (Eigene Pufferverwaltung)
	/**
	 * Dieser Konstruktor erzeugt eine Puffer-Datensenke. Die Puffer zum Speichern
	 * der Audiodaten werden bei diesem Konstruktor selbst erzeugt und von der
	 * Datenquelle verwaltet. Mit dem Aufruf des Destruktors werden die Puffer
	 * freigeben. Die Pufferzeiger k�nnen mit der Methode GetBuffers() abgerufen werden.
	 *
	 * \param pdsSource    Datenquelle
	 * \param iBuffersize  Puffergr��e [Anzahl Samples]
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgel�st.
	 */
	ITABufferDataSink( ITADatasource* pdsSource, int uiBuffersize );

	//! Konstruktor (Fremdverwaltete Puffer)
	/**
	 * Dieser Konstruktor erzeugt eine Puffer-Datensenke, welche die angegebenen Puffer
	 * zum Speichern der Audiodaten verwendet und diese Puffer nicht selbst verwaltet.
	 *
	 * \param pdsSource    Datenquelle
	 * \param vpfBuffer    Puffer
	 * \param iBuffersize  Puffergr��e [Anzahl Samples]
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgel�st.
	 */
	ITABufferDataSink( ITADatasource* pdsSource, std::vector< float* > vpfBuffer, int uiBuffersize );

	//! Destruktor
	virtual ~ITABufferDataSink();

	//! Puffergr��e [Anzahl Samples] zur�ckgeben
	int GetBuffersize() const;

	//! Pufferzeiger zur�ckgeben
	std::vector< float* > GetBuffers() const;

	//! Schreibposition in den Puffern zur�ckgeben
	unsigned int GetWriteCursor() const;

	//! Schreibposition in den Puffern setzen
	void SetWriteCursor( unsigned int uiWriteCursor );

	//! Audiodaten transferrieren
	/**
	 * Liest die gew�nschte Anzahl Samples von der angeschlossenen Datenquelle
	 * und schreibt diese in die Audiodatei. Hierbei wird die Anzahl Samples
	 * auf das n�chstgr��ere Vielfache der Blockl�nge aufgerundet.
	 */
	void Transfer( unsigned int uiSamples );

private:
	ITADatasource* m_pdsSource;				// Datenquelle
	std::vector<float*> m_vpfBuffer;		// Puffervektor
	std::vector<const float*> m_vpfBufferX;	// Export-Puffervektor (const)
	bool m_bManagedBuffer;					// Werden die Puffer verwaltet?
	unsigned int m_uiBuffersize;			// Anzahl Samples im Puffer
	unsigned int m_uiWriteCursor;			// Schreibcursor auf den Puffern
	ITAStreamInfo m_siState;				// Streamzustand
};

#endif // INCLUDE_WATCHER_ITA_BUFFER_DATA_SINK
