#pragma once
#include <inttypes.h>

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
	static CShader* Create(const SHADER_INIT_DESC* shaderInputArr, _uint iArrSize = 1);
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype(const SHADER_INIT_DESC* shaderInputArr, _uint iArrSize = 1);
	HRESULT Free() override;
public: // getter setter
	ComPtr<ID3DBlob> Get_ByteCode(SHADER_TYPE eShaderType) { return m_shaderByteCodeArr[eShaderType].Get(); }
	int Get_NumDirty() { return m_iNumFramesDirty; }
	int& Get_NumDirtyRef() { return m_iNumFramesDirty; }
	/*void Push_InputLayout(D3D12_INPUT_ELEMENT_DESC desc)
	{
		m_vecInputLayout.push_back(desc);
	}*/
public:
	void Bind_Matrix(const string& strConstantName, const _float4x4& matrix);
	//void Bind_Resource();
private:
	SHADER_TYPE m_eShaderType = SHADER_TYPE_END;
private: // 바이너리 데이터
	// ID3DBlob : 범용 메모리 버퍼, 
	ComPtr<ID3DBlob> m_shaderByteCodeArr[SHADER_TYPE_END]{};
	//vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;
private:
	int m_iNumFramesDirty = 0; // = g_numFrameResources
private:
	D3D12_INPUT_ELEMENT_DESC* input_layout_descArr = nullptr;
	ID3D12RootSignature* m_pRootSig = nullptr;
	ID3D12PipelineState* m_pPSO = nullptr;
};

_NAMESPACE