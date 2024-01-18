#include "Shader.h"
#include "Device_Utils.h"
#include "PipelineManager.h"

CShader::CShader()
{
}

CShader::CShader(const CShader& rhs)
{
	memcpy(m_shaderByteCodeArr, rhs.m_shaderByteCodeArr, sizeof(ComPtr<ID3DBlob>) * SHADER_TYPE_END);
}

CShader* CShader::Create(const SHADER_INIT_DESC* shaderInputArr, _uint iArrSize)
{
	CShader* pInstance = new CShader();

	if (pInstance == nullptr)
	{
		MSG_BOX("Shader : Fail to Create Shader");
		Safe_Release(pInstance);
	}

	if (FAILED(pInstance->Initialize_Prototype(shaderInputArr, iArrSize)))
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
	pInstance->m_bIsCloned = true;
	return pInstance;
}

HRESULT CShader::Initialize_Prototype(const SHADER_INIT_DESC* shaderInputArr, _uint iArrSize)
{
	if (FAILED(CComponent::Initialize_Prototype()))
	{
		return E_FAIL;
	}

	for (int i = 0; i < iArrSize; ++i)
	{
		SHADER_TYPE eCurType = SHADER_TYPE_END;
		if (shaderInputArr[i].entrypoint == "VS")
		{
			eCurType = TYPE_VERTEX;
		}
		else if (shaderInputArr[i].entrypoint == "PS")
		{
			eCurType = TYPE_PIXEL;
		}
		else
		{
			int a = 1;
		}
		//else if () {}

		m_shaderByteCodeArr[eCurType] = CDevice_Utils::CompileShader(shaderInputArr[i].filename, shaderInputArr[i].defines, shaderInputArr[i].entrypoint, shaderInputArr[i].target);
	}

	return S_OK;
}

HRESULT CShader::Free()
{
	return CComponent::Free();
}

void CShader::Bind_Matrix(const string& strConstantName, const _float4x4& matrix)
{
	
}
