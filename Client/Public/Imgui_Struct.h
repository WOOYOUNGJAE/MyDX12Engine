#pragma once
#include "Base.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
	class CGameObject;
_NAMESPACE
using namespace Engine;


NAMESPACE_(Client)
class CClient_Imgui;
_NAMESPACE
using namespace Client;

namespace IMGUI_CUSTOM
{
}
using namespace IMGUI_CUSTOM;

NAMESPACE_(IMGUI_CUSTOM)

struct OBJ_CUSTOMIZER
{
	OBJ_CUSTOMIZER() = delete;
	OBJ_CUSTOMIZER(CGameObject* pGameObject, CClient_Imgui* pInController) : pTarget(pGameObject), pController(pInController)
	{
	}
	~OBJ_CUSTOMIZER();

	static OBJ_CUSTOMIZER* Create(CGameObject* pGameObject, CClient_Imgui* pInController);
	void Imgui_Tick();

	bool bActive = true;
	CGameObject* pTarget = nullptr;
	UINT64* pTargetCbvSrvUavOffset = nullptr;
	UINT iCbvSrvUavDescriptorSize = 0;
	CClient_Imgui* pController = nullptr;
};
	
_NAMESPACE
