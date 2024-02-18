#include "InputManager.h"

IMPLEMENT_SINGLETON(CInputManager)

void CInputManager::Tick(HWND* pHwnd)
{
	for (INT keyChecking = 0; keyChecking < VK_MAX; ++keyChecking)
	{

		if (GetAsyncKeyState(keyChecking) & 0x8000) // 키가 눌리면
		{
			if (m_keyState[keyChecking].first) // 이전에 눌렸다면
			{
				m_keyState[keyChecking].second = KEY_PRESSING; // 누르고 있는 상태
			}
			else
			{
				m_keyState[keyChecking].first = true;
				m_keyState[keyChecking].second = KEY_DOWN;		// 막 누른 상태
			}
		}
		else // 눌리지 않았다면
		{
			if (m_keyState[keyChecking].first) // 이전에 눌렸다면
			{
				m_keyState[keyChecking].first = false;
				m_keyState[keyChecking].second = KEY_UP;
			}
			else
			{
				m_keyState[keyChecking].second = KEY_NONE;
			}
		}
	}

	if (m_keyState[VK_RBUTTON].first == true)
	{
		int a = 1;
	}

	//Mouse
	m_prevMousePos = m_curMousePos;
	GetCursorPos(&m_curMousePos);
	ScreenToClient(*pHwnd, &m_curMousePos);
#ifdef DEBUG
	// std::cout << "x : " << ptMouse.x << " y : " << ptMouse.y << std::endl;
#endif // DEBUG
}

HRESULT CInputManager::Free()
{
	return S_OK;
}

bool CInputManager::Key_Pressing(int inKey)
{
	return m_keyState[inKey].second == KEY_PRESSING;
}

Vector2 CInputManager::Get_MousePosPrevToCur()
{
	return Vector2((m_curMousePos.x - m_prevMousePos.x), (m_curMousePos.x - m_prevMousePos.x));
}

bool CInputManager::Key_Down(int inKey)
{
	return m_keyState[inKey].second == KEY_DOWN;
}

bool CInputManager::Key_Up(int inKey)
{
	return m_keyState[inKey].second == KEY_UP;
}
