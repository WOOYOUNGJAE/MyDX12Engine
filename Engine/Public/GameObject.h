#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
// 텍스쳐(SrvOffset)는 텍스쳐컴포넌트로 들고 있을 수도, 메쉬 컴포넌트가 들고 있을 수도 있음
class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
	CGameObject() = default;
	CGameObject(CGameObject& rhs) : CBase(rhs), m_strPrototypeTag(rhs.m_strPrototypeTag){};
	~CGameObject() override = default;

public: // LifeCycle
	virtual HRESULT Initialize_Prototype() = 0;
	virtual HRESULT Initialize(void* pArg = nullptr) = 0;
	virtual CGameObject* Clone(void* pArg = nullptr) = 0;
	virtual void Tick(_float fDeltaTime) = 0;
	virtual void Late_Tick(_float fDeltaTime) = 0;
	virtual void Render_Tick(){};
	virtual void Render(ID3D12GraphicsCommandList* pCmdList, struct FrameResource* pFrameResource, UINT iRenderingElementIndex){}
	HRESULT Free() override;
public: // getter setter, abstract
	virtual D3D12_PRIMITIVE_TOPOLOGY PrimitiveType()const {	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;} // Default;
	virtual MATERIAL_INFO Get_MaterialInfo() { return MATERIAL_INFO(); }
	//Offset, Handle
	virtual UINT64 Get_CbvSrvUavHeapOffset_Texture() { return m_iTextureSrvOffset; }
	UINT64* Get_CbvSrvUavHeapOffsetPtr_Texture() { return &m_iTextureSrvOffset; }

	// Tag
	wstring Get_PrototypeTag() { return m_strPrototypeTag; }
	void Set_PrototypeTag(const wstring& strTag) { m_strPrototypeTag = strTag; }
	// Transform
	virtual Matrix Get_WorldMatrix() = 0;// { return XMFLOAT4X4(); };
	virtual Vector3 Get_Pos() { return Vector3(); }
	virtual Vector3 Get_ScaleXYZ() { return Vector3(); }
	virtual void Set_Position(const Vector3& vPos) {};
	virtual void Set_Scale(const Vector3& vScale){}
public:
	virtual _bool Com_Already_Owned(const wstring& strComTag);
	// 이미 있는지, 컴포넌트 생성, 맵에 넣기, 
	virtual HRESULT Add_Component(const wstring& strComTag, class CComponent** ppOutCom, void* pArg = nullptr);

protected:
	wstring m_strPrototypeTag;
	map<wstring, CComponent*> m_mapComponents;
	UINT64 m_iTextureSrvOffset = ULONGLONG_MAX; // 일단 오브젝트당 텍스처 1개 가정
};

_NAMESPACE