#pragma once
#ifndef __IGAMESCENE_H__
#include "IGameScene.h"
#endif

enum TetrisPhase {
	TETRISPHASE_INIT = 0,
	TETRISPHASE_GAME = 0x100,
	TETRISPHASE_WAIT = 0x101,
	TETRISPHASE_GAMEOVER = 0x200
};
class CSelector;
enum GameSceneResultCode;
struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
class  CPiece;

class CTetrisScene : public IGameScene
{
	friend class CPiece;
public:
	CTetrisScene(CSelector *pv);
	virtual ~CTetrisScene(void);
	virtual GameSceneResultCode    move() override;
	virtual void    draw(ID2D1RenderTarget *pRenderTarget) override;
	virtual void    reset();
	virtual BOOL    doPiece();
	virtual BOOL    collide(int col, int row, int size, int *block);
	virtual void    copy(int col, int row, int size, int *block);
	virtual void    scanField();
	virtual void    deleteLine(int line);
protected:
	CSelector * m_pSystem;
	int         *m_iBlocks;
	BOOL        m_bGameOver;
	ID2D1SolidColorBrush      **m_ppBrushes;

	//  落下ピース関連
	int     m_iTime;
	CPiece  *m_pPiece;
	int     m_iWait;

	static const int kCols = 8;//ピースのサイズ元は10
	static const int kRows = 8;//ピースのサイず
	static const int kNumColors = 9;
	static const int kSentinelID = 0xff;
	static const int kOffsetX = 300;//表示位置
	static const int kOffsetY = 0;//表示位置
	static const int kFieldWidth = 520;// フィールドのサイズ？元は480
	static const int kFieldHeight = 520;

	//  キー入力
	BOOL    m_bEnter2;  //  連射防止
	BOOL    m_bDown2;   //  連射防止
	int     m_iLeftWait;
	int     m_iRightWait;

		
	//  状態
	enum TetrisPhase    m_eTetrisPhase;
	int  m_iPhaseWait;

};