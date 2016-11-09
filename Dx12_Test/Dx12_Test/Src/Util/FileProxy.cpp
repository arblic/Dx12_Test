//-----------------------------------------------------------------------------
//! @file   FileProxy.cpp
//! @brief  Xmlリーダ
//! @author keisuke kamakami.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FileProxy.h"

namespace FileProxy {


//-----------------------------------------------------------------------------
//!	CreateFileマクロ
//-----------------------------------------------------------------------------
#define	GetHandleRA( FileName )		::CreateFileA(	FileName,\
												GENERIC_READ,\
												FILE_SHARE_READ,\
												NULL,\
												OPEN_EXISTING,\
												FILE_ATTRIBUTE_NORMAL,\
												NULL )
#define	GetHandleRW( FileName )		::CreateFileW(	FileName,\
												GENERIC_READ,\
												FILE_SHARE_READ,\
												NULL,\
												OPEN_EXISTING,\
												FILE_ATTRIBUTE_NORMAL,\
												NULL )
#define	GetHandleWA( FileName )		::CreateFileA(	FileName,\
												GENERIC_WRITE,\
												0,\
												NULL,\
												CREATE_ALWAYS,\
												FILE_ATTRIBUTE_NORMAL,\
												NULL )
#define	GetHandleWW( FileName )		::CreateFileW(	FileName,\
												GENERIC_WRITE,\
												0,\
												NULL,\
												CREATE_ALWAYS,\
												FILE_ATTRIBUTE_NORMAL,\
												NULL )
#define	IsEnableHandle( Handle )	( (NULL!=Handle) && (INVALID_HANDLE_VALUE!=Handle) )


//-----------------------------------------------------------------------------
//!	ファイル属性取得
//-----------------------------------------------------------------------------
DWORD getAttr( const char * pFileName )	{ return GetFileAttributesA( pFileName ); }
DWORD getAttr( const wchar_t * pFileName )	{ return GetFileAttributesW( pFileName ); }

//-----------------------------------------------------------------------------
//!	ファイル存在チェック
//-----------------------------------------------------------------------------
bool isExist( const char * pFileName )		{ return ((DWORD)-1 != getAttr( pFileName )); }
bool isExist( const wchar_t * pFileName )	{ return ((DWORD)-1 != getAttr( pFileName )); }

//-----------------------------------------------------------------------------
//!	書き込み可能かチェック
//-----------------------------------------------------------------------------
bool isWritable( const char * pFileName )
{
	DWORD	Attr	= getAttr( pFileName );

	if( ((DWORD)-1) == Attr )	return true;
	if( FILE_ATTRIBUTE_READONLY & Attr ) return false;

	return true;
}

//-----------------------------------------------------------------------------
//!	書き込み可能かチェック
//-----------------------------------------------------------------------------
bool isWritable( const wchar_t * pFileName )
{
	DWORD	Attr	= getAttr( pFileName );

	if( ((DWORD)-1) == Attr )	return true;
	if( FILE_ATTRIBUTE_READONLY & Attr ) return false;

	return true;
}

//-----------------------------------------------------------------------------
//!	ファイルのサイズ取得
//-----------------------------------------------------------------------------
DWORD getSize( const char * pFileName )
{
	HANDLE	hFile	= NULL;
	DWORD	Size	= 0;

	hFile = GetHandleRA( pFileName );
	if( IsEnableHandle( hFile ) ) {
		Size = GetFileSize( hFile, NULL );
	}

	CloseHandle( hFile );
	return Size;
}

//-----------------------------------------------------------------------------
//!	ファイルのサイズ取得
//-----------------------------------------------------------------------------
DWORD getSize( const wchar_t * pFileName )
{
	HANDLE	hFile	= NULL;
	DWORD	Size	= 0;

	hFile = GetHandleRW( pFileName );
	if( IsEnableHandle( hFile ) ) {
		Size = GetFileSize( hFile, NULL );
	}

	CloseHandle( hFile );
	return Size;
}

//-----------------------------------------------------------------------------
//!	読み取り
//-----------------------------------------------------------------------------
DWORD read( const char * pFileName, void * pBuffer, const DWORD BuffSize )
{
	HANDLE	hFile		= NULL;
	DWORD	ReadSize	= 0;

	hFile = GetHandleRA( pFileName );
	if( IsEnableHandle( hFile ) ) {
		ReadSize = GetFileSize( hFile, NULL );
		if( ReadSize <= BuffSize ) {
			ReadFile( hFile, pBuffer, BuffSize, &ReadSize, NULL );
		} else {
			ReadSize	= 0;
		}
	}

	CloseHandle( hFile );
	return ReadSize;
}

//-----------------------------------------------------------------------------
//!	読み取り
//-----------------------------------------------------------------------------
DWORD read( const wchar_t * pFileName, void * pBuffer, const DWORD BuffSize )
{
	HANDLE	hFile		= NULL;
	DWORD	ReadSize	= 0;

	hFile = GetHandleRW( pFileName );
	if( IsEnableHandle( hFile ) ) {
		ReadSize = GetFileSize( hFile, NULL );
		if( ReadSize <= BuffSize ) {
			ReadFile( hFile, pBuffer, BuffSize, &ReadSize, NULL );
		} else {
			ReadSize	= 0;
		}
	}

	CloseHandle( hFile );
	return ReadSize;
}

//-----------------------------------------------------------------------------
//!	書き込み
//-----------------------------------------------------------------------------
DWORD write( const char * pFileName, const void * pBuffer, const DWORD BuffSize, const DWORD Offset )
{
	HANDLE	hFile		= NULL;
	DWORD	WrittenSize	= 0;

	hFile = GetHandleWA( pFileName );
	if( IsEnableHandle( hFile ) ) {
		if( 0 < Offset ) {
			SetFilePointer( hFile, Offset, NULL, FILE_END );
		}
		WriteFile( hFile, pBuffer, BuffSize, &WrittenSize, NULL );
	}

	CloseHandle( hFile );
	return WrittenSize;
}

//-----------------------------------------------------------------------------
//!	書き込み
//-----------------------------------------------------------------------------
DWORD write( const wchar_t * pFileName, const void * pBuffer, const DWORD BuffSize, const DWORD Offset )
{
	HANDLE	hFile		= NULL;
	DWORD	WrittenSize	= 0;

	hFile = GetHandleWW( pFileName );
	if( IsEnableHandle( hFile ) ) {
		if( 0 < Offset ) {
			SetFilePointer( hFile, Offset, NULL, FILE_END );
		}
		WriteFile( hFile, pBuffer, BuffSize, &WrittenSize, NULL );
	}

	CloseHandle( hFile );
	return WrittenSize;
}

//-----------------------------------------------------------------------------
//!	ディレクトリ作成
//-----------------------------------------------------------------------------
bool createDir( const char * pDirectoryName )
{
	// ディレクトリの存在判定
	DWORD attr = ::GetFileAttributesA( pDirectoryName );
	if( (attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) ) return TRUE;
	
	// 親ディレクトリの取得
	char pDrive[ _MAX_DRIVE ];
	char pDir[ _MAX_DIR ];
	::_splitpath_s( pDirectoryName, pDrive, _countof(pDrive), pDir, _countof(pDir), NULL, 0, NULL, 0 );
	
	char pParentDir[ MAX_PATH ];
	sprintf_s( pParentDir, _countof(pParentDir), "%s%s", pDrive, pDir );
	
	size_t last_pos = strlen( pParentDir ) - 1;
	if( (pParentDir[last_pos] == '\\') || (pParentDir[last_pos] == '/') ) pParentDir[ last_pos ] = '\0';
	
	// 親ディレクトリを再帰的に作成
	if( !createDir( pParentDir ) ) return FALSE;
	
	// 自ディレクトリを作成
	return (FALSE!=::CreateDirectoryA( pDirectoryName, NULL ));
}

//-----------------------------------------------------------------------------
//!	ディレクトリ作成
//-----------------------------------------------------------------------------
bool createDir( const wchar_t * pDirectoryName )
{
	// ディレクトリの存在判定
	DWORD attr = ::GetFileAttributesW( pDirectoryName );
	if( (attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) ) return TRUE;
	
	// 親ディレクトリの取得
	wchar_t pDrive[ _MAX_DRIVE ];
	wchar_t pDir[ _MAX_DIR ];
	::_wsplitpath_s( pDirectoryName, pDrive, _countof(pDrive), pDir, _countof(pDir), NULL, 0, NULL, 0 );
	
	wchar_t pParentDir[ MAX_PATH ];
	swprintf_s( pParentDir, _countof(pParentDir), L"%s%s", pDrive, pDir );
	
	size_t last_pos = wcslen( pParentDir ) - 1;
	if( (pParentDir[last_pos] == L'\\') || (pParentDir[last_pos] == L'/') ) pParentDir[ last_pos ] = L'\0';
	
	// 親ディレクトリを再帰的に作成
	if( !createDir( pParentDir ) ) return FALSE;
	
	// 自ディレクトリを作成
	return (FALSE!=::CreateDirectoryW( pDirectoryName, NULL ));
}

//-----------------------------------------------------------------------------
//!	ディレクトリ内の指定拡張子ファイルを消す
//-----------------------------------------------------------------------------
bool cleanDir( const char * pDirectoryName, const char * pExtention )
{
	HANDLE				hSearch	= INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA	fd;
	char				SearchStr[ 512 ]={0};
	char				PickupDir[ 512 ]={0};
	char				DeleteFileName[ 512 ]={0};

	if( !pDirectoryName )	return FALSE;

	// ディレクトリ名の末尾を削る
	strcpy_s( PickupDir, _countof(PickupDir), pDirectoryName );
	if( '/'==PickupDir[ _countof(PickupDir)-1 ] ) {
		PickupDir[ _countof(PickupDir)-1 ]	= '\n';
	}
	if( '\\'==PickupDir[ _countof(PickupDir)-1 ] ) {
		PickupDir[ _countof(PickupDir)-1 ]	= '\n';
	}

	// 拡張子名の .(ドット)を削る
	if( pExtention ) {
		const char * pCopyStartPtr	= pExtention;
		if( '.'==pExtention[0] ) {
			pCopyStartPtr	= pExtention + 1;
		}
		sprintf_s( SearchStr, _countof(SearchStr), "%s\\*.%s", PickupDir, pCopyStartPtr );
	} else {
		sprintf_s( SearchStr, _countof(SearchStr), "%s\\*.*", PickupDir );
	}

	// 全てのファイルを列挙する
	hSearch	= FindFirstFileA( SearchStr, &fd );
	if( INVALID_HANDLE_VALUE==hSearch ) {
		return FALSE;
	}

	do {
		sprintf_s( DeleteFileName, "%s\\%s", PickupDir, fd.cFileName );
		DeleteFileA( DeleteFileName );
	} while( FindNextFileA( hSearch, &fd ) );

	FindClose( hSearch );

	return TRUE;
}

//-----------------------------------------------------------------------------
//!	ディレクトリ内の指定拡張子ファイルを消す
//-----------------------------------------------------------------------------
bool cleanDir( const wchar_t * pDirectoryName, const wchar_t * pExtention )
{
	HANDLE				hSearch	= INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAW	fd;
	wchar_t				SearchStr[ 512 ]={0};
	wchar_t				PickupDir[ 512 ]={0};
	wchar_t				DeleteFileName[ 512 ]={0};

	if( !pDirectoryName )	return FALSE;

	// ディレクトリ名の末尾を削る
	wcscpy_s( PickupDir, _countof(PickupDir), pDirectoryName );
	if( '/'==PickupDir[ _countof(PickupDir)-1 ] ) {
		PickupDir[ _countof(PickupDir)-1 ]	= L'\n';
	}
	if( '\\'==PickupDir[ _countof(PickupDir)-1 ] ) {
		PickupDir[ _countof(PickupDir)-1 ]	= L'\n';
	}

	// 拡張子名の .(ドット)を削る
	if( pExtention ) {
		const wchar_t * pCopyStartPtr	= pExtention;
		if( _T('.')==pExtention[0] ) {
			pCopyStartPtr	= pExtention + 1;
		}
		swprintf_s( SearchStr, _countof(SearchStr), L"%s\\*.%s", PickupDir, pCopyStartPtr );
	} else {
		swprintf_s( SearchStr, _countof(SearchStr), L"%s\\*.*", PickupDir );
	}

	// 全てのファイルを列挙する
	hSearch	= FindFirstFileW( SearchStr, &fd );
	if( INVALID_HANDLE_VALUE==hSearch ) {
		return FALSE;
	}

	do {
		swprintf_s( DeleteFileName, L"%s\\%s", PickupDir, fd.cFileName );
		DeleteFileW( DeleteFileName );
	} while( FindNextFileW( hSearch, &fd ) );

	FindClose( hSearch );

	return TRUE;
}

//-----------------------------------------------------------------------------
//!	拡張子を変更する。拡張子は .(ドット)がついててもついてなくてもよい
//-----------------------------------------------------------------------------
bool changeExtension( char * pDestName, const size_t Length, const char * pFileName, const char * pExtension )
{
	if( !pDestName )	return false;
	if( !Length )		return false;
	if( !pFileName )	return false;
	if( !pExtension )	return false;

	char pDrive[ _MAX_DRIVE ];
	char pDir[ _MAX_DIR ];
	char pFname[ _MAX_FNAME ];
	::_splitpath_s( pFileName, pDrive, _countof(pDrive), pDir, _countof(pDir), pFname, _countof(pFname), NULL, 0 );

	if( '.'==pExtension[0] ) {
		sprintf_s( pDestName, Length, "%s%s%s%s", pDrive, pDir, pFname, pExtension );
	} else {
		sprintf_s( pDestName, Length, "%s%s%s.%s", pDrive, pDir, pFname, pExtension );
	}

	return true;
}

//-----------------------------------------------------------------------------
//!	拡張子を変更する。拡張子は .(ドット)がついててもついてなくてもよい
//-----------------------------------------------------------------------------
bool changeExtension( wchar_t * pDestName, const size_t Length, const wchar_t * pFileName, const wchar_t * pExtension )
{
	if( !pDestName )	return false;
	if( !Length )		return false;
	if( !pFileName )	return false;
	if( !pExtension )	return false;

	wchar_t pDrive[ _MAX_DRIVE ];
	wchar_t pDir[ _MAX_DIR ];
	wchar_t pFname[ _MAX_FNAME ];
	::_wsplitpath_s( pFileName, pDrive, _countof(pDrive), pDir, _countof(pDir), pFname, _countof(pFname), NULL, 0 );

	if( L'.'==pExtension[0] ) {
		swprintf_s( pDestName, Length, L"%s%s%s%s", pDrive, pDir, pFname, pExtension );
	} else {
		swprintf_s( pDestName, Length, L"%s%s%s.%s", pDrive, pDir, pFname, pExtension );
	}

	return true;
}

//-----------------------------------------------------------------------------
//!	拡張子直接変更する。拡張子は .(ドット)がついててもついてなくてもよい。
//!	変更する拡張子は元の拡張子より文字列的に等しいかそれ以下の文字数でなければならない
//-----------------------------------------------------------------------------
bool changeExtensionDirect( char * pFileName, const char * pExtension )
{
	if( !pFileName )	return false;
	if( !pExtension )	return false;

	char	pExt[ _MAX_EXT ];
	size_t	FileLen, SrcExtLen, DstExtLen, Sub = 0;

	::_splitpath_s( pFileName, NULL, 0, NULL, 0, NULL, 0, pExt, _countof(pExt) );

	FileLen		= strlen( pFileName );
	SrcExtLen	= strlen( pExt );

	// 指定拡張子がドットから開始
	if( '.'==pExtension[ 0 ] ) {
		strcpy_s( pExt, _countof(pExt), pExtension + 1 );
		DstExtLen	= strlen( pExtension );
		SrcExtLen	-= 1;
	} else {
		strcpy_s( pExt, _countof(pExt), pExtension );
		DstExtLen	= strlen( pExtension );
	}

	if( DstExtLen <= SrcExtLen ) {
		for( size_t i=0; i<DstExtLen; ++i ) {
			pFileName[ FileLen - SrcExtLen + i ]	= pExt[ i ];
		}

		// 差分
		Sub	= SrcExtLen - DstExtLen;
		if( 0 < Sub ) {
			pFileName[ FileLen - Sub ]	= '\0';
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//!	拡張子直接変更する。拡張子は .(ドット)がついててもついてなくてもよい。
//!	変更する拡張子は元の拡張子より文字列的に等しいかそれ以下の文字数でなければならない
//-----------------------------------------------------------------------------
bool changeExtensionDirect( wchar_t * pFileName, const wchar_t * pExtension )
{
	if( !pFileName )	return false;
	if( !pExtension )	return false;

	wchar_t	pExt[ _MAX_EXT ];
	size_t	FileLen, SrcExtLen, DstExtLen, Sub = 0;

	::_wsplitpath_s( pFileName, NULL, 0, NULL, 0, NULL, 0, pExt, _countof(pExt) );

	FileLen		= wcslen( pFileName );
	SrcExtLen	= wcslen( pExt );

	// 指定拡張子がドットから開始
	if( L'.'==pExtension[ 0 ] ) {
		SrcExtLen	-= 1;
		wcscpy_s( pExt, _countof(pExt), pExtension + 1 );
		DstExtLen	= wcslen( pExtension );
	} else {
		wcscpy_s( pExt, _countof(pExt), pExtension );
		DstExtLen	= wcslen( pExtension );
	}

	if( DstExtLen <= SrcExtLen ) {
		for( size_t i=0; i<DstExtLen; ++i ) {
			pFileName[ FileLen - SrcExtLen + i ]	= pExt[ i ];
		}

		// 差分
		Sub	= SrcExtLen - DstExtLen;
		if( 0 < Sub ) {
			pFileName[ FileLen - Sub ]	= L'\0';
		}

		return true;
	}

	return false;
}

};/*namespace FileProxy*/

//---< end of file >-----------------------------------------------------------