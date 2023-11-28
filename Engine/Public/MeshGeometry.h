#pragma once
#include "Component.h"
#include "Engine_Defines.h"

NAMESPACE_(Engine)
	// MeshGeometry는 Prototype만 가능, Clone 비허용.
class CMeshGeometry abstract: public CComponent
{
protected:
	CMeshGeometry();
	CMeshGeometry(const CMeshGeometry& rhs): CComponent(rhs),
	m_pDevice(rhs.m_pDevice),
	m_pCommandList(rhs.m_pCommandList)
	{}
	~CMeshGeometry() override = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Free() override;
public: // getter setter
	virtual D3D12_VERTEX_BUFFER_VIEW VertexBufferView();
	virtual D3D12_INDEX_BUFFER_VIEW IndexBufferView() const;
	virtual _uint Num_Indices() { return m_iNumIndices; }

protected:
	ComPtr<ID3D12Device> m_pDevice = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
	MY_VERTEX* m_vertexData = nullptr; // Array
	_float3* m_vertexPosArr = nullptr;
	// Index 상속해서? TODO
	ComPtr<ID3D12Resource> m_pResource = nullptr;
	// 임시 저장공간
	ComPtr<ID3DBlob> m_vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> m_indexBufferCPU = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferUploader = nullptr;
	
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