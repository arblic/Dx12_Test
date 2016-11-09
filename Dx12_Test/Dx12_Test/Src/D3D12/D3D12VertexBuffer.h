//-----------------------------------------------------------------------------
//! @file   D3D12VertexBuffer.h
//! @brief  D3D12 頂点バッファ
//! @author 
//-----------------------------------------------------------------------------
#ifndef __D3D12_VERTEXBUFFER_H__
#define	__D3D12_VERTEXBUFFER_H__

//-----------------------------------------------------------------------------
//!	D3D12
//-----------------------------------------------------------------------------
namespace D3D12 {

//-----------------------------------------------------------------------------
//!	頂点バッファ
//-----------------------------------------------------------------------------
class cVertexBuffer {
public:
	cVertexBuffer( void );
	virtual~cVertexBuffer( void );

	bool create( cDevice * pDevice, const void * pData, size_t Size, size_t Stride );
	void destroy( void );

private:
public:
	ID3D12Resource *		m_pBuffer;
	UINT					m_Size;
	UINT					m_Stride;
};

};/*namespace D3D12*/

#endif/*__D3D12_VERTEXBUFFER_H__*/

//---< end of file >-----------------------------------------------------------