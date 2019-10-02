#pragma once
#include "IGameScene.h"
class CTitle :
	public IGameScene
{
public:
	CTitle();
	virtual ~CTitle();
	virtual	GameSceneResultCode	move()	override;
	virtual	void	draw(ID2D1RenderTarget *pRenderTarget)	override;
};

