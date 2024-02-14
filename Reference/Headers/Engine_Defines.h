#pragma once

#include <Windows.h>

// XTK12
#include "DDSTextureLoader.h"
#include "VertexTypes.h"
#include "ResourceUploadBatch.h"


#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include "d3dx12.h"
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
//#include <dxgi.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

// Imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

// libs
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif

#endif // _DEBUG

// My Headers
#include "Engine_Typedef.h"
#include "Engine_Functions.h"
#include "Engine_Macros.h"
#include "Engine_Struct.h"
#include "Engine_Enums.h"
// namespace
using namespace Engine;
using namespace Engine::MyMath;
using std::wstring;
// extern
extern _uint g_iNumFrameResource;