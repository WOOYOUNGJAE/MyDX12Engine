#pragma once
#include "Component.h"

NAMESPACE_(Engine)
using namespace std;
class ENGINE_DLL CShader : public CComponent
{
private:
	CShader();
	CShader(const CShader& rhs);
	~CShader() override = default;

public:
	static CShader* Create(const SHADER_INIT_DESC& shaderInput);
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype(const SHADER_INIT_DESC& shaderInput);
	HRESULT Free() override;

private:
	//wstring m_strShaderName;
private:
	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;
};

_NAMESPACE