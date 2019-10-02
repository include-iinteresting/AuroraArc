#pragma once

struct IDirectSoundBuffer;
struct IDirectSoundBuffer8;

/**
* @class	IStreamingSound
* @brief	サウンドオブジェクトのインターフェース
*/
class IStreamingSound
{
public:
	virtual ~IStreamingSound();
	virtual	void	Update() = 0;
	virtual void	Play(DWORD dwPriority, DWORD dwFlag) = 0;
	virtual	void	Stop() = 0;
	virtual void	Done() = 0;
	virtual	void	SetLoopFlag(bool bFlag) = 0;
};

