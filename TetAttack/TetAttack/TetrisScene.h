#pragma once
#ifndef __IGAMESCENE_H__
#include "IGameScene.h"
#endif
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
protected:
	CSelector * m_pSystem;
	int         *m_iBlocks;
	BOOL        m_bGameOver;
	ID2D1SolidColorBrush      **m_ppBrushes;

	//  落下ピース関連
	int     m_iTime;
	CPiece  *m_pPiece;
	int     m_iWait;

	static const int kCols = 10;
	static const int kRows = 10;
	static const int kNumColors = 9;
	static const int kSentinelID = 0xff;
	static const int kOffsetX = 0;
	static const int kOffsetY = 0;
	static const int kFieldWidth = 480;
	static const int kFieldHeight = 480;
};