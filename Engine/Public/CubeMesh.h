#pragma once
#include "MeshGeometry.h"

NAMESPACE_(Engine)

class ENGINE_DLL CCubeMesh final: public CMeshGeometry
{
private:
	CCubeMesh() = default;
	CCubeMesh(const CCubeMesh& rhs) : CMeshGeometry(rhs) {}
	~CCubeMesh() override = default;

public:
	static CCubeMesh* Create();
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init�� Copy��)
	HRESULT Free() override;

private:
};

_NAMESPACE