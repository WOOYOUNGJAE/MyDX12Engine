#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)

class CTriangleMesh : public CMeshData
{

private:
	CTriangleMesh();
	CTriangleMesh(CTriangleMesh& rhs);
	~CTriangleMesh() override = default;

public:
	static CTriangleMesh* Create();
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;

private:
	VertexPositionColor* m_vertexData = nullptr; // Array
};

_NAMESPACE