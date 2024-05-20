#pragma once
namespace Engine
{
	struct PASS_CB_VP
	{
		Matrix mViewMat;
		Matrix mProjMat;
		
		FLOAT padding[32];
	};


	struct MATERIAL_INFO
	{
		Vector3 vAmbient;
		FLOAT fShininess;

		Vector3 vDiffuse;
		FLOAT padding0;

		Vector3 vSpecular;
		FLOAT padding1;
	};

	struct LIGHT_INFO
	{
		Vector3 vLightStrength;
		FLOAT fFallOffStart;

		Vector3 vLightDir;
		FLOAT fFallOffEnd;

		Vector3 vLightPos;
		FLOAT fSpotPower;

	};

	struct OBJECT_CB
	{
		Matrix mWorldMat;
		Matrix mInvTranspose;
		MATERIAL_INFO material;
	
		Vector4 padding[5];
	};

	//void foo() { sizeof(*this); }

	struct PASS_CB_VP_LIGHT
	{
		Matrix mViewMat;
		Matrix mProjMat;

		LIGHT_INFO light;

		Vector4 padding[5];
	};



#if DXR_ON
NAMESPACE_(DXR)
	struct OBJECT_CB_DYNAMIC
	{
		Vector4 albedo; // 반사율
	};

	struct SCENE_CB
	{
		Matrix viewProjectionInv;
		Vector4 cameraPosition;
		Vector4 lightPosition;
		Vector4 lightAmbientColor;
		Vector4 lightDiffuseColor;
	};

	struct OBJECT_CB_STATIC
	{
		Vector4 albedo; // 반사율
	};
_NAMESPACE
#endif

}
