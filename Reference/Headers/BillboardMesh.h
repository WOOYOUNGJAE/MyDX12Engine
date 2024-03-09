#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)
// Create�� ��� ���ڸ� ���� ������ �׳� Quad
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
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init�� Copy��)
	HRESULT Free() override;

private:
	vector<VertexPositionNormalTexture> m_vecVertexData;
	vector<UINT32> m_vecIndexData; // UINT16 ���� �޽��� �ڽĿ� ���� ����
};

_NAMESPACE