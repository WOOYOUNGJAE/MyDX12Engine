#pragma once
#include "MeshData.h"
#include "MeshDataType.h"
NAMESPACE_(Engine)
// 점, 인덱스 모두 1개, G-Shader에서 버텍스 증식
// 빌보드 Tree 등에 이용
class ENGINE_DLL CSingleVertexMesh : public CMeshData, IMeshDataType<VertexPositionSize, UINT16>
{
private:
	CSingleVertexMesh();
	CSingleVertexMesh(CSingleVertexMesh& rhs);
	~CSingleVertexMesh() override = default;

public:
	static CSingleVertexMesh* Create();
	CMeshData* Clone(void* pArg = nullptr) override;
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init은 Copy용)
	HRESULT Free() override;

public:
	DXGI_FORMAT Get_IndexFormat() override;
	size_t Get_SingleVertexSize() override { return sizeof(VertexPositionSize); };
};

_NAMESPACE