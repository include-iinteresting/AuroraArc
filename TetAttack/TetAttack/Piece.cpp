#include "StdAfx.h"
#include <d2d1.h>
#include "Selector.h"
#include "TetrisScene.h"
#include "Piece.h"


CPiece::CPiece(CTetrisScene *pt)
{
	m_pScene = pt;
	resetPos();
}


CPiece::~CPiece(void)
{

}

//  落下ピースをリセット
void CPiece::resetPos() {
	int    rnd = rand();
	m_iType = (rnd % 7);
	m_iAngle = 0;
	setPiece(m_iType, m_iAngle);
	m_iColX = (m_pScene->kCols >> 1) - (m_iSize >> 1);
	m_iRowY = -m_iSize;

}

//  落下ピースに形状をセット
void CPiece::setPiece(int type, int angle) {
	int        k = 0;
	k = type * shape::TYPE_PITCH;
	k += angle * shape::ROTATION_PITCH;
	for (int r = 0; r < PIECE_SIZE; ++r) {
		for (int c = 0; c < PIECE_SIZE; ++c) {
			m_iBlocks[r*PIECE_SIZE + c] = shape::data[k++];
		}
	}
	m_iSize = shape::size[type];
}

//  落下ピースを一段階落下
BOOL CPiece::down() {
	int    r = m_iRowY + 1;
	m_iRowY = r;
	return    false;
}

//  落下ピースを描画する
void CPiece::draw(ID2D1RenderTarget *pRenderTarget) {
	int    x, y;
	int    nx, ny;
	int    col, row;

	row = m_iRowY;
	y = (row * CTetrisScene::kFieldHeight) / CTetrisScene::kRows;
	for (int i = 0; i < m_iSize; ++i) {
		++row;
		ny = (row * CTetrisScene::kFieldHeight) / CTetrisScene::kRows;

		col = m_iColX;
		x = (col * CTetrisScene::kFieldWidth) / CTetrisScene::kCols;
		for (int j = 0; j < m_iSize; ++j) {
			++col;
			nx = (col*CTetrisScene::kFieldWidth) / CTetrisScene::kCols;
			if (m_iBlocks[i*PIECE_SIZE + j] > 0) {
				if (m_iBlocks[i*PIECE_SIZE + j] != 0) {
					D2D1_RECT_F    rc;
					rc.left = x + CTetrisScene::kOffsetX;
					rc.top = y + CTetrisScene::kOffsetY;
					rc.right = nx - 2 + CTetrisScene::kOffsetX;
					rc.bottom = ny - 2 + CTetrisScene::kOffsetY;
					int    bk = m_iBlocks[i*PIECE_SIZE + j];
					if (bk > 0 && bk < CTetrisScene::kSentinelID) {
						bk = (bk % (CTetrisScene::kNumColors - 1)) + 1;
						pRenderTarget->FillRectangle(&rc, m_pScene->m_ppBrushes[bk]);
					}
				}
			}
			x = nx;
		}
		y = ny;
	}
}


const int shape::size[] = { 2,4,3,3,3,3,3 };
const int shape::data[] = {
	//    ■
	1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	//    Ｉ
	0, 4, 0, 0,
	0, 4, 0, 0,
	0, 4, 0, 0,
	0, 4, 0, 0,

	0, 0, 0, 0,
	4, 4, 4, 4,
	0, 0, 0, 0,
	0, 0, 0, 0,

	0, 4, 0, 0,
	0, 4, 0, 0,
	0, 4, 0, 0,
	0, 4, 0, 0,

	0, 0, 0, 0,
	4, 4, 4, 4,
	0, 0, 0, 0,
	0, 0, 0, 0,

	//    └

	0, 13,0, 0,
	0, 13,0, 0,
	0, 13,13,0,
	0, 0, 0, 0,

	0, 0, 13,0,
	13,13,13,0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	13,13,0, 0,
	0, 13,0, 0,
	0, 13,0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	13,13,13,0,
	13,0, 0, 0,
	0, 0, 0, 0,

	//    ┘
	0, 6, 0, 0,
	0, 6, 0, 0,
	6, 6, 0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	6, 6, 6, 0,
	0, 0, 6, 0,
	0, 0, 0, 0,

	0, 6, 6, 0,
	0, 6, 0, 0,
	0, 6, 0, 0,
	0, 0, 0, 0,

	6, 0, 0, 0,
	6, 6, 6, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	//    ┣
	0, 3, 0, 0,
	0, 3, 3, 0,
	0, 3, 0, 0,
	0, 0, 0, 0,

	0, 3, 0, 0,
	3, 3, 3, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,

	0, 3, 0, 0,
	3, 3, 0, 0,
	0, 3, 0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	3, 3, 3, 0,
	0, 3, 0, 0,
	0, 0, 0, 0,

	//    Natural
	2, 0, 0, 0,
	2, 2, 0, 0,
	0, 2, 0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	0, 2, 2, 0,
	2, 2, 0, 0,
	0, 0, 0, 0,

	2, 0, 0, 0,
	2, 2, 0, 0,
	0, 2, 0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	0, 2, 2, 0,
	2, 2, 0, 0,
	0, 0, 0, 0,

	//    Natural（反転）
	0, 15,0, 0,
	15,15,0, 0,
	15,0, 0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	15,15,0, 0,
	0, 15,15,0,
	0, 0, 0, 0,

	0, 15,0, 0,
	15,15,0, 0,
	15,0, 0, 0,
	0, 0, 0, 0,

	0, 0, 0, 0,
	15,15,0, 0,
	0, 15,15,0,
	0, 0, 0, 0
};