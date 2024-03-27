#pragma once

#include "SceneNode.h"
class CGameObject;
NAMESPACE_(Engine)
// �ַ� BVH �뵵�� ���� AABB ���
// DXR������ BLAS�� ���ؼ��� Ȱ��
class CSceneNode_AABB : public CSceneNode
{
protected:
	CSceneNode_AABB() = default;
	~CSceneNode_AABB() override = default;

public:
	static CSceneNode_AABB* Create();
	HRESULT Free() override;
public:
	DXR::BLAS* Get_BLAS() { return m_pContainingBLAS; }

private:
	Vector3 mBoxMin = Vector3::Zero;
	Vector3 mBoxMax = Vector3::Zero;
#if DXR_ON
	DXR::BLAS* m_pContainingBLAS = nullptr;
#endif

};

_NAMESPACE