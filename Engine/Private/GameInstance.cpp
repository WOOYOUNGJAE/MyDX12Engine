#include "GameInstance.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{
}

HRESULT CGameInstance::Free()
{
	return S_OK;
}

HRESULT CGameInstance::Init_Engine()
{
	
	return S_OK;
}

HRESULT CGameInstance::Release_Engine()
{
	return S_OK;
}
