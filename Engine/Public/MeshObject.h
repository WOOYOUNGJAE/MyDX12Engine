#pragma once
#include "Component.h"

NAMESPACE_(Engine)

// MeshData들을 갖고 있는 컴포넌트 오브젝트
class CMeshData;
class CMeshObject : public CComponent
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
	std::vector<CMeshData*>* Get_vecMeshData() { return &m_vecMeshData; }
	virtual D3D12_VERTEX_BUFFER_VIEW* Get_VertexBufferViewPtr(UINT iMeshIndex = 0);
	virtual D3D12_INDEX_BUFFER_VIEW* Get_IndexBufferViewPtr(UINT iMeshIndex = 0);
public:
	void Add_MeshData(CMeshData* pMeshData);
	std::vector<CMeshData*> m_vecMeshData;


};

_NAMESPACE