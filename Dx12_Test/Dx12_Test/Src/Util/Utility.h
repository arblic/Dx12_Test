//-----------------------------------------------------------------------------
//! @file   Utility.h
//! @brief  ユーティリティ
//! @author keisuke kamakami.
//-----------------------------------------------------------------------------
#ifndef __UTILITY_H__
#define	__UTILITY_H__

template< typename T >
void SAFE_RELEASE( T*& pT ) {
	if( pT ) {
		pT->Release();
		pT	= nullptr;
	}
}

#endif	/*__UTILITY_H__*/

//---< end of file >-----------------------------------------------------------