#pragma once
#include "FrameResource_Struct.h"

namespace Engine
{


#pragma region Init_Desc
	struct GRAPHIC_DESC
	{
		enum WINMODE { WINMODE_FULL, WINMODE_WIN, WINMODE_END };
		HWND			hWnd;
		unsigned int	iSizeX, iSizeY;
		WINMODE			eWinMode;
	};

	struct GAMEOBJECT_INIT_DESC
	{
		bool bMakeBLAS;
		Vector3 vStartPos;
		Vector3 vStartLook = Vector3(0, 0, 1);
		Vector3 vStartScale;
		std::wstring strTag;
		void* pArg;
	};

	struct GRIDOBJECT_INIT_DESC
	{
		UINT iNumSlices;
		UINT iNumStacks;
	};

	struct SHADER_INIT_DESC
	{
		enum INPUT_LAYOUT_ENUM { POS_COLOR, POS_TEXCOORD, POS_NORMAL_TEXCOORD, POS_SIZE, INPUT_LAYOUT_ENUM_END };
		INPUT_LAYOUT_ENUM inputLayout;
		std::wstring filename; // ���
		D3D_SHADER_MACRO* defines; // ��޿ɼ�, �Ϲ������� nullptr
		std::string entrypoint; // ���̴� ���α׷� ������ �Լ� �̸�, �ϳ��� hlsl���� ���� �Լ�(���̴�) ����
		std::string target; // ���̴� ������ ����
	};

	struct TEXTURE_INIT_DESC
	{
		bool bIsCubeMap;
		ID3D12Device* pDevice;
		ResourceUploadBatch* pResourceUpload;
		std::wstring strPath;
		UINT iCbvSrvUavHeapOffset;
	};

	struct MESHOBJ_INIT_DESC
	{
		bool bIsSingle;
		std::wstring strPrototypeTag;
	};
#pragma endregion



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

#if DXR_ON
	NAMESPACE_(DXR)
	// CPU���� ��� �����ϴ� AS, �޽� Vertex Index�� SRV�� ������ ����
	// Index Srv ������� ���� �ٷ� Vertex Srv ��������� ��
	struct ACCELERATION_STRUCTURE_CPU 
	{
		ID3D12Resource* srv_Vertex;
		ID3D12Resource* srv_Index;
		D3D12_RAYTRACING_GEOMETRY_DESC dxrGeometryDesc; // BLAS���� ������ �ʿ�
	};
	struct BLAS
	{
		ID3D12Resource* indexBuffer; // Rasterize ������������ ���ҽ� �״��
		ID3D12Resource* vertexBuffer; 
		ID3D12Resource* uav_BLAS;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accelerationStructureDesc;
		D3D12_RAYTRACING_GEOMETRY_DESC dxrGeometryDesc;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
		std::vector<UINT16> vecIndices;
		std::vector<VertexPositionNormalColorTexture> vecVertices;
		UINT iStartIndex_in_IB_SRV;
		UINT iStartIndex_in_VB_SRV;
	};

	struct TLAS
	{
		ID3D12Resource* uav_TLAS;
		ID3D12Resource* pInstanceDesc;
		UINT64 IB_VB_SRV_startOffsetInDescriptors;
	};

	struct TLAS_NODE_INIT_DESC
	{
		
		ID3D12Resource* pUAV_BLAS;
		UINT iNumBlas;
	};

	struct TABLE_RECORD_DESC
	{
		UINT iIdentifierSize;
		void* pIdentifierData;

		UINT iLocalRootArgumentSize;
		void* pLocalRootArgumentData;
	};

	_NAMESPACE
#endif
}
