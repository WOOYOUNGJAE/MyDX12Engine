#pragma once

namespace Engine
{
	struct GRAPHIC_DESC
	{
		enum WINMODE { WINMODE_FULL, WINMODE_WIN, WINMODE_END };
		HWND			hWnd;
		unsigned int	iSizeX, iSizeY;
		WINMODE			eWinMode;
	};

	struct MY_VERTEX
	{
		enum VERTEX_ELEM
		{
			POS,
			COLOR,
			NORMAL,
		};
		_float3 vPos;
		_float4 vColor;
		// TODO ..추가 예정
	};
}
