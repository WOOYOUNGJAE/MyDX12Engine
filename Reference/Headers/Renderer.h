#pragma once
#include "Component.h"
// �������� ���� ����, ���ӿ�����Ʈ�� ������Ʈ�� ������ ���� Ŭ�еǴ� ���� �ƴ� ������

NAMESPACE_(Engine)

class CRenderer final : public CComponent
{
private:
	CRenderer() = default;
	~CRenderer() override = default;

public: // typedef
	enum RENDERGROUP { FIRST, NOLIGHT, NOBLEND, BLEND, UI, RENDERGROUP_END };
public:
	static CRenderer* Create();
	CComponent* Clone(void* pArg) override; // return this
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Render();
	HRESULT Free() override;
public:
	void AddTo_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pGameObject);

private:
	std::list<CGameObject*> m_RenderGroup[RENDERGROUP_END];
};
_NAMESPACE
