//***************************************************************************************
// BlurFilter.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a blur operation on the topmost mip level of an input texture.
//***************************************************************************************

#ifndef BLURFILTER_H
#define BLURFILTER_H

#include <Windows.h>
#include <DirectXMath.h>
#include "d3dUtil.h"

class BlurFilter
{
public:
	BlurFilter();
	~BlurFilter();


	ID3D11ShaderResourceView* GetBlurredOutput();

	// ���ɸ�˹ģ��Ȩ�ء�
	void SetGaussianWeights(float sigma);

	// �ֶ�ָ��ģ��Ȩ�ء�
	void SetWeights(const float weights[9]);

	///<summary>
	/// ��Ⱥ͸߶�Ӧ����������ĳߴ���ƥ����ģ����
	/// �����ٴε���Init���������³�ʼ��ģ��������
	/// �ߴ���ʽ��
	///</summary>
	void Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);

	///<summary>
	/// ģ�����������ģ�������� ��ע�⣬����޸������������������ĸ�����
	///</summary>
	void BlurInPlace(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount);

private:

	UINT mWidth;
	UINT mHeight;
	DXGI_FORMAT mFormat;

	ID3D11ShaderResourceView* mBlurredOutputTexSRV;
	ID3D11UnorderedAccessView* mBlurredOutputTexUAV;
};

#endif // BLURFILTER_H