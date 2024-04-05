#pragma once
#include "Base.h"

NAMESPACE_(Engine)
struct FrameResource;
class CFrameResourceManager : public CBase
{
	friend class CRenderer;
	friend class CDXRRenderer;
	DECLARE_SINGLETON(CFrameResourceManager)
protected:
	CFrameResourceManager() = default;
	~CFrameResourceManager() override = default;

public:
	HRESULT Initialize(); // DeviceResource ���� �� ����
	HRESULT Build_FrameResource(); // Build FrameResource, Build CBV inside, SRV�ε� �� ����
	HRESULT Free() override;
private: // FrameResource
	std::vector<FrameResource*> m_vecFrameResource;
	FrameResource* m_pCurFrameResource = nullptr;
	UINT m_iCurFrameResourceIndex = 0;
private:
	UINT m_iObjCBVHeapStartOffset = 0;
	UINT m_iPassCBVHeapStartOffset = 0;
	UINT m_iObjCbvDescriptorSize = 0;
};

_NAMESPACE