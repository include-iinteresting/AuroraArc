#include "stdafx.h"
#include <crtdbg.h>
#include <tchar.h>
#include <stdio.h>
#include "TextureLoader.h"

#include <d2d1.h>
#include <dwrite.h>    //  文字表示に使用
#include "Selector.h"
#pragma comment(lib,"d2d1.lib")    //  direct2d に必要
#pragma comment(lib,"dwrite.lib")  //  文字表示に使用

#undef SAFE_RELEASE
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY
#define SAFE_RELEASE(o) if (o){ (o)->Release(); o = NULL; }
#define SAFE_DELETE(o)  if (o){ delete (o); o = NULL; }
#define SAFE_DELETE_ARRAY(o) if (o){ delete [] (o); o = NULL; }



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//  グローバル変数
ID2D1Factory             *g_pD2DFactory = NULL;      //  D2D1Factoryオブジェクト
ID2D1HwndRenderTarget    *g_pRenderTarget = NULL;    //  描画ターゲット

double      g_dblDenominator;
double      g_dblFrame;
__int64     g_i64OldTimer;

CSelector    *g_pSelector = NULL;

#define FPS   30
#define INTERVAL   (1.0/FPS)



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//
//  関数：WinMain
//  説明：アプリケーションのエントリーポイント
//
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(161);

	CoInitialize(NULL);

	//  変数宣言
	WNDCLASSEX wcex;  //  ウィンドウクラス構造体
	HWND hWnd;        //  ウィンドウハンドル
	HWND hScreen;
	RECT    bounds, client,screen;  //  RECT 構造体

							 //  (1)初期化部分

							 //  (1)-a ウィンドウクラスの登録
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;  //  ウィンドウプロシージャの登録
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;  //  アプリケーションインスタンス
	wcex.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("Tetris");  //  ウィンドウクラス名
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	hScreen = GetDesktopWindow();
	GetWindowRect(hScreen, &screen);//&のほうに画面の大きさが入ってる

	//  (1)-b ウィンドウの生成
	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, wcex.lpszClassName, _T("Tetris"),
		WS_VISIBLE | WS_POPUP,
		screen.left, screen.top, screen.right, screen.bottom, NULL, NULL, hInstance, NULL);
	/*
	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, wcex.lpszClassName, _T("Tetris"),
		WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 1280, 960, NULL, NULL, hInstance, NULL);
	*/
	if (!hWnd)
		return FALSE;

	//  ウィンドウサイズの調整
	//  ここは無くても動く
	GetWindowRect(hWnd, &bounds);
	GetClientRect(hWnd, &client);
	MoveWindow(hWnd, bounds.left, bounds.top,
		screen.right * 2 - client.right,
		screen.bottom * 2 - client.bottom,
		false);

	//  ウィンドウの再表示
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	//  再表示の際、WndProc が呼び出される事にも注意！

	//  Direct2D 初期化
	{
		HRESULT hr;
		//  Direct2D Factory の生成
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
		GetClientRect(hWnd, &client);
		D2D1_SIZE_U size = D2D1::SizeU(
			client.right - client.left,
			client.bottom - client.top
		);

		//  Direct2D 描画ターゲットの生成
		hr = g_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&g_pRenderTarget
		);
	}

	//  ゲームシステムの初期化
	g_pSelector = new CSelector(g_pRenderTarget);

	//  タイマーの初期化
	::QueryPerformanceCounter((LARGE_INTEGER*)&g_i64OldTimer);
	__int64   i64Tmp;
	::QueryPerformanceFrequency((LARGE_INTEGER*)&i64Tmp);
	g_dblDenominator = 1.0 / (double)i64Tmp;
	g_dblFrame = 0.0f;



	//  (2)メッセージループ
	MSG        msg;
	while (true) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {

			double   t;
			::QueryPerformanceCounter((LARGE_INTEGER*)&i64Tmp);
			t = (i64Tmp - g_i64OldTimer) * g_dblDenominator;
			g_i64OldTimer = i64Tmp;
			g_dblFrame += t;

			if (g_dblFrame >= INTERVAL) {
				int   c = (int)(g_dblFrame / INTERVAL);
				g_dblFrame -= INTERVAL * c;

				if (g_pSelector)
					g_pSelector->doAnim();

				InvalidateRect(hWnd, NULL, false);  //  書き換えの実行
			}


		}
	}

	//  (3)終了処理

	SAFE_DELETE(g_pSelector);
	SAFE_RELEASE(g_pRenderTarget);
	SAFE_RELEASE(g_pD2DFactory);

	CTextureLoader::Destroy();


	CoUninitialize();
	return (int)msg.wParam;
}


//
//  関数：WndProc
//  説明：ウインドウに渡されたイベントのハンドラ
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {

	case WM_PAINT:
		if (g_pRenderTarget) {
			g_pRenderTarget->BeginDraw();  //  描画開始

			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity()); //  Transform の設定

			g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));//  画面のクリア

			if (g_pSelector)
				g_pSelector->doDraw(g_pRenderTarget);

			g_pRenderTarget->EndDraw();  //  描画終了
		}
		ValidateRect(hWnd, NULL);
		break;


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return    0;
}