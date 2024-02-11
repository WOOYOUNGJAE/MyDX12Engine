#pragma once

namespace Engine
{
	struct GRAPHIC_DESC
	{
		enum WINMODE { WINMODE_FULL, WINMODE_WIN, WINMODE_END };
		HWND			hWnd;
		unsigned int	iSizeX, iSizeY;
		WINMODE			eWinMode;
	};

	struct MY_VERTEX
	{
		enum VERTEX_ELEM
		{
			POS,
			COLOR,
			NORMAL,
		};
		_float3 pos;
		_float4 color;
		// TODO ..추가 예정
	};

	struct SHADER_INIT_DESC
	{
		std::wstring filename; // 경로
		D3D_SHADER_MACRO* defines; // 고급옵션, 일반적으로 nullptr
		std::string entrypoint; // 쉐이더 프로그램 짐입점 함수 이름, 하나의 hlsl에는 여러 함수(쉐이더) 가능
		std::string target; // 쉐이더 종류와 버전
	};

	struct VERTEX_INPUT_ELEMENT_DESC_BUILT_IN
	{
		VERTEX_INPUT_ELEMENT_DESC_BUILT_IN()
		{
			Default_Position = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			Default_Color = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		}
		D3D12_INPUT_ELEMENT_DESC Default_Position;
		D3D12_INPUT_ELEMENT_DESC Default_Color;
	};

	struct CONSTANT_BUFFER_DATA
	{
		CONSTANT_BUFFER_DATA()
		{
			XMStoreFloat4x4(&WorldViewProj, XMMatrixIdentity());
		}
		_float4x4 WorldViewProj;
	};

	struct QUEUE_FLUSH_DESC
	{
		UINT64* pCurFenceVal;
		ID3D12CommandQueue* pCommandQueue;
		ID3D12Fence* pFence;
		HANDLE* pFenceEvent;
	};

	struct MyVector
	{
		MyVector(float _x, float _y, float _z)
		{
			x = _x; y = _y; z = _z;
			XMFLOAT3 tempFloat3(x, y, z);
			vector = XMLoadFloat3(&tempFloat3);
		}

		MyVector operator+(const MyVector& rhs)
		{
			XMVECTOR tempVector = XMVectorAdd(vector, rhs.vector);
			XMFLOAT3 tempFloat3{};
			XMStoreFloat3(&tempFloat3, tempVector);
			return MyVector(tempFloat3.x, tempFloat3.y, tempFloat3.z);
		}

		float x;
		float y;
		float z;
		XMVECTOR vector;
	};

	struct TEXTURE_INIT_DESC
	{
		bool bIsCubeMap;
		ID3D12Device* pDevice;
		ResourceUploadBatch* pResourceUpload;
		std::wstring strPath;
		UINT iCbvSrvUavHeapOffset;
	};

	struct TEXTURE_LOAD_DESC // for Client
	{
		bool bIsCubeMap;
		std::wstring strPath;
	};
}
