#pragma once
#include "Base.h"
#include "Client_Defines.h"

NAMESPACE_(Engine)
	class CGameObject;
_NAMESPACE
using namespace Engine;

namespace IMGUI_CUSTOM
{
}
using namespace IMGUI_CUSTOM;

NAMESPACE_(IMGUI_CUSTOM)
	struct OBJ_CUSTOMIZER
	{
		OBJ_CUSTOMIZER() = delete;
		OBJ_CUSTOMIZER(CGameObject* pGameObject) : pTarget(pGameObject)
		{
		}
		~OBJ_CUSTOMIZER();

		static OBJ_CUSTOMIZER* Create(CGameObject* pGameObject);
		void Imgui_Tick();

		bool bActive = true;
		CGameObject* pTarget = nullptr;
		UINT64* pTargetCbvSrvUavOffset = nullptr;
	};
	
_NAMESPACE
