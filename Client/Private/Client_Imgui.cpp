#include "pch.h"
#include "Client_Imgui.h"

#include "GameInstance.h"
#include "Renderer.h"
#include "ComponentManager.h"
#include "Imgui_Struct.h"

CClient_Imgui* CClient_Imgui::Create(ID3D12Device* pDevice)
{
    CClient_Imgui* pInstance = new CClient_Imgui();

    if (FAILED(pInstance->Initialize(pDevice)))
    {
        MSG_BOX("Client_Imgui Create Failed");
        return nullptr;
    }

    return pInstance;
}

HRESULT CClient_Imgui::Initialize(ID3D12Device* pDevice)
{
    HRESULT hr = S_OK;

    m_pRenderer = reinterpret_cast<CRenderer*>(CComponentManager::Get_Instance()->FindandGet_Prototype(L"Renderer"));
    Safe_AddRef(m_pRenderer);


    // Create Srv Heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 2;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pImguiSrvHeap));
    if (FAILED(hr)) { return E_FAIL; }

    

    pDevice->CopyDescriptorsSimple(
        2,
        D3D12_CPU_DESCRIPTOR_HANDLE(m_pImguiSrvHeap->GetCPUDescriptorHandleForHeapStart()),
            m_pRenderer->Get_CbvSrvUavStart_CPU(),
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_CbvSrvUavHeapStart = m_pRenderer->Get_CbvSrvUavStart_GPU();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_pIo = &ImGui::GetIO();
    
    m_pIo->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    m_pIo->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    m_pIo->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    m_pIo->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (m_pIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX12_Init(pDevice, m_iNumFramesInFlight,
        DXGI_FORMAT_R8G8B8A8_UNORM, m_pImguiSrvHeap,
        m_pImguiSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_pImguiSrvHeap->GetGPUDescriptorHandleForHeapStart());

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // ObjCustomizer
    CGameInstance* pG_Instance = CGameInstance::Get_Instance();
    Safe_AddRef(pG_Instance);

    CGameObject* pTargetObj = nullptr;
    pTargetObj = pG_Instance->FindandGet_GameObj_Cloned(L"Triangle");
    m_vecObjCustomizer.push_back(OBJ_CUSTOMIZER::Create(pTargetObj, this));

    Safe_Release(pG_Instance);
    return hr;
}

void CClient_Imgui::Imgui_Tick()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    for (auto& iter : m_vecObjCustomizer)
    {
        iter->Imgui_Tick();
    }

    // Rendering
    ImGui::Render();
}

void CClient_Imgui::Imgui_MainRender()
{
    m_pRenderer->Get_CmdList()->SetDescriptorHeaps(1, &m_pImguiSrvHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pRenderer->Get_CmdList());

}

void CClient_Imgui::IMgui_EndRender()
{
    m_pRenderer->EndRender();
}

void CClient_Imgui::Imgui_Present()
{
	// Update and Render additional Platform Windows
    if (m_pIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, (void*)m_pRenderer->Get_CmdList());
    }

    // Renderer -> Present
    m_pRenderer->Present();
}

HRESULT CClient_Imgui::Free()
{
    m_pRenderer->Flush_CommandQueue();

    for (auto& iter : m_vecObjCustomizer)
    {
        Safe_Delete(iter);
    }

    // Release Imgui
    Safe_Release(m_pImguiSrvHeap);
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Safe_Release(m_pRenderer);

    return S_OK;
}

void CClient_Imgui::Draw_ImguiImage(UINT64 const iOffset, ImVec2 imVec2)
{
    m_CbvSrvUavHeapStart.Offset(iOffset);
    m_CbvSrvUavHeapStart = m_pRenderer->Get_CbvSrvUavStart_GPU();
    ImGui::Image((ImTextureID)m_CbvSrvUavHeapStart.ptr, imVec2);
}
