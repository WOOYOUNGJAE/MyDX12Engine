#pragma once
#include "Base.h"
// DirectX Raytracing Renderer
class CDXRRenderer : public CBase
{
protected:
	CDXRRenderer() = default;
	~CDXRRenderer() override = default;

public:
	static CDXRRenderer* Create(ID3D12Device** _Inout_ ppDevice);
	HRESULT Initialize(ID3D12Device** _Inout_ ppDevice);
	HRESULT Free() override;
private:
	ID3D12GraphicsCommandList4* m_pDxrCommandList = nullptr; // CmdList for Dxr
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
};

