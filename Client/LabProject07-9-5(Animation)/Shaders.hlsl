struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8);
};


#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}
	float4 cIllumination = Lighting(input.positionW, normalW);
	return(lerp(cColor, cIllumination, 0.5f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128			//45 * 3 ����  // ��ĵ��� �迭 

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	uint4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = float3(0.0f, 0.0f, 0.0f);
	output.normalW = float3(0.0f, 0.0f, 0.0f);
	output.tangentW = float3(0.0f, 0.0f, 0.0f);
	output.bitangentW = float3(0.0f, 0.0f, 0.0f);
	matrix mtxVertexToBoneWorld;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
		output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
		output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
		output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3)mtxVertexToBoneWorld);
	}

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
//	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////


//struct VS_TEXTURED_INPUT
//{
//	float3 position : POSITION;
//	float2 uv : TEXCOORD;
//};
//
//struct VS_TEXTURED_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float2 uv : TEXCOORD;
//};
//
//VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
//{
//	VS_TEXTURED_OUTPUT output;
//
//	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
//	output.uv = input.uv;
//
//
//	return(output);
//}
//
//Texture2D gtxtSnowTexture : register(t13);
//float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
//{
//	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
//
//	if (cColor.a < 0.3f)
//	{
//		discard;
//	}
//	return(cColor);
//}

Texture2D gtxtSnowBillboard : register (t14);

struct VS_IN
{
	float3 posW : POSITION;
	float2 sizeW : SIZE;
};

struct VS_OUT
{
	float3 centerW : POSITION;
	float2 sizeW : SIZE;
};

struct GS_OUT {
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
	uint primID : SV_PrimitiveID;
};


VS_OUT VSBillboard(VS_IN input, uint nInstanceID :SV_InstanceID)
{
	VS_OUT output;

	//float4 v = mul(float4(input.posW, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject)


	//output.centerW = mul(input.posW, gmtxGameObject);
	//input.posW = mul(input.posW, gmtxBillboardWorld);
	output.centerW = input.posW;
	output.sizeW = input.sizeW;

	return (output);
}

[maxvertexcount(4)]
void GSBillboard(point VS_OUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_OUT> outstream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = gvCameraPosition.xyz - input[0].centerW;
	vLook = normalize(vLook);

	float3 vRight = cross(vUp, vLook);

	float fHalfW = input[0].sizeW.x * 0.5f;
	float fHalfH = input[0].sizeW.y * 0.5f;

	float4 pVertices[4];

	pVertices[0] = float4(input[0].centerW + fHalfW * vRight - fHalfH * vUp, 1.0f);
	pVertices[1] = float4(input[0].centerW + fHalfW * vRight + fHalfH * vUp, 1.0f);
	pVertices[2] = float4(input[0].centerW - fHalfW * vRight - fHalfH * vUp, 1.0f);
	pVertices[3] = float4(input[0].centerW - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 pUVs[4] = { float2(0.0f,1.0f),float2(0.0f,0.0f),float2(1.0f,1.0f),float2(1.0f,0.0f) };
	float4 pColors[4] = { float4(1.0f,0.0f,0.0f,1.0f),float4(0.0f,1.0f,0.0f,1.0f),float4(0.0f,0.0f,1.0f,1.0f),float4(1.0f,1.0f,1.0f,1.0f) };

	GS_OUT output;
	//[unroll]
	for (int i = 0; i < 4; i++)
	{
		output.posW = pVertices[i].xyz;
		output.posH = mul(mul(pVertices[i], gmtxView), gmtxProjection);
		output.normalW = vLook;
		output.tangentW = vRight;		//�� ���������� ���� ���庯ȯ
		output.uv = pUVs[i];
		output.color = pColors[i];
		output.primID = primID;
		outstream.Append(output);
	}
}
float4 PSBillboard(GS_OUT input) :SV_Target
{
	float3 uvw = float3(input.uv,(input.primID % 3));
	//float3 uvwNormal = float3(input.uv, (input.primID % 3));


	float4 cColor = gtxtSnowBillboard.Sample(gssWrap, uvw);

	float3 cNormal = (float3)1;						//������� �׻� �÷��̾�� �����̹Ƿ�

	float3 vNormal = 2.0f * cNormal - 1.0f;			//[0,1] -> [-1,1]

	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	float3 normalW = mul(vNormal, TBN);

	float4 cIllumination = Lighting(input.posW, normalW);

	return(lerp(cColor,cIllumination,0.3f));
}

