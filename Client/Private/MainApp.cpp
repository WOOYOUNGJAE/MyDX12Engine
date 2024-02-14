#include "pch.h"
#include "MainApp.h"
#include "Renderer.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "Camera_Free.h"
#include "ClientLoader.h"
CMainApp::CMainApp() : m_pGameInstance{ CGameInstance::Get_Instance()}
{
	Safe_AddRef(m_pGameInstance);
}

CMainApp::~CMainApp()
{
}

HRESULT CMainApp::Initialize()
{
	GRAPHIC_DESC graphic_desc{};
	graphic_desc.hWnd = g_hwnd;
	graphic_desc.iSizeX = g_iWinSizeX;
	graphic_desc.iSizeY = g_iWinSizeY;
	graphic_desc.eWinMode = GRAPHIC_DESC::WINMODE_WIN;

	CGameObject* pObjectControlling = nullptr;

	HRESULT hr = 
	m_pGameInstance->Init_Engine(graphic_desc, &m_pDevice);
	if (FAILED(hr)) { return hr; }

	// Loader
	CClientLoader* pLoader = CClientLoader::Create();
	pLoader->Load();
	Safe_Release(pLoader);



#pragma region InLevel
	hr = m_pGameInstance->Add_GameObjPrototype(L"Camera_Free", CCamera_Free::Create());
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObject_InScene(L"Camera_Free", L"Default");
	if (FAILED(hr)) { return hr; }
	hr = m_pGameInstance->Add_GameObject_InScene(L"Triangle", L"Layer0", &pObjectControlling);
	if (FAILED(hr)) { return hr; }
	/*hr = m_pGameInstance->Add_GameObject_InScene(L"Triangle", L"Layer0", &pObjectControlling);
	if (FAILED(hr)) { return hr; }*/
	/*hr = m_pGameInstance->Add_GameObject_InScene(L"Cube", L"Layer0", &pObjectControlling);
	if (FAILED(hr)) { return hr; }*/
	// m_pGameInstance->Update_ObjPipelineLayer(pObjectControlling, Pipeline::ENUM_PSO::PSO_DEFAULT);
#pragma endregion InLevel

	// Renderer
	m_pRenderer = m_pGameInstance->Get_Instance()->Get_Renderer();

	if (m_pRenderer == nullptr)
	{
		MSG_BOX("MainApp: Renderer is nullptr");
		return E_FAIL;
	}

	hr = Init_Imgui();
	if (FAILED(hr))
	{
		MSG_BOX("IMGUI Init Failed");
		return E_FAIL;
	}

	
	return S_OK;
}
void CMainApp::Tick(_float fDeltaTime)
{
	m_pRenderer->BeginRender();

	m_pGameInstance->Engine_Tick(fDeltaTime);

	m_pRenderer->MainRender();
	m_pRenderer->EndRender();
	m_pRenderer->Present();
}

HRESULT CMainApp::Free()
{
	// Release Imgui
	Safe_Release(g_ImguiSrvDescHeap);
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_pRenderer);
	Safe_Release(m_pGameInstance);

	CGameInstance::Release_Engine();


	
	return S_OK;
}

HRESULT CMainApp::Init_Imgui()
{
	HRESULT hr = S_OK;

	// Imugui
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_ImguiSrvDescHeap));

	 // Imgui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX12_Init(m_pDevice, 3/*NUM_FRAMES_IN_FLIGHT*/,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_ImguiSrvDescHeap,
        g_ImguiSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_ImguiSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

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

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	return hr;
}

CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp;

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Init CMainApp");
		return nullptr;
	}

	return pInstance;
}
