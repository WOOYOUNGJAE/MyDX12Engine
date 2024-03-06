#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)
// Create�� ��� ���ڸ� ���� ������ �׳� Quad
class ENGINE_DLL CGridMesh : public CMeshData
{
private:
	CGridMesh();
	CGridMesh(CGridMesh& rhs);
	~CGridMesh() override = default;

public:
	static CGridMesh* Create(UINT iNumSlices = 1, UINT iNumStacks = 1);
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init�� Copy��)
	HRESULT Free() override;

private:
	UINT m_iNumSlices = 1;
	UINT m_iNumStacks = 1;
};

_NAMESPACE