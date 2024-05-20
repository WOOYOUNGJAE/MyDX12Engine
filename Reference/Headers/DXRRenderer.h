#pragma once
#include "Base.h"
#include "UploadBuffer.h"
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
	void Set_FrameResource(); // FrameResourceMgr 생성 후 호출
	void Update_Static_PassCB(const CAMERA_DESC& camDesc); // static pass data
	void Update_Dynamic_PassCB(); // dynamic pass data
	void Update_Static_Object_CB();
	void Update_Dynamic_Object_CB();
	void BeginRender();
	void MainRender();
	void EndRender();
	void Present();

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
private: // Pass Info
	DXR::SCENE_CB curSceneCB{};
	Matrix m_mProj;
private:
	D3D12_DISPATCH_RAYS_DESC m_disptchRaysDesc{};
private:
	map<UINT, class CObjLayer*>* pAllObjLayers = nullptr;
public:
	static ID3D12Resource* m_bottomLevelAccelerationStructure;
};

_NAMESPACE

#endif