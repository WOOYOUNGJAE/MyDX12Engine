#pragma once
#include "MeshGeometry.h"

NAMESPACE_(Engine)

// Texture좌표도 있어야 해서 점 4*6개, 인덱스 6*6개
class ENGINE_DLL CCubeMesh final: public CMeshData
{
private:
	CCubeMesh();
	CCubeMesh(CCubeMesh& rhs);
	~CCubeMesh() override = default;

public:
	static CCubeMesh* Create();
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init은 Copy용)
	HRESULT Free() override;

private:
	VertexPositionNormalTexture* m_vertexData = nullptr; // Array

};

_NAMESPACE