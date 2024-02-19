#pragma once
#include "Base.h"
#define VK_MAX 0xff

NAMESPACE_(Engine)

enum KEY_STATE {KEY_NONE, KEY_DOWN, KEY_UP, KEY_PRESSING, KEY_STATE_END };

// Keyboard and Mouse Input
class ENGINE_DLL CInputManager : public CBase
{
	DECLARE_SINGLETON(CInputManager)

protected:
	CInputManager();
	~CInputManager() override = default;
	
public:
	void Tick(HWND* pHwnd);
	HRESULT Free() override;
public: // Mouse
	Vector2 Get_MouseDirPrevToCur();
public: // Get Key State
	bool Key_Pressing(int inKey);
	bool Key_Down(int inKey);
	bool Key_Up(int inKey);
public:
private:
	// first는 단순 OX second는 up down pressing
	std::pair<bool, KEY_STATE> m_keyState[VK_MAX];
	POINT m_curMousePos{};
	POINT m_prevMousePos{};
};

_NAMESPACE