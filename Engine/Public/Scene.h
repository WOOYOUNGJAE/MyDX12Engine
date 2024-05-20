#pragma once
#include "Base.h"
NAMESPACE_(Engine)
// SceneGraph와 씬 오브젝트들 참조만, 직접 메모리 관리는 하지 않음
class CScene : public CBase
{
protected:
	CScene() = default;
	~CScene() override = default;

public:

private:

};

_NAMESPACE