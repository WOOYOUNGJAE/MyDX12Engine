#pragma once
#include "Base.h"
class CRenderer : public CBase
{
	DECLARE_SINGLETON(CRenderer)
private:
	CRenderer() = default;
	~CRenderer() override = default;

public:
	HRESULT Free() override;

private: // typedef
	typedef ComPtr<ID3D12PipelineState> PSO;
	enum ENUM_PSO { PSO_DEFAULT, ENUM_PSO_END };
private:
	PSO m_PSOArr[ENUM_PSO_END];

};

