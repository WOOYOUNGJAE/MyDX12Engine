#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class CAsset;
class CTexture;
class CMeshData;

// Manage MeshData, Texture
class ENGINE_DLL CAssetManager : public CBase
{
	DECLARE_SINGLETON(CAssetManager)
protected:
	CAssetManager() = default;
	~CAssetManager() override = default;

public:
	HRESULT Free() override;
public: // Getter
	CMeshData** Get_SingleMeshDataArr() { return m_pSingleMeshDataArr; }
	map<wstring, list<CMeshData*>>& Get_MeshData_ClusteredMap() { return m_mapMeshData_Clustered; }
public:
	HRESULT Add_Texture(const wstring& strAssetName, CTexture* pTextureInstance);
	CTexture* FindandGet_Texture(const wstring& strAssetName);
public:
	HRESULT Add_MeshDataPrototype(GEOMETRY_TYPE eGeometryType, CMeshData* pMeshData);
	CMeshData* FindandGet_MeshData(GEOMETRY_TYPE eGeometryType);
	CMeshData* Clone_MeshData(GEOMETRY_TYPE eGeometryType, void* pArg);
	HRESULT Add_MeshData_ClusteredPrototype(const wstring& strPrototypeTag, list<CMeshData*> meshDataList);
	list<CMeshData*>& FindandGet_MeshData_Clustered(const wstring& strPrototypeTag);
	list<CMeshData*> Clone_MeshData_Clustered(const wstring& strPrototypeTag);
private:
	map<wstring, CTexture*> m_mapTextures;
	CMeshData* m_pSingleMeshDataArr[GEOMETRY_TYPE::GEOMETRY_TYPE_COUNT]{};
	map<wstring, list<CMeshData*>> m_mapMeshData_Clustered; // 여러 MeshData 모여있는
#if DXR_ON
public:
	void Build_IB_VB_SRV_Serialized(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList, UINT iStructureByteStride);
	UINT64 Get_IB_VB_SRV_startOffset() { return IB_VB_SRV_startOffsetInDescriptors; }
private:
	UINT64 IB_VB_SRV_startOffsetInDescriptors;
	ID3D12Resource* m_pUploadBuffer_CombinedIndices = nullptr;
	ID3D12Resource* m_pUploadBuffer_CombinedVertices = nullptr;
	ID3D12Resource* m_pCombinedPrototypeMeshIndices = nullptr; // 생성된 모든 메쉬 프로토타입들 인덱스들 배열 리소스로 저장
	ID3D12Resource* m_pCombinedPrototypeMeshVertices = nullptr;
#endif
};


_NAMESPACE
