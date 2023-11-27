#pragma once
#include "GameObject.h"

NAMESPACE_(Engine)

class ENGINE_DLL CCube : public CGameObject
{
protected:
	CCube() = default;
	CCube(const CCube& rhs): CGameObject(rhs){}
	~CCube() override = default;

public:
	static CCube* Create();
	CGameObject* Clone(void* pArg) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Tick(_float fDeltaTime) override;
	void Late_Tick(_float fDeltaTime) override;
	void Pipeline_Tick() override;
	void Render() override;
	HRESULT Free() override;
public:
	int& Get_NumFrameDirtyRef() override;
	_matrix Get_WorldMatrix() override;
	_float3 Get_Pos() override;
protected:
	class CTransform* m_pTransformCom = nullptr;
	class CCubeMesh* m_pCubeMeshCom = nullptr;
	class CRenderer* m_pRendererCom = nullptr;
	class CShader* m_pShaderCom = nullptr;
};

_NAMESPACE