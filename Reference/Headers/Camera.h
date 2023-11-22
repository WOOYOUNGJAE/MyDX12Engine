#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)
// Client에서 상속받아서 구현
class ENGINE_DLL CCamera abstract: public CGameObject
{
protected:
	CCamera();
	CCamera(const CCamera& rhs);
	~CCamera() override = default;

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override { return S_OK; }
	HRESULT Free() override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
public:
	virtual void Update_PipelineView();

protected: // pointer
	class CPipeline* m_pPipeline = nullptr;
	class CTransform* m_pTransformCom = nullptr;
protected:
	_float m_fFovy = 0.f;
	_float m_fAspect = 0.f;
	_float m_fNear = 0.f;
	_float m_fFar = 1.f;

};

_NAMESPACE