#pragma once
#include "InputManager.h"

extern bool g_bImguiOn;

const unsigned int	g_iWinSizeX = 1280;
const unsigned int	g_iWinSizeY = 720;

extern HWND			g_hwnd;

extern ID3D12DescriptorHeap* g_ImguiSrvDescHeap;

namespace Client
{
}

using Engine::CInputManager;
extern CInputManager* g_Input;	

using namespace Client;
