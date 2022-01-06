#include "ITADataSource.h"

#include <ITAException.h>

ITADatasource::ITADatasource( ) : m_iRefCount( 0 ) {}

ITADatasource::~ITADatasource( )
{
	if( GetNumReferences( ) > 0 )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Datasource may not be destroyed. It still has users." );
}

int ITADatasource::GetNumReferences( )
{
	return m_iRefCount;
}

void ITADatasource::ClearReferences( )
{
	m_iRefCount = 0;
}

int ITADatasource::AddReference( )
{
	return ++m_iRefCount;
}

int ITADatasource::RemoveReference( )
{
	while( true )
	{
		int i = m_iRefCount;

		if( i <= 0 )
			// Da hat der Programmierer irgendwo ein Reference-Leak!
			ITA_EXCEPT1( MODAL_EXCEPTION, "Datasource does not have any references." );

		if( m_iRefCount.compare_exchange_weak( i, i - 1 ) )
			return i - 1;
	}

	// Wird nie erreicht!
	return -1;
}