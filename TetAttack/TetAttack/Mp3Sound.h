#pragma once
#include "IStreamingSound.h"

#ifndef __ACM_H__
#define __ACM_H__
#include <mmreg.h>
#include <MSAcm.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "msacm32.lib")
#endif

class CMp3Sound :
	public IStreamingSound
{
public:
	CMp3Sound(const char *pFilename, bool bLoopFlag);
	virtual ~CMp3Sound();

	virtual	void	Update()	override;
	virtual	void	Play(DWORD dwPriority, DWORD dwFlag)	override;
	virtual	void	Stop()	override;
	virtual	void	Done()	override;
	virtual	void	SetLoopFlag(bool bFlag)	override;
	virtual	void	SetVolume(LONG lVolume) override;
	virtual	LONG	GetVolume()	override;
private:
	void			CreateSecondaryBuffer(WAVEFORMATEX wfx);
	void			DecodePrepare();
	unsigned	int	GetPcmBufferFromMp3(HANDLE hFile, HACMSTREAM has, ACMSTREAMHEADER ash, char* buffer, int bufferSize, bool isLoop, bool* isEnd);
private:
	IDirectSoundBuffer	*m_pDSB;
	IDirectSoundBuffer8	*m_pSecondary;

	bool			m_bLoopFlag;
	bool			m_bFinFlag;

	const	char*	m_pFilename;

	HANDLE			m_hFile;
	HACMSTREAM		m_has;
	ACMSTREAMHEADER	m_ash;

	unsigned	int	m_uiBufSize;
	unsigned	int	m_uiWriteFlag;
	DWORD			m_dwPoint;
	LONG			m_lStartBlockPos;
};

