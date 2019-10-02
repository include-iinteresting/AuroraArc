#pragma once


#ifndef __DIRECTSOUNT_H__
#define __DIRECTSOUND_H__


#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY
#undef SAFE_RELEASE
#define SAFE_DELETE(o)	if(o) { delete(o); o = NULL; };
#define SAFE_DELETE_ARRAY(o)	if(o) { delete[](o); o = NULL; };
#define SAFE_RELEASE(o)	if(o) { (o)->Release(); o = NULL; };

struct IDirectSound8;
struct IDirectSoundBuffer;

/**
* @class	CDirectSound
* @brief	DirectSound‚ðˆµ‚¤‚½‚ß‚Ì‘å–{‚ÌƒNƒ‰ƒX
*/
class CDirectSound
{
public:
	static	void	Initialize(HWND hWnd);
	static	void	Finalize();
	static	IDirectSound8		*GetDirectSound8();
	static	IDirectSoundBuffer	*GetPrimaryBuffer();
};


#endif
