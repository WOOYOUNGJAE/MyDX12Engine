#pragma once
#include "MeshGeometry.h"

NAMESPACE_(Engine)

class CTriangleMesh : public CMeshGeometry
{

private:
	CTriangleMesh();
	CTriangleMesh(const CTriangleMesh& rhs);
	~CTriangleMesh() override = default;

public:
	static CTriangleMesh* Create();
	CComponent* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;
};

_NAMESPACE