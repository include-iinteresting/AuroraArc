#pragma once
#include "IGameScene.h"

enum TitlePhase : short
{
	TITLEPHASE_INIT = 0x00,
	TITLEPHASE_RUN = 0x01,
	TITLEPHASE_DONE = 0x02
};

struct ID2D1Bitmap;

class CTitleScene :
	public IGameScene
{
public:
	CTitleScene(ID2D1RenderTarget *pRenderTarget);
	virtual ~CTitleScene();
	virtual	GameSceneResultCode	move()	override;
	virtual	void	draw(ID2D1RenderTarget *pRenderTarget)	override;
private:
	void	InitTexture(ID2D1RenderTarget *pRenderTarget);
private:
	ID2D1Bitmap *m_pBGImage;	//!	BGImage
	TitlePhase	m_ePhase;		//!	TitlePhase
};

