#include "Shader.h"
#include "Device_Utils.h"
#include "PipelineManager.h"

CShader::CShader()
{
}

CShader::CShader(CShader& rhs) : CComponent(rhs), m_iInputLayoutSize(rhs.m_iInputLayoutSize),
m_pInputLayoutArr(rhs.m_pInputLayoutArr)
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

	switch (shaderInputArr->inputLayout)
	{
	case SHADER_INIT_DESC::POS_COLOR:
	{
		m_pInputLayoutArr = new D3D12_INPUT_ELEMENT_DESC[2]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		m_iInputLayoutSize = 2;
		break;
	}
	case SHADER_INIT_DESC::POS_TEXCOORD:
	{
		m_pInputLayoutArr = new D3D12_INPUT_ELEMENT_DESC[2]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		m_iInputLayoutSize = 2;
		break;
	}
	case SHADER_INIT_DESC::POS_NORMAL_TEXCOORD:
	{
		m_pInputLayoutArr = new D3D12_INPUT_ELEMENT_DESC[3]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		m_iInputLayoutSize = 3;
		break;
	}
	default:
		break;
	}

	return S_OK;
}

HRESULT CShader::Free()
{
	if (m_iClonedNum == 0)
	{
		Safe_Delete_Array(m_pInputLayoutArr);		
	}

	return CComponent::Free();
}