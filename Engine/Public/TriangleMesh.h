#pragma once
#include "MeshGeometry.h"

NAMESPACE_(Engine)

class CTriangleMesh : public CMeshGeometry
{

private:
	CTriangleMesh();
	CTriangleMesh(CTriangleMesh& rhs);
	~CTriangleMesh() override = default;

public:
	static CTriangleMesh* Create();
	CComponent* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init�� Copy��)
	HRESULT Free() override;

private:
	VertexPositionColor* m_vertexData = nullptr; // Array
};

_NAMESPACE