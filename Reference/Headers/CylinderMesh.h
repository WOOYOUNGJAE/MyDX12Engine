#pragma once
#include "MeshData.h"

NAMESPACE_(Engine)

class ENGINE_DLL CCylinderMesh : public CMeshData
{
private:
	CCylinderMesh();
	CCylinderMesh(CCylinderMesh& rhs);
	~CCylinderMesh() override = default;

public:
	static CCylinderMesh* Create(UINT iNumSlices, FLOAT fTopRadius = 0.5f, FLOAT fBottomRadius = 0.5f);
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;

private:
	UINT m_iNumSlices = 1;
	FLOAT m_fTopRadius = 0.5f;
	FLOAT m_fBottomRadius = 0.5f;
};

_NAMESPACE