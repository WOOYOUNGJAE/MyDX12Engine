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
		Matrix InvTranspose;
	};

	struct SCENE_CB
	{
		Matrix viewProjectionInv;
		Vector4 cameraPosition;
		Vector4 lightPosition;
		Vector4 lightAmbientColor;
		Vector4 lightDiffuseColor;
	};

	struct MATERIAL_INFO
	{
		Vector4 albedo = Vector4::Zero; // 반사율

		FLOAT reflectanceCoef = 0.f;
		FLOAT diffuseCoef = 0.9f;
		FLOAT specularCoef = 0.7f;
		FLOAT specularPower = 50.f;

		FLOAT stepScale = 1.f;
	};
	struct OBJECT_CB_STATIC
	{
		OBJECT_CB_STATIC() = default;
		OBJECT_CB_STATIC(MATERIAL_INFO& rhsMaterial)
		{
			albedo = rhsMaterial.albedo;
			reflectanceCoef = rhsMaterial.reflectanceCoef;
			diffuseCoef = rhsMaterial.diffuseCoef;
			specularCoef = rhsMaterial.specularCoef;
			specularPower = rhsMaterial.specularPower;
			stepScale = rhsMaterial.stepScale;
			startIndex_in_IB_SRV = 0;
			startIndex_in_VB_SRV = 0;
			padding = {};
		}
		~OBJECT_CB_STATIC() = default;
		Vector4 albedo;

		UINT startIndex_in_IB_SRV; // Index Buffer ShaderResourceView의 시작 인덱스
		UINT startIndex_in_VB_SRV;
		FLOAT reflectanceCoef;
		FLOAT diffuseCoef;

		FLOAT specularCoef;
		FLOAT specularPower;
		FLOAT stepScale;                      // Step scale for ray marching of signed distance primitives. 
		FLOAT padding;
	};
	//struct OBJECT_CB_STATIC
	//{
	//	OBJECT_CB_STATIC(MATERIAL_INFO& rhs)
	//	{
	//		albedo = rhs.albedo;
	//		reflectanceCoef = rhs.reflectanceCoef;
	//		diffuseCoef = rhs.diffuseCoef;
	//		specularCoef = rhs.specularCoef;
	//		specularPower = rhs.specularPower;
	//		stepScale = rhs.stepScale;
	//		startIndex_in_IB_SRV = 0;
	//		startIndex_in_VB_SRV = 0;
	//	}
	//	OBJECT_CB_STATIC() = default;
	//	~OBJECT_CB_STATIC() = default;
	//	Vector4 albedo;
	//	UINT startIndex_in_IB_SRV; // Index Buffer ShaderResourceView의 시작 인덱스
	//	UINT startIndex_in_VB_SRV;

	//	FLOAT reflectanceCoef;
	//	FLOAT diffuseCoef;
	//	FLOAT specularCoef;
	//	FLOAT specularPower;

	//	FLOAT stepScale;                      // Step scale for ray marching of signed distance primitives. 
	//	// - Some object transformations don't preserve the distances and 
	//	//   thus require shorter steps.
	//	Vector3 padding;
	//};
_NAMESPACE
#endif

}
