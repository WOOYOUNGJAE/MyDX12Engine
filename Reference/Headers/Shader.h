#pragma once
#include "Component.h"

NAMESPACE_(Engine)
using namespace std;
class ENGINE_DLL CShader : public CComponent
{
public: // typedef
	enum SHADER_TYPE {TYPE_VERTEX, TYPE_PIXEL, SHADER_TYPE_END,};
private:
	CShader();
	CShader(const CShader& rhs);
	~CShader() override = default;

public:
	static CShader* Create(const SHADER_INIT_DESC& shaderInput);
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype(const SHADER_INIT_DESC& shaderInput);
	HRESULT Free() override;
public: // getter setter
	ComPtr<ID3DBlob> Get_ByteCode(SHADER_TYPE eShaderType) { return m_shaderByteCodeArr[eShaderType].Get(); }
	/*void Push_InputLayout(D3D12_INPUT_ELEMENT_DESC desc)
	{
		m_vecInputLayout.push_back(desc);
	}*/
private:
	SHADER_TYPE m_eShaderType = SHADER_TYPE_END;
private:
	// ID3DBlob : 범용 메모리 버퍼, 
	ComPtr<ID3DBlob> m_shaderByteCodeArr[SHADER_TYPE_END];
	//vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;
};

_NAMESPACE