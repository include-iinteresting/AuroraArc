//
//  class IGameScene
//
#ifndef __IGAMESCENE_H__
#define __IGAMESCENE_H__

//
//  Scene ÇÃèIÇÌÇËï˚íËêî
//  IGameScene::move() Ç™ï‘Ç∑íl
//
enum GameSceneResultCode {
	GAMESCENE_DEFAULT = 0,
	GAMESCENE_END_OK = 1,
	GAMESCENE_END_TIMEOUT = 2,
	GAMESCENE_END_FAILURE = 3,
};

struct ID2D1RenderTarget;

class IGameScene
{
public:
	virtual ~IGameScene() = 0;
	virtual GameSceneResultCode    move() = 0;
	virtual void    draw(ID2D1RenderTarget *pRenderTarget) = 0;
	//virtual void    reset() = 0;
};
#endif