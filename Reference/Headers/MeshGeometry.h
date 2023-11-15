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
	virtual HRESULT Free() override;

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
};

_NAMESPACE