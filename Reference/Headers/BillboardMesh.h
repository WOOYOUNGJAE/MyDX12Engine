#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)
// Create에 어떠한 인자를 넣지 않으면 그냥 Quad
class ENGINE_DLL CBillboardMesh : public CMeshData
{
private:
	CBillboardMesh();
	CBillboardMesh(CBillboardMesh& rhs);
	~CBillboardMesh() override = default;

public:
	static CBillboardMesh* Create();
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init은 Copy용)
	HRESULT Free() override;

private:
	vector<VertexPositionNormalTexture> m_vecVertexData;
	vector<UINT32> m_vecIndexData; // UINT16 쓰는 메쉬는 자식에 따로 구현
};

_NAMESPACE