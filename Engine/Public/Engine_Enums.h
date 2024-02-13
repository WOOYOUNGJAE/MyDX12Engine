#pragma once

namespace Engine
{
	namespace Pipeline
	{
		enum ENUM_PSO { PSO_DEFAULT, PSO_END };
	}

	//PSO TYPES
	enum RENDER_PRIORITY {RENDER_FIRST, RENDER_AFTER, RENDER_PRIORITY_END};
	enum RENDER_BLENDMODE { NOBLEND, BLEND, RENDER_BLENDMODE_END};
	enum RENDER_SHADERTYPE
	{						// InputLayout 
		SHADERTYPE_SIMPLE,	// Pos, Color
		SHADERTYPE_SIMPLE2, // Pos, TexCoord
		RENDER_SHADERTYPE_END
	};
	enum ROOTSIG_TYPE { ROOTSIG_DEFAULT, /*ROOTSIG_TEX, ROOTSIG_TEX_TRANSMAT,*/ ROOTSIG_TYPE_END};
	//enum RENDER_PARAMCOMBO {PARAM_SIMPLE, PARAM_SAMPLER, RENDER_PARAMCOMBO_END};

	// RootSignature DescriptorTable
	enum TABLE_TYPE {TEX, TEX_CB, TABLE_TYPE_END};

	enum MANAGED_RESOURCE_TYPE : unsigned long long
	{ MANAGED_RESOURCE_TEX, MANAGED_RESOURCE_OBJ_CONS, MANAGED_RESOURCE_TYPE_END};



	// xtk12 vertex type ���
	//enum VERTEX_TYPE { VERTEX_POSITION, VERTEX_POSITION_COLOR, VERTEX_POSITION_TEXTURE, VERTEX_TYPE_END};
}