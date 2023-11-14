#pragma once
#include "MeshGeometry.h"
class CCubeMesh final: public CMeshGeometry
{
private:
	CCubeMesh() = default;
	~CCubeMesh() override = default;

public:
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize() override;
	HRESULT Free() override;

private:
};

