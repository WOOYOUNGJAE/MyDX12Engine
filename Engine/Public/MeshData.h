#pragma once
#include "Component.h"
#include "Engine_Defines.h"

NAMESPACE_(Engine)
	// MeshGeometry는 Prototype만 가능, Clone 비허용.
class CMeshData abstract: public CBase
{
protected:
	CMeshData();
	CMeshData(CMeshData& rhs);
	~CMeshData() override = default;

public:
	virtual CMeshData* Clone(void* pArg = nullptr) = 0;
	virtual HRESULT Initialize_Prototype();
	void Init_VBV_IBV();
	virtual HRESULT Initialize(void* pArg) = 0;
	virtual HRESULT Free() override;
public: // getter setter
	virtual D3D12_VERTEX_BUFFER_VIEW* Get_VertexBufferViewPtr();
	virtual D3D12_INDEX_BUFFER_VIEW* Get_IndexBufferViewPtr();
	virtual _uint Num_Indices() { return m_iNumIndices; }

protected:
	ID3D12Device* m_pDevice = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	
	_float3* m_vertexPosArr = nullptr;
	//ComPtr<ID3D12Resource> m_pResource = nullptr;
	// 임시 저장공간
	ID3DBlob* m_vertexBufferCPU = nullptr;
	ID3DBlob* m_indexBufferCPU = nullptr;
	ID3D12Resource* m_vertexBufferGPU = nullptr;
	ID3D12Resource* m_indexBufferGPU = nullptr;
	ID3D12Resource* m_vertexUploadBuffer = nullptr;
	ID3D12Resource* m_indexUploadBuffer = nullptr;
protected: // View
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};
protected: // Data about the buffers.
	UINT m_iNumVertex = 0;
	UINT m_iNumIndices = 0;
	UINT m_iVertexByteStride = 0; 
	UINT m_iVertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT; 
	UINT m_iIndexBufferByteSize = 0;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

_NAMESPACE