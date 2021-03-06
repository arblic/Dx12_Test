﻿//-----------------------------------------------------------------------------
//! @file   FileProxy.h
//! @brief  ファイルプロキシ
//! @author keisuke kamakami.
//-----------------------------------------------------------------------------
#ifndef __FILEPROXY_H__
#define	__FILEPROXY_H__

//-----------------------------------------------------------------------------
//!	FileProxy 名前空間
//-----------------------------------------------------------------------------
namespace FileProxy {

	size_t	getAttr( const char * pFileName );
	size_t	getAttr( const wchar_t * pFileName );

	bool	isExist( const char * pFileName );
	bool	isExist( const wchar_t * pFileName );

	bool	isWritable( const char * pFileName );
	bool	isWritable( const wchar_t * pFileName );

	size_t	getSize( const char * pFileName );
	size_t	getSize( const wchar_t * pFileName );

	size_t	read( const char * pFileName, void * pBuffer, const size_t BuffSize );
	size_t	read( const wchar_t * pFileName, void * pBuffer, const size_t BuffSize );

	size_t	write( const char * pFileName, const void * pBuffer, const size_t BuffSize, const size_t Offset=0 );
	size_t	write( const wchar_t * pFileName, const void * pBuffer, const size_t BuffSize, const size_t Offset=0 );

	bool	createDir( const char * pDirectoryName );
	bool	createDir( const wchar_t * pDirectoryName );

	bool	cleanDir( const char * pDirectoryName, const char * pExtention );
	bool	cleanDir( const wchar_t * pDirectoryName, const wchar_t * pExtention );

	bool	changeExtension( char * pDestName, const size_t Length, const char * pFileName, const char * pExtension );
	bool	changeExtension( wchar_t * pDestName, const size_t Length, const wchar_t * pFileName, const wchar_t * pExtension );

	bool	changeExtensionDirect( char * pFileName, const char * pExtension );
	bool	changeExtensionDirect( wchar_t * pFileName, const wchar_t * pExtension );

};/*namespace FileProxy*/

#endif	/*__FILEPROXY_H__*/

//---< end of file >-----------------------------------------------------------