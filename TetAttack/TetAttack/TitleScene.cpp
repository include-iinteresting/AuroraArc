#include "stdafx.h"
#include "TitleScene.h"
#include <d2d1.h>

/**
* @brief	コンストラクタ
*/
CTitleScene::CTitleScene()
{
}


/**
* @brief	デストラクタ
*/
CTitleScene::~CTitleScene()
{
}


/**
* @brief	更新
* @return	GameSceneResultCode		シーンの状態
*/
GameSceneResultCode CTitleScene::move()
{
	return GameSceneResultCode::GAMESCENE_DEFAULT;
}

/**
* @brief	pRenderTarget	レンダーターゲット
*/
void CTitleScene::draw(ID2D1RenderTarget * pRenderTarget)
{
	ID2D1SolidColorBrush *pBrush = NULL;

	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &pBrush);

	D2D1_SIZE_F size = pRenderTarget->GetSize();
	D2D1_RECT_F rc;
	rc.left = 0.0f;
	rc.top = 0.0f;
	rc.right = size.width;
	rc.bottom = size.height;

	pRenderTarget->FillRectangle(rc, pBrush);

	if (pBrush)
	{
		pBrush->Release();
		pBrush = NULL;
	}
}
