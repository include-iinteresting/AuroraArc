#pragma once
#include "IGameScene.h"


class CTitleScene :
	public IGameScene
{
public:
	CTitleScene();
	virtual ~CTitleSceme();
	virtual	GameSceneResultCode	move()	override;
	virtual	void	draw(ID2D1RenderTarget *pRenderTarget)	override;
};

