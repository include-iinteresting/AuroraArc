#include "stdafx.h"
#include "TitleScene.h"
#include <d2d1.h>
#include "TextureLoader.h"

#define BG_TEXTURE_PATH _T("res/TitleBG.png")

#undef SAFE_RELEASE
#define SAFE_RELEASE(o) if(o) { (o)->Release(); o = NULL; };

/**
* @brief	コンストラクタ
*/
CTitleScene::CTitleScene(ID2D1RenderTarget *pRenderTarget)
{
	InitTexture(pRenderTarget);
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

	D2D1_SIZE_F size = pRenderTarget->GetSize();
	D2D1_RECT_F rc;
	rc.left = 0.0f;
	rc.top = 0.0f;
	rc.right = size.width;
	rc.bottom = size.height;

	if (m_pBGImage)
		pRenderTarget->DrawBitmap(m_pBGImage, rc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
}


/**
* @brief	画像に関する初期化
*/
void CTitleScene::InitTexture(ID2D1RenderTarget *pRenderTarget)
{
	m_pBGImage = NULL;
	CTextureLoader::CreateD2D1BitmapFromFile(pRenderTarget, BG_TEXTURE_PATH, &m_pBGImage);
}
