#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)
// Create에 어떠한 인자를 넣지 않으면 그냥 Quad
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
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init은 Copy용)
	HRESULT Free() override;

private:
	UINT m_iNumSlices = 1;
	UINT m_iNumStacks = 1;
};

_NAMESPACE