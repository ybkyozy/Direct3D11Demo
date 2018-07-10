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

	// ��ȡ�����������λ��
	XMVECTOR GetPositionXM()const;
	XMFLOAT3 GetPosition()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);
	
	// ��ȡ������Ļ���������
	XMVECTOR GetRightXM()const;
	XMFLOAT3 GetRight()const;
	XMVECTOR GetUpXM()const;
	XMFLOAT3 GetUp()const;
	XMVECTOR GetLookXM()const;
	XMFLOAT3 GetLook()const;

	// �õ�ƽ��������ԡ�
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// ����ͼ�ռ������л�ȡ��ƽ���Զƽ��ߴ硣
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;
	
	// ���þ�ͷ
	void SetLens(float fovY, float aspect, float zn, float zf);

	// ͨ��LookAt��������������ռ䡣
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	// ��ȡ View/Proj ����.
	XMMATRIX View()const;
	XMMATRIX Proj()const;
	XMMATRIX ViewProj()const;

	// ����ƽ��
	void Strafe(float d);
	// ����ƽ��
	void fly(float d);
	// ǰ������
	void Walk(float d);

	// ���µ�ͷ
	void Pitch(float angle);
	// ����ҡͷ
	void RotateY(float angle);

	// �޸������λ��/����󣬵����ؽ���ͼ����
	void UpdateViewMatrix();

private:

	// ���������ռ�����ϵ���������ϵ��
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;

	// ����ƽ��������
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	// ���� View/Proj ����.
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
};

#endif // CAMERA_H