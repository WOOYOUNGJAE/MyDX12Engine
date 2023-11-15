#pragma once
#include "MeshGeometry.h"

NAMESPACE_(Engine)

class ENGINE_DLL CCubeMesh final: public CMeshGeometry
{
private:
	CCubeMesh() = default;
	~CCubeMesh() override = default;

public:
	static CCubeMesh* Create();
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize() override;
	HRESULT Free() override;

private:
};

_NAMESPACE