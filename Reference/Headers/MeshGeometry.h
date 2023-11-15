#pragma once
#include "Component.h"
#include "Engine_Defines.h"
class CMeshGeometry abstract: public CComponent
{
protected:
	CMeshGeometry() = default;
	~CMeshGeometry() override = default;

public:
	virtual HRESULT Free() override;

protected:

protected:
	MY_VERTEX* m_vertexData = nullptr; // Array
	_float3* m_vertexPosArr = nullptr;
	// Index ����ؼ�? TODO
	ComPtr<ID3D12Resource> m_pResource = nullptr;
	// �ӽ� �������
	ComPtr<ID3DBlob> m_vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> m_indexBufferCPU = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferUploader = nullptr;
};
