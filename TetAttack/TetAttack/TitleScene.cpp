#include "stdafx.h"
#include "TitleScene.h"
#include <d2d1.h>
#include "TextureLoader.h"
#include "SocketClient.h"

#define BG_TEXTURE_PATH _T("res/tetattack.png")

#undef SAFE_RELEASE
#define SAFE_RELEASE(o) if(o) { (o)->Release(); o = NULL; };

/**
* @brief	コンストラクタ
* @param	[in]	pRenderTarget	レンダーターゲット
*/
CTitleScene::CTitleScene(ID2D1RenderTarget *pRenderTarget)
{
	InitTexture(pRenderTarget);

	m_ePhase = TitlePhase::TITLEPHASE_INIT;

	m_pSocket = new CSocketClient();
	m_pSocket->ConnectServer();

	char pBuf[32];
	memset(pBuf, 0, sizeof(pBuf));
	m_pSocket->Receive(pBuf, sizeof(pBuf));
	int a = 0;
}


/**
* @brief	デストラクタ
*/
CTitleScene::~CTitleScene()
{
	SAFE_RELEASE(m_pBGImage);
	if (m_pSocket)
	{
		delete m_pSocket;
		m_pSocket = NULL;
	}
}


/**
* @brief	更新
* @return	GameSceneResultCode		シーンの状態
*/
GameSceneResultCode CTitleScene::move()
{
	switch (m_ePhase)
	{
	case TITLEPHASE_INIT:
		m_ePhase = TitlePhase::TITLEPHASE_RUN;
		break;
	case TITLEPHASE_RUN:
		if (GetAsyncKeyState(VK_SPACE))
			m_ePhase = TitlePhase::TITLEPHASE_DONE;
		break;
	case TITLEPHASE_DONE:
		return GameSceneResultCode::GAMESCENE_END_OK;
	}

	return GameSceneResultCode::GAMESCENE_DEFAULT;
}


/**
* @brief	描画
* @param	[in]	pRenderTarget	レンダーターゲット
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
