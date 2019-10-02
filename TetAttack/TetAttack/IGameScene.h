//
//  class IGameScene
//
#ifndef __IGAMESCENE_H__
#define __IGAMESCENE_H__
class IGameScene
{
public:
	virtual ~IGameScene() = 0;
	virtual GameSceneResultCode    move() = 0;
	virtual void    draw(ID2D1RenderTarget *pRenderTarget) = 0;
	//virtual void    reset() = 0;
};
#endif