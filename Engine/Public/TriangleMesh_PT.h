#pragma once

#include "MeshData.h"

NAMESPACE_(Engine)
// TriangleMesh with PositionVal, Texture Typed Vertex
class CTriangleMesh_PT : public CMeshData
{
private:
	CTriangleMesh_PT();
	CTriangleMesh_PT(CTriangleMesh_PT& rhs);
	~CTriangleMesh_PT() override = default;

public:
	static CTriangleMesh_PT* Create();
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;

private:
	//VertexPositionNormalTexture* m_vertexData = nullptr; // Array
	VertexPositionNormalTexture* m_vertexData = nullptr; // Array
};


_NAMESPACE