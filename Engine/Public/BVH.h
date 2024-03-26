#pragma once
#include "Base.h"

NAMESPACE_(Engine)
class CNode_AABB;
class CBVH : public CBase
{
protected:
	CBVH() = default;
	~CBVH() override = default;

public:
	HRESULT Free() override;

private:
	CNode_AABB* m_pRoot = nullptr;

};

_NAMESPACE