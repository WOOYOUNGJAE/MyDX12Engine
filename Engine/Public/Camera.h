#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)
// Client에서 상속받아서 구현
class ENGINE_DLL CCamera abstract: public CGameObject
{
protected:
	CCamera();
	CCamera(CCamera& rhs);
	~CCamera() override = default;

public: // typedef
	struct CAMERA_DESC
	{
		_float m_fFovy = 0.f;
		_float m_fAspect = 0.f;
		_float m_fNear = 0.f;
		_float m_fFar = 1.f;
	};
public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	HRESULT Free() override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
public:
	virtual void Update_PipelineView();
	_float3 Get_Pos() override;
protected: // pointer
	class CPipelineManager* m_pPipelineManager = nullptr;
	class CTransform* m_pTransformCom = nullptr;
protected:
	CAMERA_DESC cam_desc = { 0.f, 0.f, 0.f, 1.f };

	_float m_fFovy = XMConvertToRadians(60.f);
	_float m_fAspect = (_float)1280 / 720; // TODO TEMP
	_float m_fNear = 0.2f;
	_float m_fFar = 300.f;
};

_NAMESPACE