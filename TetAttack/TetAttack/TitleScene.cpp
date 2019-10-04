#include "stdafx.h"
#include "TitleScene.h"
#include <d2d1.h>
#include "TextureLoader.h"

#define BG_TEXTURE_PATH _T("res/TitleBG.png")

#undef SAFE_RELEASE
#define SAFE_RELEASE(o) if(o) { (o)->Release(); o = NULL; };

/**
* @brief	�R���X�g���N�^
*/
CTitleScene::CTitleScene(ID2D1RenderTarget *pRenderTarget)
{
	InitTexture(pRenderTarget);
}


/**
* @brief	�f�X�g���N�^
*/
CTitleScene::~CTitleScene()
{
}


/**
* @brief	�X�V
* @return	GameSceneResultCode		�V�[���̏��
*/
GameSceneResultCode CTitleScene::move()
{
	return GameSceneResultCode::GAMESCENE_DEFAULT;
}


/**
* @brief	pRenderTarget	�����_�[�^�[�Q�b�g
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
* @brief	�摜�Ɋւ��鏉����
*/
void CTitleScene::InitTexture(ID2D1RenderTarget *pRenderTarget)
{
	m_pBGImage = NULL;
	CTextureLoader::CreateD2D1BitmapFromFile(pRenderTarget, BG_TEXTURE_PATH, &m_pBGImage);
}
