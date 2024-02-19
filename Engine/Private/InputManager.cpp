#include "InputManager.h"

IMPLEMENT_SINGLETON(CInputManager)

CInputManager::CInputManager() 
{
	ZeroMemory(m_keyState, sizeof(m_keyState));
}

void CInputManager::Tick(HWND* pHwnd)
{
	for (INT keyChecking = 0; keyChecking < VK_MAX; ++keyChecking)
	{

		if (GetAsyncKeyState(keyChecking) & 0x8000) // Ű�� ������
		{
			if (m_keyState[keyChecking].first) // ������ ���ȴٸ�
			{
				m_keyState[keyChecking].second = KEY_PRESSING; // ������ �ִ� ����
			}
			else
			{
				m_keyState[keyChecking].first = true;
				m_keyState[keyChecking].second = KEY_DOWN;		// �� ���� ����
			}
		}
		else // ������ �ʾҴٸ�
		{
			if (m_keyState[keyChecking].first) // ������ ���ȴٸ�
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

	//Mouse
	m_prevMousePos = m_curMousePos;
	GetCursorPos(&m_curMousePos);
	ScreenToClient(*pHwnd, &m_curMousePos);
#ifdef _DEBUG
	 //std::cout << "x : " << m_curMousePos.x << " y : " << m_curMousePos.y << std::endl;
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

Vector2 CInputManager::Get_MouseDirPrevToCur()
{
	return Vector2((m_curMousePos.x - m_prevMousePos.x), (m_curMousePos.y - m_prevMousePos.y));
}

bool CInputManager::Key_Down(int inKey)
{
	return m_keyState[inKey].second == KEY_DOWN;
}

bool CInputManager::Key_Up(int inKey)
{
	return m_keyState[inKey].second == KEY_UP;
}
