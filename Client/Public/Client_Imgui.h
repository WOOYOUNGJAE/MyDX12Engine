#pragma once
#include "Base.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
class CRenderer;
_NAMESPACE



NAMESPACE_(IMGUI_CUSTOM)
struct OBJ_CUSTOMIZER;
_NAMESPACE
using namespace IMGUI_CUSTOM;

NAMESPACE_(Client)

class CClient_Imgui : public CBase
{
protected:
	CClient_Imgui() = default;
	~CClient_Imgui() override = default;

public:
	static CClient_Imgui* Create(ID3D12Device* pDevice);
	HRESULT Initialize(ID3D12Device* pDevice);
	void Imgui_Tick();
	void Imgui_MainRender(); // Render Begin -> Main Render -> Render End -> Present
	void IMgui_EndRender();
	void Imgui_Present();
	HRESULT Free() override;
public:
	void Draw_ImguiImage(UINT64 const iOffset, ImVec2 imVec2);
private: // Pointer
	CRenderer* m_pRenderer = nullptr;
private:
	bool m_show_demo_window = true;
	bool m_show_another_window = false;
	INT m_iNumFramesInFlight = 2;
	ID3D12DescriptorHeap* m_pImguiSrvHeap = nullptr;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CbvSrvUavHeapStart;
	ImGuiIO* m_pIo = nullptr;
	ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
private: // My Imgui
	std::vector<OBJ_CUSTOMIZER*> m_vecObjCustomizer;
};

_NAMESPACE