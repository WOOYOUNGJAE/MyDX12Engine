#pragma once

#include "Base.h"

NAMESPACE_(Engine)
// BVH�� ��尡 �� ����, SceneGraph�� ��尡 �� ���� ����
class CNode_AABB : public CBase
{
protected:
	CNode_AABB() = default;
	~CNode_AABB() override = default;

public:
	HRESULT Free() override;

#if DXR_ON

#endif

};

_NAMESPACE