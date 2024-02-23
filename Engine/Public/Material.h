#pragma once
#include "Component.h"

NAMESPACE_(Engine)
// 컴포넌트 구조 유지 위해 material 정보를 갖고 있기만 하는 컴포넌트, 
class CMaterial :
    public CComponent
{
protected:
    CMaterial() = default;
    CMaterial(CMaterial& rhs) : CComponent(rhs) {}
    ~CMaterial() override = default;

public:
    static CMaterial* Create();
    CComponent* Clone(void* pArg) override;
    HRESULT Initialize(void* pArg) override;
    HRESULT Free() override;
public:
    MATERIAL_INFO Get_MaterialInfo() { return m_materialInfo; }

private:
    MATERIAL_INFO m_materialInfo{ Vector3::Zero, 0.5f, Vector3::One * 0.5f, 0.f, Vector3::One * 0.5f};
};

_NAMESPACE