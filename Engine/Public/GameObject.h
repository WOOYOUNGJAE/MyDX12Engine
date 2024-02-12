#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
	CGameObject() = default;
	~CGameObject() override = default;

public: // LifeCycle
	virtual HRESULT Initialize_Prototype() = 0;
	virtual HRESULT Initialize(void* pArg = nullptr) = 0;
	virtual CGameObject* Clone(void* pArg = nullptr) = 0;
	virtual void Tick(_float fDeltaTime) = 0;
	virtual void Late_Tick(_float fDeltaTime) = 0;
	virtual void Render_Tick(){};
	HRESULT Free() override;
public: // getter setter, abstract
	virtual int& Get_NumFrameDirtyRef() { int a = -1; return a; };
	virtual _matrix Get_WorldMatrix() { return _matrix(); };
	virtual _float3 Get_Pos() { return _float3(); }
	virtual D3D12_VERTEX_BUFFER_VIEW VertexBufferView() { return D3D12_VERTEX_BUFFER_VIEW(); }
	virtual D3D12_INDEX_BUFFER_VIEW IndexBufferView()const {return D3D12_INDEX_BUFFER_VIEW();}
	virtual D3D12_PRIMITIVE_TOPOLOGY PrimitiveType()const {	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;} // Default;
	virtual _uint Num_Indices() { return 0; }
	virtual UINT64 Get_CbvSrvUavHeapOffset_Texture() { return m_iCbvSrvUavOffset; }
public:
	virtual _bool Com_Already_Owned(const wstring& strComTag);
	// 이미 있는지, 컴포넌트 생성, 맵에 넣기, 
	virtual HRESULT Add_Component(const wstring& strComTag, class CComponent** ppOutCom, void* pArg = nullptr);

protected:
	map<wstring, CComponent*> m_mapComponents;
	UINT64 m_iCbvSrvUavOffset = ULONGLONG_MAX; // 일단 오브젝트당 텍스처 1개 가정
};

_NAMESPACE