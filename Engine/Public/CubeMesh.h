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
	CComponent* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (InitÀº Copy¿ë)
	HRESULT Free() override;

private:
#ifdef _DEBUG
	static _uint m_iClonedNum;
#endif // DEBUG

};

_NAMESPACE