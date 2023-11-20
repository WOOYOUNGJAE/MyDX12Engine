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

public:
	enum SHADER_TYPE {TYPE_VERTEX, TYPE_PIXEL, SHADER_TYPE_END,
	};
private:
	//wstring m_strShaderName;
	SHADER_TYPE m_eShaderType = SHADER_TYPE_END;
private:
	// ID3DBlob : ���� �޸� ����, 
	ComPtr<ID3DBlob> m_shaderByteCodeArr[SHADER_TYPE_END];
	//ComPtr<ID3DBlob> m_psByteCode = nullptr;
	vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;
};

_NAMESPACE