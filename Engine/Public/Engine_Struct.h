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
		// TODO ..�߰� ����
	};

	struct SHADER_INIT_DESC
	{
		std::wstring& filename; // ���
		D3D_SHADER_MACRO* defines; // ��޿ɼ�, �Ϲ������� nullptr
		std::string& entrypoint; // ���̴� ���α׷� ������ �Լ� �̸�, �ϳ��� hlsl���� ���� �Լ�(���̴�) ����
		std::string& target; // ���̴� ������ ����
	};

	struct VERTEX_INPUT_ELEMENT_DESC_BUILT_IN
	{
		VERTEX_INPUT_ELEMENT_DESC_BUILT_IN()
		{
			Default_Position = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			Default_Color = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		}
		D3D12_INPUT_ELEMENT_DESC Default_Position;
		D3D12_INPUT_ELEMENT_DESC Default_Color;
	};

}
