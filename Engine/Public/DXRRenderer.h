#pragma once
#include "Base.h"
// DirectX Raytracing Renderer

#if DXR_ON

NAMESPACE_(Engine)
class CDXRResource;
class CDXRRenderer : public CBase
{
protected:
	CDXRRenderer() = default;
	~CDXRRenderer() override = default;

public:
	static CDXRRenderer* Create();
	HRESULT Initialize();
	HRESULT Free() override;
	void Do_RayTracing();
public:
	void DispatchRay();
	void Set_ComputeRootDescriptorTable_Global(); // Global��Ʈ�ñ״��İ� ���ε��� ���¿��� ���̺� ���ε�

private: // Pointers
	CDXRResource* m_pDXRResources = nullptr;
	ID3D12GraphicsCommandList4* m_pCommandList = nullptr;
	ID3D12StateObject* m_pDXR_PSO = nullptr;
private:
	D3D12_DISPATCH_RAYS_DESC m_disptchRaysDesc{};
};

_NAMESPACE

#endif