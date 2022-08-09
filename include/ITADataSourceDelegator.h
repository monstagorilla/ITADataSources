/*
   +---------------------------------------------------------+
   |                                                         |
   |  ITADatasourceDelegator.h                               |
   |  Abstrakte Basisklasse für Datenquellen                 |
   |                                                         |
   |  Autoren:  Frank Wefers                                 |
   |                                                         |
   |  (c) Copyright Institut für technische Akustik (ITA)    |
   |      Aachen university of technology (RWTH), 2005-2022  |
   |                                                         |
   +---------------------------------------------------------+
                                                               */
// $Id: ITADatasourceDelegator.h,v 1.2 2008-12-12 12:56:01 fwefers Exp $

#ifndef _ITADATASOURCEDELEGATOR_H_
#define _ITADATASOURCEDELEGATOR_H_

#include <ITADataSource.h>

/**
 * Diese Klasse implementiert die Schnittstelle der ITADatasource, stellt die
 * Datenquellen-Dienstleistungen aber nicht selbst bereit, sondern bezieht diese
 * Dienstleistungen von einer anderen Datenquelle. Diese lose Kopplung ist per
 * uses-Beziehung realisiert. Die Klasse vermeidet dem Programmierer Tippaufwand
 * wenn das Bereitstellen von Datenquellen-Dienstleistungen mit konkreten
 * Implementierungsklassen realisiert werden soll.
 *
 * Autor: Frank Wefers <Frank.Wefers@akustik.rwth-aachen.de>
 */

class ITADatasourceDelegator : public ITADatasource
{
protected:
	//! Geschützer Standardkonstruktor
	ITADatasourceDelegator( ) : ITADatasource( ), m_pDatasourceDelegatorTarget( 0 ) {}

	//! Geschützer Standardkonstruktor
	ITADatasourceDelegator( ITADatasource* pTarget ) : m_pDatasourceDelegatorTarget( pTarget ) {}

public:
	virtual ~ITADatasourceDelegator( ) {}

	unsigned int GetBlocklength( ) const { return m_pDatasourceDelegatorTarget->GetBlocklength( ); }

	unsigned int GetNumberOfChannels( ) const { return m_pDatasourceDelegatorTarget->GetNumberOfChannels( ); }

	double GetSampleRate( ) const { return m_pDatasourceDelegatorTarget->GetSampleRate( ); }

	virtual const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo )
	{
		return m_pDatasourceDelegatorTarget->GetBlockPointer( uiChannel, pStreamInfo );
	}

	virtual void IncrementBlockPointer( ) { m_pDatasourceDelegatorTarget->IncrementBlockPointer( ); }

protected:
	ITADatasource* GetDatasourceDelegatorTarget( ) { return m_pDatasourceDelegatorTarget; }

	void SetDatasourceDelegatorTarget( ITADatasource* pTarget ) { m_pDatasourceDelegatorTarget = pTarget; }

private:
	ITADatasource* m_pDatasourceDelegatorTarget;
};

#endif // _ITADATASOURCEDELEGATOR_H_
