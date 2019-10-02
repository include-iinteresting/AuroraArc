#pragma once
#include "IOneshotSound.h"



class CWaveSound : public IOneshotSound
{
public:
	CWaveSound(const char* pFilename);
	virtual ~CWaveSound();

	virtual	void	Play(DWORD dwPriority, LONG lVol)	override;
	virtual	void	Stop()	override;
private:
	void			LoadPcmData(const char* pFilename);
	void			CreateSecondaryBuffer();
private:
	const char*			m_pFilename;
	IDirectSoundBuffer	*m_pDSB;
	IDirectSoundBuffer8	*m_pSecondary;

	WAVEFORMATEX		m_wfx;
	BYTE*				m_pBuffer;
	DWORD				m_dwSize;
};

