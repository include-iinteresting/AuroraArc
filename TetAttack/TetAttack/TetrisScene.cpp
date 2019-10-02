#include "StdAfx.h"
#include <d2d1.h>
#include <tchar.h>
#include <dwrite.h>
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

//  �R���X�g���N�^
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

//  �f�X�g���N�^
CTetrisScene::~CTetrisScene(void)
{
	//  ��Еt��
	SAFE_DELETE(m_pPiece);

	for (int i = 0; i < kNumColors; ++i) {
		SAFE_RELEASE(m_ppBrushes[i]);
	}
	delete m_ppBrushes;
	SAFE_DELETE(m_iBlocks);
}

//  �Ֆʂ̏�����
void    CTetrisScene::reset() {
	SAFE_DELETE(m_iBlocks);
	m_iBlocks = new int[kRows*kCols];
	int    k = 0;
	for (int r = 0; r < kRows; ++r) {
		for (int c = 0; c < kCols; ++c) {
			m_iBlocks[r * kCols + c] = 0;  //  ���̍s�̉E�ӂ� k+1 �ɂ���ƃu���b�N���o����i���ɖ߂����j
			k = (k + 1) % (kNumColors - 1);
		}
	}

	SAFE_DELETE(m_pPiece);
	m_pPiece = new CPiece(this);
	m_iTime = 0;

	m_bGameOver = false;


	//  �L�[����֘A�̏�����
	m_bEnter2 = true;
	m_bDown2 = true;
	m_iLeftWait = 0;
	m_iRightWait = 0;

	//  ��Ԃ̏�����
	m_eTetrisPhase = TETRISPHASE_INIT;
}

//  �A�j���[�V��������
GameSceneResultCode    CTetrisScene::move() {
	

	switch (m_eTetrisPhase) {
	case    TETRISPHASE_INIT:
		m_eTetrisPhase = TETRISPHASE_GAME;
	case    TETRISPHASE_GAME:  //  �u���b�N���蒅����܂ŗ���������
		m_iTime++;
		if (!doPiece()) {
			m_eTetrisPhase = TETRISPHASE_WAIT;
			m_iPhaseWait = 0;
			if (m_bGameOver) {
				m_eTetrisPhase = TETRISPHASE_GAMEOVER;
			}
		}
		break;
	case    TETRISPHASE_WAIT:  //  ��莞�ԑ҂������ƁA�u���b�N������
		m_iPhaseWait++;
		if (m_iPhaseWait > 30) {
			m_iPhaseWait = 0;
			m_eTetrisPhase = TETRISPHASE_GAME;
			scanField();
		}
	case    TETRISPHASE_GAMEOVER:
		m_iPhaseWait++;
		if (m_iPhaseWait > 120) {
			m_iPhaseWait = 0;
			reset();
			m_eTetrisPhase = TETRISPHASE_INIT;
		}
	}
	return    GAMESCENE_DEFAULT;
}

//  �����s�[�X�̃A�j���[�V����
BOOL    CTetrisScene::doPiece() {
	if (m_pPiece == NULL) {
		return    false;
	}


	//  �L�[����Ńs�[�X�𓮂���
	if (GetAsyncKeyState(VK_RETURN)) {
		if (!m_bEnter2)
			m_pPiece->rotate();
		m_bEnter2 = true;
	}
	else {
		m_bEnter2 = false;
	}
	if (GetAsyncKeyState(VK_LEFT)) {
		if (0 >= --m_iLeftWait) {
			m_pPiece->moveHorizontal(-1);
			m_iLeftWait = 12;
		}

	}
	else {
		m_iLeftWait = 1;
	}
	if (GetAsyncKeyState(VK_RIGHT)) {
		if (0 >= --m_iRightWait) {
			m_pPiece->moveHorizontal(+1);
			m_iRightWait = 12;
		}
	}
	else {
		m_iRightWait = 1;
	}
	if (GetAsyncKeyState(VK_DOWN)) {
		if (!m_bDown2)
			m_iWait = 6;
		m_bDown2 = true;
	}
	else {
		m_iWait = 30;
		m_bDown2 = false;
	}

	if (m_iTime > m_iWait) {
		m_iTime = 0;
		return    m_pPiece->down();
	}
	return    true;
}



//  �`�揈��
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
	//  �����s�[�X�̕`��
	if (m_pPiece != NULL) {
		m_pPiece->draw(pRenderTarget);
	}

	if (m_bGameOver) {
		IDWriteTextFormat *pFormat = m_pSystem->GetSystemTextFormat();
		rc.left = kOffsetX;
		rc.top = kOffsetY;
		rc.right = rc.left + kFieldWidth;
		rc.bottom = rc.top + kFieldHeight;
		DWRITE_TEXT_ALIGNMENT oldAlignment = pFormat->GetTextAlignment();
		pFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
		pRenderTarget->DrawText(_T("GAME OVER"), 9, pFormat, &rc, m_ppBrushes[7]);
		pFormat->SetTextAlignment(oldAlignment);
		SAFE_RELEASE(pFormat);
	}

}


//  �^����ꂽ�s�[�X�ƃu���b�N��ǂƂ̓����蔻����s���B
//  �������Ă��Ȃ��Ƃ� true �Փ˂���� false
BOOL CTetrisScene::collide(int col, int row, int size, int *block) {
	int    c, r = row;
	for (int i = 0; i < size; ++i) {
		c = col;
		for (int j = 0; j < size; ++j) {
			if (block[i*PIECE_SIZE + j]>0) {
				if (r >= kRows)
					return    false;
				if (c < 0)
					return    false;
				if (c >= kCols)
					return    false;
				if (r >= 0 && m_iBlocks[r*kCols + c] > 0)
					return false;
			}
			c++;
		}
		r++;
	}
	return    true;
}

//  �^����ꂽ�s�[�X�̃f�[�^���A�t�B�[���h�ɃR�s�[����B
void CTetrisScene::copy(int col, int row, int size, int *block) {

	int    c;
	int r = row;
	for (int i = 0; i < size; ++i) {
		c = col;
		for (int j = 0; j < size; ++j) {
			if (block[i*PIECE_SIZE + j] > 0) {
				if (r < 0) {
					m_bGameOver = true;  //  ��ԏ�̍s����ɃR�s�[������Q�[���I�[�o�[
					return;
				}
				m_iBlocks[r*kCols + c] = block[i*PIECE_SIZE + j];
			}
			++c;
		}
		r++;
	}
}


//  �t�B�[���h���X�L����
void CTetrisScene::scanField() {
	int    delLines = 0;
	int    i = kRows - 1;
	int    j;
	while (i >= 0) {
		for (j = 0; j < kCols; ++j) {
			if (m_iBlocks[i*kCols + j] == 0)
				break;
		}
		if (j == kCols) {
			++delLines;
			deleteLine(i);
		}
		else {
			--i;
		}
	}
}

//  �s�̍폜
void CTetrisScene::deleteLine(int line) {
	int    c;
	for (int j = line; j > 0; --j) {
		for (c = 0; c < kCols; ++c) {
			m_iBlocks[j*kCols + c] = m_iBlocks[(j - 1)*kCols + c];
		}
	}
	for (c = 0; c < kCols; ++c) {
		m_iBlocks[c] = 0;
	}
}