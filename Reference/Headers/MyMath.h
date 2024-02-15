#pragma once
#include "Base.h"

XMFLOAT3 inline Add_XMFLOAT3(XMFLOAT3& f3a, XMFLOAT3& f3b)
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVectorAdd(XMLoadFloat3(&f3a), XMLoadFloat3(&f3b)));
	return result;
}