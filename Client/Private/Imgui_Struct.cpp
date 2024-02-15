#include "pch.h"
#include "Imgui_Struct.h"
#include "Client_Imgui.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "D3DResourceManager.h"

using namespace ImGui;

IMGUI_CUSTOM::OBJ_CUSTOMIZER::~OBJ_CUSTOMIZER()
{
}

IMGUI_CUSTOM::OBJ_CUSTOMIZER* IMGUI_CUSTOM::OBJ_CUSTOMIZER::Create(CGameObject* pGameObject, CClient_Imgui* pInController)
{
	OBJ_CUSTOMIZER* pInstance = new OBJ_CUSTOMIZER(pGameObject, pInController);

	pInstance->pTargetCbvSrvUavOffset = pInstance->pTarget->Get_CbvSrvUavOffsetPtr();
	pInstance->iCbvSrvUavDescriptorSize = CGameInstance::Get_Instance()->Get_CbvSrvUavDescriptorSize();
	pInstance->pController = pInController;

	return pInstance;
}

void IMGUI_CUSTOM::OBJ_CUSTOMIZER::Imgui_Tick()
{
	CD3DResourceManager* pResourceManager = CD3DResourceManager::Get_Instance();

	Begin("Object Customizer");

	Text("Object Tag : %ls [%d]", pTarget->Get_PrototypeTag().c_str(), pTarget->Get_ClonedNum());


	//ImVec2 rect = ImGui::GetItemRectSize();
	ImVec2 rect =

	ImGui::GetWindowContentRegionMax() * 0.5f;
	//pController->Draw_ImguiImage(*pTargetCbvSrvUavOffset, rect);
	Text("Texture");
	SameLine();
	if (Button("Prev"))
	{
		if (*pTargetCbvSrvUavOffset >= iCbvSrvUavDescriptorSize)
		{
			*pTargetCbvSrvUavOffset -= iCbvSrvUavDescriptorSize;
		}
	}
	SameLine();
	if (Button("Next"))
	{
		if (*pTargetCbvSrvUavOffset + iCbvSrvUavDescriptorSize <= pResourceManager->Get_LastSrvOffset())
		{
			*pTargetCbvSrvUavOffset += iCbvSrvUavDescriptorSize;
		}
	}

	End();
}
 //{
 //       static float f = 0.0f;
 //       static int counter = 0;

 //       ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

 //       ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
 //       ImGui::Checkbox("Demo Window", &m_show_demo_window);      // Edit bools storing our window open/close state
 //       ImGui::Checkbox("Another Window", &m_show_another_window);

 //       ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
 //       ImGui::ColorEdit3("clear color", (float*)&m_clear_color); // Edit 3 floats representing a color

 //       if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
 //           counter++;
 //       ImGui::SameLine();
 //       ImGui::Text("counter = %d", counter);

 //       ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_pIo->Framerate, m_pIo->Framerate);
 //       ImGui::End();
 //   }