#ifndef D3DAPP_H
#define D3DAPP_H

#include <windows.h>
#include <string>
#include "D3DUtil.h"
#include "GameTimer.h"
#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

#pragma comment(lib, "legacy_stdio_definitions.lib")


class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	int Run();

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene();
	virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//����¼�
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	//����FPS
	void CalculateFrameStats();

protected:
	HINSTANCE m_hAppInst;                            //ʵ�����
	HWND m_hMainWnd;                                //�����ھ��
	bool m_isAppPaused;
	bool m_isMinimized;
	bool m_isMaximized;
	bool m_isResizing;
	UINT m_4xMsaaQuality;                            //4�ز�������
	
	GameTimer m_timer;                                //��ʱ��
	D3D_DRIVER_TYPE m_d3dDriverType;                //��������
	ID3D11Device *m_pD3dDevice;                        //�豸
	ID3D11DeviceContext *m_pImmediateContext;        //�豸������
	IDXGISwapChain *m_pSwapChain;                    //������
	ID3D11Texture2D *m_pDepthStencilBuffer;            //���ģ�建��
	ID3D11RenderTargetView *m_pRenderTargetView;    //��ȾĿ����ͼ
	ID3D11DepthStencilView *m_pDepthStencilView;    //���ģ�建����ͼ
	D3D11_VIEWPORT m_screenViewPort;                //�ӿ�

	std::wstring m_mainWndCaption;                    //���ڱ���
	int m_clientWidth;
	int m_clientHeight;
	bool m_isEnable4xMsaa;                            //�Ƿ�֧��4�ز���
};
#endif    //D3DAPP_H
