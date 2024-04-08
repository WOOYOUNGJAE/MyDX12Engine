#pragma once
#include "Base.h"
// DirectX Raytracing Renderer

#if DXR_ON

NAMESPACE_(Engine)
class CDXRResource;
class CDeviceResource;
struct FrameResource;
class ENGINE_DLL CDXRRenderer : public CBase
{
protected:
	CDXRRenderer() = default;
	~CDXRRenderer() override = default;

public:
	static CDXRRenderer* Create();
	HRESULT Initialize();
	HRESULT Free() override;

public: // Rendering
	void BeginRender();
	void MainRender();
	void EndRender();
	void Present();
	void Do_RayTracing();
public:
	void DispatchRay();
	void Set_ComputeRootDescriptorTable_Global(); // Global루트시그니쳐가 바인딩된 상태에서 테이블 바인딩

private: // Pointers
	CDeviceResource* m_pDeviceResource = nullptr;
	CDXRResource* m_pDXRResources = nullptr;
	ID3D12CommandAllocator** m_pCommandAllocatorArr = nullptr;
	ID3D12GraphicsCommandList4* m_pCommandList = nullptr;
	ID3D12StateObject* m_pDXR_PSO = nullptr;
	ID3D12Resource** m_pRenderTargetArr = nullptr;
	//std::vector<class CBVH*>* m_pVecAccelerationTree = nullptr;
public:
	UINT m_iFrameIndex = 0; // Equal with DeviceResource's
private: // FrameResource
	std::vector<FrameResource*> m_vecFrameResource;
	FrameResource* m_pCurFrameResource = nullptr;
	UINT m_iCurFrameResourceIndex = 0;
private:
	D3D12_DISPATCH_RAYS_DESC m_disptchRaysDesc{};

};

_NAMESPACE

#endif