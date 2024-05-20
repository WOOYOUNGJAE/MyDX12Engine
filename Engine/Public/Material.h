#pragma once
#include "Component.h"

NAMESPACE_(Engine)
// ������Ʈ ���� ���� ���� material ������ ���� �ֱ⸸ �ϴ� ������Ʈ, 
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

#if DXR_ON
public:
    DXR::OBJECT_CB_STATIC Get_DXR_MaterialInfo() { return m_DXR_materialInfo; }
private:
    // BLAS �ϳ��� 1�� �Ҵ�
    DXR::OBJECT_CB_STATIC m_DXR_materialInfo{ Vector4::One* 0.7f };
#endif
};

_NAMESPACE