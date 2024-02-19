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

	struct SHADER_INIT_DESC
	{
		std::wstring filename; // 경로
		D3D_SHADER_MACRO* defines; // 고급옵션, 일반적으로 nullptr
		std::string entrypoint; // 쉐이더 프로그램 짐입점 함수 이름, 하나의 hlsl에는 여러 함수(쉐이더) 가능
		std::string target; // 쉐이더 종류와 버전
	};

	struct GAMEOBJECT_INIT_DESC
	{
		Vector3 vStartPos;
		std::wstring strTag;
		void* pArg;
	};

	struct OBJ_CONSTANT_BUFFER
	{
		OBJ_CONSTANT_BUFFER()
		{
			XMStoreFloat4x4(&mWorldMat, XMMatrixIdentity());
			ZeroMemory(padding, sizeof(FLOAT) * 48);
		}
		Matrix mWorldMat;
	private:
		FLOAT padding[48];
	};
	struct PASS_CONSTANT_BUFFER
	{
		Matrix mViewMat;
		Matrix mProjMat;
	private:
		FLOAT padding[32];
	};

	struct CAMERA_DESC
	{
		FLOAT fFovy;
		FLOAT fAspectRatio;
		FLOAT fNear;
		FLOAT fFar;
	};


	struct QUEUE_FLUSH_DESC
	{
		UINT64* pCurFenceVal;
		ID3D12CommandQueue* pCommandQueue;
		ID3D12Fence* pFence;
		HANDLE* pFenceEvent;
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

	struct RENDERER_BUNDLE
	{
		ID3D12Device* pDevice = nullptr;
		ID3D12CommandAllocator* pCmdAllocator = nullptr;
		ID3D12CommandQueue* pCommandQueue = nullptr;
		ID3D12GraphicsCommandList* pCmdList = nullptr;
		IDXGISwapChain3* pSwapChain = nullptr;
		ID3D12Resource** pRenderTargetArr = nullptr;
		UINT iFrameIndex = 0;
	};

}
