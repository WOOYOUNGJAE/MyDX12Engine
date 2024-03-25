#pragma once
#include "Component.h"

NAMESPACE_(Engine)
using namespace std;
// MeshData들을 갖고 있는 컴포넌트 오브젝트
class CMeshData;
class ENGINE_DLL CMeshObject : public CComponent
{
protected:
    CMeshObject() = default;
    CMeshObject(CMeshObject& rhs);
    ~CMeshObject() override = default;

public:
	static CMeshObject* Create();
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;
public: // getter setter
	vector<CMeshData*>* Get_vecMeshData() { return &m_vecMeshData; }
	virtual D3D12_VERTEX_BUFFER_VIEW* Get_VertexBufferViewPtr(UINT iMeshIndex = 0);
	virtual D3D12_INDEX_BUFFER_VIEW* Get_IndexBufferViewPtr(UINT iMeshIndex = 0);
	virtual UINT Get_CbvSrvUavOffset(UINT iMeshIndex = 0);
public:
	void Add_MeshData(CMeshData* pMeshData);
	vector<CMeshData*> m_vecMeshData;

#if DXR_ON
public:
	//DXR::ACCELERATION_STRUCTURE_CPU Create_AccelerationStructures_CPU(CMeshData* pMeshData);
	HRESULT Build_AccelerationStructures();
	HRESULT Build_BLAS(DXR::ACCELERATION_STRUCTURE_CPU& refAS_CPU);
public: // Acceleration Structure
	/*ID3D12Resource* m_TLAS = nullptr;
	vector<ID3D12Resource*> m_vecBLAS;
	vector<DXR::ACCELERATION_STRUCTURE_CPU> m_vecAS_CPU;*/
	vector<DXR::BLAS> m_vecBlas;
#endif

};

_NAMESPACE