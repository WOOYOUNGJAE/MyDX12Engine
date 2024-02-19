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

	pInstance->fPosArr[0] = pGameObject->Get_Pos().x;
	pInstance->fPosArr[1] = pGameObject->Get_Pos().y;
	pInstance->fPosArr[2] = pGameObject->Get_Pos().z;

	pInstance->fScaleArr[0] = pGameObject->Get_ScaleXYZ().x;
	pInstance->fScaleArr[1] = pGameObject->Get_ScaleXYZ().y;
	pInstance->fScaleArr[2] = pGameObject->Get_ScaleXYZ().z;

	

	return pInstance;
}

void IMGUI_CUSTOM::OBJ_CUSTOMIZER::Imgui_Tick()
{
	CD3DResourceManager* pResourceManager = CD3DResourceManager::Get_Instance();

	Begin("Object Customizer");

	Text("Object Tag : %ls [%d]", pTarget->Get_PrototypeTag().c_str(), pTarget->Get_ClonedNum());
	NewLine();

	Text("Texture"); SameLine();
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

	Text("Scale (x, y, z)");
	if (InputFloat3("Scale ", fScaleArr) == false)
	{
		pTarget->Set_Scale(Vector3(fScaleArr[0], fScaleArr[1], fScaleArr[2]));
	}

	Text("Position (x, y, z)");
	if (InputFloat3("Position ", fPosArr) == false)
	{
		pTarget->Set_Position(Vector3(fPosArr[0], fPosArr[1], fPosArr[2]));
	}


	End();
}
 //  ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
 //  ImGui::Checkbox("Demo Window", &m_show_demo_window);      // Edit bools storing our window open/close state
 //  ImGui::Checkbox("Another Window", &m_show_another_window);

 //  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
 //  ImGui::ColorEdit3("clear color", (float*)&m_clear_color); // Edit 3 floats representing a color

 //  if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
 //      counter++;
 //  ImGui::SameLine();
 //  ImGui::Text("counter = %d", counter);
 //  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_pIo->Framerate, m_pIo->Framerate);