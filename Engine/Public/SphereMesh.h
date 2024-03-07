#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)

class ENGINE_DLL CSphereMesh : public CMeshData
{
private:
	CSphereMesh();
	CSphereMesh(CSphereMesh& rhs);
	~CSphereMesh() override = default;

public:
	static CSphereMesh* Create(UINT iNumSlices, UINT iNumStacks);
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;

private:
	UINT m_iNumSlices = 1;
	UINT m_iNumStacks = 1;
};

_NAMESPACE