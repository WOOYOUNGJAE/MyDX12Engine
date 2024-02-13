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
		std::wstring filename; // ���
		D3D_SHADER_MACRO* defines; // ��޿ɼ�, �Ϲ������� nullptr
		std::string entrypoint; // ���̴� ���α׷� ������ �Լ� �̸�, �ϳ��� hlsl���� ���� �Լ�(���̴�) ����
		std::string target; // ���̴� ������ ����
	};

	struct OBJ_CONSTANT_BUFFER
	{
		OBJ_CONSTANT_BUFFER()
		{
			XMStoreFloat4x4(&WorldViewProj, XMMatrixIdentity());
			ZeroMemory(padding, sizeof(FLOAT) * 48);
		}
		_float4x4 WorldViewProj;
		FLOAT padding[48];
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

}
