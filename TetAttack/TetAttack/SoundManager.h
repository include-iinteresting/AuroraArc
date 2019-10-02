#pragma once


/**
* @class	CSoundManager
* @brief	サウンド管理クラス
*/
class CSoundManager
{
public:
	//!<	SoundManager
	static	void	Initialize(HWND hWnd);
	static	void	Finalize();

	//!<	StreamingSound
	static	void	LoadStreamingSound(const char* pFilename, bool bLoopFlag);
	static	void	UnloadStreamingSound(unsigned int numSound);
	static	void	StreamPlay(unsigned int numSound, DWORD dwPriority, DWORD dwFlag);
	static	void	StreamStop(unsigned int numSound);
	static	void	StreamDone(unsigned int numSound);

	//!<	OneshotSound
	static	void	LoadOneshotSound(const char* pFilename);
	static	void	UnloadOneshotSound(unsigned int numSound);
	static	void	OneshotPlay(unsigned int numSound, DWORD dwPriority, LONG lVolume);
	static	void	OneshotStop(unsigned int numSound);

};

