//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//***************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include "d3dUtil.h"

class Camera
{
public:
	Camera();
	~Camera();

	// 获取或设置摄像机位置
	XMVECTOR GetPositionXM()const;
	XMFLOAT3 GetPosition()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);
	
	// 获取摄像机的基本向量。
	XMVECTOR GetRightXM()const;
	XMFLOAT3 GetRight()const;
	XMVECTOR GetUpXM()const;
	XMFLOAT3 GetUp()const;
	XMVECTOR GetLookXM()const;
	XMFLOAT3 GetLook()const;

	// 得到平截体的属性。
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// 在视图空间坐标中获取近平面和远平面尺寸。
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;
	
	// 设置镜头
	void SetLens(float fovY, float aspect, float zn, float zf);

	// 通过LookAt参数定义摄像机空间。
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	// 获取 View/Proj 矩阵.
	XMMATRIX View()const;
	XMMATRIX Proj()const;
	XMMATRIX ViewProj()const;

	// 左右平移
	void Strafe(float d);
	// 上下平移
	void fly(float d);
	// 前后行走
	void Walk(float d);

	// 上下点头
	void Pitch(float angle);
	// 左右摇头
	void RotateY(float angle);

	// 修改摄像机位置/方向后，调用重建视图矩阵。
	void UpdateViewMatrix();

private:

	// 相对于世界空间坐标系的相机坐标系。
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;

	// 保存平截面属性
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	// 保存 View/Proj 矩阵.
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
};

#endif // CAMERA_H