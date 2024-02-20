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
	CShader(CShader& rhs);
	~CShader() override = default;

public:
	static CShader* Create(const SHADER_INIT_DESC* shaderInputArr, _uint iArrSize = 1);
	CComponent* Clone(void* pArg) override;
	HRESULT Initialize_Prototype(const SHADER_INIT_DESC* shaderInputArr, _uint iArrSize = 1);
	HRESULT Free() override;
public: // getter setter
	ComPtr<ID3DBlob> Get_ByteCode(SHADER_TYPE eShaderType) { return m_shaderByteCodeArr[eShaderType].Get(); }
	UINT Get_InputLayoutSize() const { return m_iInputLayoutSize; }
	D3D12_INPUT_ELEMENT_DESC** Get_InputLayoutArr()  { return &m_pInputLayoutArr; }
	int Get_NumDirty() { return m_iNumFramesDirty; }
	int& Get_NumDirtyRef() { return m_iNumFramesDirty; }

private:
	ComPtr<ID3DBlob> m_shaderByteCodeArr[SHADER_TYPE_END]{};  // 바이너리 데이터
	D3D12_INPUT_ELEMENT_DESC* m_pInputLayoutArr = nullptr;
	UINT m_iInputLayoutSize = 0;
private:
	int m_iNumFramesDirty = 0; // = g_numFrameResources
};

_NAMESPACE