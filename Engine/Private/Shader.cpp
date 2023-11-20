#include "Shader.h"
#include "Device_Utils.h"

CShader::CShader() /*:
	m_pDevice(CGraphic_Device::Get_Instance()->Get_Device()),
	m_pCommandList(CGraphic_Device::Get_Instance()->Get_CommandList())*/
{
}

CShader::CShader(const CShader& rhs)
{
	memcpy(m_shaderByteCodeArr, rhs.m_shaderByteCodeArr, sizeof(ComPtr<ID3DBlob>) * SHADER_TYPE_END);
}

CShader* CShader::Create(const SHADER_INIT_DESC& shaderInput)
{
	CShader* pInstance = new CShader();

	if (pInstance == nullptr)
	{
		MSG_BOX("Shader : Fail to Create Shader");
		Safe_Release(pInstance);
	}

	if (FAILED(pInstance->Initialize_Prototype(shaderInput)))
	{
		MSG_BOX("Shader : Fail to Create Shader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CShader::Clone(void* pArg)
{
	CShader* pInstance = new CShader(*this);

	if (pInstance == nullptr)
	{
		Safe_Release(pInstance);
	}

	return pInstance;
}

HRESULT CShader::Initialize_Prototype(const SHADER_INIT_DESC& shaderInput)
{
	if (FAILED(CComponent::Initialize_Prototype()))
	{
		return E_FAIL;
	}

	if (shaderInput.filename.find(L"vShader"))
	{
		m_eShaderType = TYPE_VERTEX;
	}
	else if (shaderInput.filename.find(L"pShader"))
	{
		m_eShaderType = TYPE_PIXEL;		
	}
	//else if () {}

	m_shaderByteCodeArr[m_eShaderType] = CDevice_Utils::CompileShader(shaderInput.filename, shaderInput.defines, shaderInput.entrypoint, shaderInput.target);

	/*for (auto& iter : m_shaderByteCodeArr)
	{
		if (iter == nullptr)
			continue;
	}*/



	return S_OK;
}

HRESULT CShader::Free()
{
	return CComponent::Free();
}
