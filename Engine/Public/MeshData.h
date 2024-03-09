#pragma once
#include "Component.h"
#include "Engine_Defines.h"

NAMESPACE_(Engine)
using std::vector;
	// MeshGeometry는 Prototype만 가능, Clone 비허용.
	// VertexPositionNormalTexture와 UINT32가 디폴트,
	// 그 외의 경우는 상속으로
class ENGINE_DLL CMeshData abstract: public CBase
{
protected:
	CMeshData();
	CMeshData(CMeshData& rhs);
	~CMeshData() override = default;

public:
	virtual CMeshData* Clone(void* pArg = nullptr) = 0;
	virtual HRESULT Initialize_Prototype();
	void Init_VBV_IBV();
	virtual HRESULT Initialize(void* pArg) = 0;
	virtual HRESULT Free() override;
public: // getter setter
	virtual D3D12_VERTEX_BUFFER_VIEW* Get_VertexBufferViewPtr();
	virtual D3D12_INDEX_BUFFER_VIEW* Get_IndexBufferViewPtr();
	virtual UINT Get_CbvSrvUavOffset() { return m_iCbvSrvUavOffset; }
	virtual _uint Num_Indices() { return m_iNumIndices; }
	void Set_CbvSrvUavOffset(UINT iOffset) { m_iCbvSrvUavOffset = iOffset; }
public: // getter
	vector<VertexPositionNormalTexture>& Get_vecVertices() { return m_vecVertexData; }
	vector<UINT32>& Get_vecIndices() { return m_vecIndexData; }
public: // static util func
	static void Normalize_Vertices(CMeshData* pMeshData);
	static void Normalize_Vertices(std::list<CMeshData*>& refMeshList);
	static void Normalize_Vertices(std::vector<CMeshData*>& refVecMesh);
protected:
	ID3D12Device* m_pDevice = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;

	//ComPtr<ID3D12Resource> m_pResource = nullptr;
	// 임시 저장공간
	ID3DBlob* m_vertexBufferCPU = nullptr;
	ID3DBlob* m_indexBufferCPU = nullptr;
	ID3D12Resource* m_vertexBufferGPU = nullptr;
	ID3D12Resource* m_indexBufferGPU = nullptr;
	ID3D12Resource* m_vertexUploadBuffer = nullptr;
	ID3D12Resource* m_indexUploadBuffer = nullptr;
protected: // View
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};
protected: // Data about the buffers.
	UINT m_iNumVertices = 0;
	UINT m_iNumIndices = 0;
	UINT m_iVertexByteStride = 0; 
	UINT m_iVertexBufferByteSize = 0;
	DXGI_FORMAT m_IndexFormat = DXGI_FORMAT_R32_UINT; 
	UINT m_iIndexBufferByteSize = 0;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
protected:
	UINT m_iCbvSrvUavOffset = UINT32_MAX; // 모든 메쉬 오브젝트가 텍스트를 필수로 들고 있지 않음
protected:
	vector<VertexPositionNormalTexture> m_vecVertexData;
	vector<UINT32> m_vecIndexData; // UINT16 쓰는 메쉬는 자식에 따로 구현
};

_NAMESPACE