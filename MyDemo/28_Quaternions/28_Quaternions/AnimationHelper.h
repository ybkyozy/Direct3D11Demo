//***************************************************************************************
// AnimationHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Contains classes for defining keyframes and animation.
//***************************************************************************************

#ifndef ANIMATION_HELPER_H
#define ANIMATION_HELPER_H

#include "d3dUtil.h"

///<summary>
/// 一个关键帧定义了骨骼瞬时变换。
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

    float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes; 	

};

#endif // ANIMATION_HELPER_H