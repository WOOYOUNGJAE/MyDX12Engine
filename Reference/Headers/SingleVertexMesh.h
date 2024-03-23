#pragma once
#include "MeshData.h"
#include "MeshDataType.h"
NAMESPACE_(Engine)
// ��, �ε��� ��� 1��, G-Shader���� ���ؽ� ����
// ������ Tree � �̿�
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
	HRESULT Initialize(void* pArg) override; // Do Nothing (Init�� Copy��)
	HRESULT Free() override;

public:
	DXGI_FORMAT Get_IndexFormat() override;
	size_t Get_SingleVertexSize() override { return sizeof(VertexPositionSize); };
};

_NAMESPACE