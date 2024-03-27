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
#include <set>
#include <algorithm>
#include <filesystem>

#include "d3dx12.h"
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

// Imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma comment(lib, "..\\..\\External\\Assimp\\assimp-vc143-mt")


using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
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

// My
#define DXR_ON TRUE

#include "Engine_Typedef.h"
#include "Engine_Functions.h"
#include "Engine_Macros.h"
#include "Engine_Struct.h"
#include "Engine_Enums.h"
#include "MyMath.h"
// namespace
using namespace Engine;
using namespace Engine::MyMath;
using std::wstring;
// extern
extern _uint g_iNumFrameResource;