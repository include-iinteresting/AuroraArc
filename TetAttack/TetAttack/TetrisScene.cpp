#include "StdAfx.h"
#include <d2d1.h>
#include "Selector.h"
#include "TetrisScene.h"
#include "Piece.h"

//  Settings
#define    OFFSET_X    0
#define    OFFSET_Y    0
#define    FIELD_WIDTH    480
#define    FIELD_HEIGHT    480

static D2D1::ColorF    colors[] = {
	D2D1::ColorF(0.f,0.f,0.f),    //  0:black
	D2D1::ColorF(0.f,0.f,1.f),    //  1:blue
	D2D1::ColorF(0.f,1.f,0.f),    //  2:green
	D2D1::ColorF(0.f,1.f,1.f),    //  3:cyan
	D2D1::ColorF(1.f,0.f,0.f),    //  4:red
	D2D1::ColorF(1.f,1.f,0.f),    //  5:magenta
	D2D1::ColorF(1.f,1.f,0.f),    //  6:yellow
	D2D1::ColorF(1.f,1.f,1.f),    //  7:white
	D2D1::ColorF(.5f,.5f,.5f),    //  8:gray
};

//  コンストラクタ
CTetrisScene::CTetrisScene(CSelector *pv)
{
	m_pSystem = pv;
	m_iBlocks = NULL;
	ID2D1RenderTarget *pRenderTarget = NULL;
	pRenderTarget = pv->GetRenderTaget();
	m_ppBrushes = new ID2D1SolidColorBrush*[kNumColors];
	for (int i = 0; i < kNumColors; ++i) {
		pRenderTarget->CreateSolidColorBrush(
			colors[i],
			&m_ppBrushes[i]
		);
	}
	SAFE_RELEASE(pRenderTarget);

	m_pPiece = NULL;
	m_iWait = 30;
	m_iTime = 0;
	srand(::time_t(NULL));

	reset();
}

//  デストラクタ
CTetrisScene::~CTetrisScene(void)
{
	//  後片付け
	SAFE_DELETE(m_pPiece);

	for (int i = 0; i < kNumColors; ++i) {
		SAFE_RELEASE(m_ppBrushes[i]);
	}
	delete m_ppBrushes;
	SAFE_DELETE(m_iBlocks);
}

//  盤面の初期化
void    CTetrisScene::reset() {
	SAFE_DELETE(m_iBlocks);
	m_iBlocks = new int[kRows*kCols];
	int    k = 0;
	for (int r = 0; r < kRows; ++r) {
		for (int c = 0; c < kCols; ++c) {
			m_iBlocks[r * kCols + c] = 0;  //  この行の右辺を k+1 にするとブロックを出せる（元に戻す事）
			k = (k + 1) % (kNumColors - 1);
		}
	}

	SAFE_DELETE(m_pPiece);
	m_pPiece = new CPiece(this);
	m_iTime = 0;

	m_bGameOver = false;
}

//  アニメーション処理
GameSceneResultCode    CTetrisScene::move() {
	m_iTime++;
	doPiece();

	return    GAMESCENE_DEFAULT;
}

//  落下ピースのアニメーション
BOOL    CTetrisScene::doPiece() {
	if (m_pPiece == NULL) {
		return    false;
	}
	if (m_iTime > m_iWait) {
		m_iTime = 0;
		return    m_pPiece->down();
	}
	return    true;
}



//  描画処理
void    CTetrisScene::draw(ID2D1RenderTarget *pRenderTarget) {
	int    x, y, nx, ny;
	D2D1_RECT_F    rc;
	x = 0, y = 0;

	for (int r = 0; r < kRows; ++r) {
		x = 0;
		ny = (r + 1) * kFieldHeight / kRows;
		for (int c = 0; c < kCols; ++c) {
			nx = (c + 1) * kFieldWidth / kCols;
			int    bk = m_iBlocks[r * kCols + c];
			if (bk != 0 && bk < kSentinelID) {
				bk = (bk % (kNumColors - 1)) + 1;
				rc.left = x + kOffsetX;
				rc.top = y + kOffsetY;
				rc.right = nx - 2 + kOffsetX;
				rc.bottom = ny - 2 + kOffsetY;
				pRenderTarget->FillRectangle(&rc, m_ppBrushes[bk]);
			}
			x = nx;
		}
		y = ny;
	}
	//  落下ピースの描画
	if (m_pPiece != NULL) {
		m_pPiece->draw(pRenderTarget);
	}
}