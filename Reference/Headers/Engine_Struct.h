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
		enum INPUT_LAYOUT_ENUM {POS_COLOR, POS_TEXCOORD, POS_NORMAL_TEXCOORD, INPUT_LAYOUT_ENUM_END};
		INPUT_LAYOUT_ENUM inputLayout;
		std::wstring filename; // ���
		D3D_SHADER_MACRO* defines; // ��޿ɼ�, �Ϲ������� nullptr
		std::string entrypoint; // ���̴� ���α׷� ������ �Լ� �̸�, �ϳ��� hlsl���� ���� �Լ�(���̴�) ����
		std::string target; // ���̴� ������ ����
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
