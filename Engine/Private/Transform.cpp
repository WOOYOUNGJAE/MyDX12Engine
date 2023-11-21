#include "Transform.h"

void CTransform::Set_WorldMatrix(MATRIX_ENUM eEnum, _fvector vVec)
{
	_matrix		WorldMatrix = XMLoadFloat4x4(&m_WorldMatrix);

	WorldMatrix.r[eEnum] = vVec;

	XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
}

void CTransform::Refresh_Transform(TRANSFORM_ENUM eEnum)
{
	switch (eEnum)
	{
	case LOOK:
		{
			_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMLoadFloat3(&m_Look));
		}
		break;
	case POSITION:
		break;
	case SCALE:
		break;
	default:
		break;
	}
}
