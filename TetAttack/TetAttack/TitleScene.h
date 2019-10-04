#pragma once
#include "IGameScene.h"

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
	ID2D1Bitmap * m_pBGImage;
};

