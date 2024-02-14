#pragma once
#include "Base.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
class CRenderer;
_NAMESPACE

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
private: // Pointer
	CRenderer* m_pRenderer = nullptr;
private:
	bool m_show_demo_window = true;
	bool m_show_another_window = false;
	INT m_iNumFramesInFlight = 2;
	ID3D12DescriptorHeap* m_pImguiSrvHeap = nullptr;
	ImGuiIO* m_pIo = nullptr;
	ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

_NAMESPACE