#pragma once

#include "MeshGeometry.h"

NAMESPACE_(Engine)
// TriangleMesh with Position, Texture Typed Vertex
class CTriangleMesh_PT : public CMeshGeometry
{
private:
	CTriangleMesh_PT();
	CTriangleMesh_PT(CTriangleMesh_PT& rhs);
	~CTriangleMesh_PT() override = default;

public:
	static CTriangleMesh_PT* Create();
	CComponent* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;

private:
	VertexPositionTexture* m_vertexData = nullptr; // Array
};


_NAMESPACE