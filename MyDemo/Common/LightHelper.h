//***************************************************************************************
// LightHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper classes for lighting.
//***************************************************************************************

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;

// Note: Make sure structure alignment agrees with HLSL structure padding rules. 
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient; //环境光
	XMFLOAT4 Diffuse; //漫反射光
	XMFLOAT4 Specular; //高光
	XMFLOAT3 Direction; //方向
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient; //环境光
	XMFLOAT4 Diffuse; //漫反射光
	XMFLOAT4 Specular; //高光

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position; //位置
	float Range; //范围

	// Packed into 4D vector: (A0, A1, A2, Pad)
	XMFLOAT3 Att; //衰减系数
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient; //环境光
	XMFLOAT4 Diffuse; //漫反射光
	XMFLOAT4 Specular; //高光

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position; //位置
	float Range; //范围

	// Packed into 4D vector: (Direction, Spot)
	XMFLOAT3 Direction; //方向
	float Spot; //强度系数

	// Packed into 4D vector: (Att, Pad)
	XMFLOAT3 Att; //衰减系数
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient; //环境光
	XMFLOAT4 Diffuse; //漫反射光
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect; //反射光
};

#endif // LIGHTHELPER_H