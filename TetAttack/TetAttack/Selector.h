/******************************************************
*@file Selector.h
*  ゲームシーンセレクタ CSelector の宣言
******************************************************/
#pragma once

enum GamePhase {
	GAMEPHASE_INIT = 0,
	GAMEPHASE_RESET = 0x010,
	GAMEPHASE_GAME = 0x100,
	GAMEPHASE_CLEAR = 0x120
};

//
//  Scene の終わり方定数
//  IGameScene::move() が返す値
//
enum GameSceneResultCode {
	GAMESCENE_DEFAULT = 0,
	GAMESCENE_END_OK = 1,
	GAMESCENE_END_TIMEOUT = 2,
	GAMESCENE_END_FAILURE = 3,
};

class    IGameScene;    //  シーン管理クラス
struct   ID2D1RenderTarget;
struct   IDWriteTextFormat;
struct   ID2D1SolidColorBrush;

/**
*@class CSelector
*/
class CSelector
{
public:
	CSelector(ID2D1RenderTarget *pRenderTarget);
	virtual ~CSelector(void);
	void    doAnim();
	void    doDraw(ID2D1RenderTarget *pRenderTarget);
	ID2D1RenderTarget   *GetRenderTaget();
	IDWriteTextFormat   *GetSystemTextFormat();
	//  待ち時間定数(msec)
	static const int k_iReadyTime = 2000;
	static const int k_iAnimTime = 250;
	static const int k_iClearTime = 3000;

protected:
	ID2D1RenderTarget * m_pRenderTarget;
	IDWriteTextFormat   *m_pTextFormat;
	ID2D1SolidColorBrush *m_pWhiteBrush;
	INT          m_iCount;
	INT          m_iWait;
	INT          m_iCurrent;
	IGameScene   *m_pScene;
	GamePhase    m_eGamePhase;
};

//  マクロ定義

#undef SAFE_RELEASE
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY
#define SAFE_RELEASE(o) if (o){ (o)->Release(); o = NULL; }
#define SAFE_DELETE(o)  if (o){ delete (o); o = NULL; }
#define SAFE_DELETE_ARRAY(o) if (o){ delete [] (o); o = NULL; }