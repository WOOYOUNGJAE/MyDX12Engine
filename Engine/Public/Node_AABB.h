#pragma once

#include "Base.h"

NAMESPACE_(Engine)
// BVH의 노드가 될 수도, SceneGraph의 노드가 될 수도 있음
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