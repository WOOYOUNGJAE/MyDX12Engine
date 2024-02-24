#pragma once
#include "MeshGeometry.h"

NAMESPACE_(Engine)

// Texture��ǥ�� �־�� �ؼ� �� 4*6��, �ε��� 6*6��
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
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init�� Copy��)
	HRESULT Free() override;

private:
	VertexPositionNormalTexture* m_vertexData = nullptr; // Array

};

_NAMESPACE