#pragma once

#include "IStreamingSound.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

/**
* @class	OggSound
* @brief	Oggâπê∫ÇàµÇ§ÉNÉâÉX
*/
class COggSound : public IStreamingSound
{
public:
	COggSound(const char* pFilename, bool bLoopFlag);
	virtual ~COggSound();
	void	Update()	override;

	void	Play(DWORD dwPriority = 0, DWORD dwFlag = 0)	override;
	void	Stop()	override;
	void	Done()	override;
	void	SetLoopFlag(bool bFlag)	override;
	void	SetVolume(LONG lVolume)	override;
	LONG	GetVolume()	override;
private:
	void			CreateSecondaryBuffer();
	unsigned int	GetPcmBufferFromOgg(OggVorbis_File *ovf, char* buffer, int bufferSize, bool isLoop, bool* isEnd = 0);
private:
	IDirectSoundBuffer*		m_pDSB;
	IDirectSoundBuffer8*	m_pSecondary;

	const char*				m_pFilename;
	OggVorbis_File			m_ovf;

	unsigned	int			m_uiBufSize;		//!	DirectSoundBufferSize
	unsigned	int			m_uiWriteFlag;
	DWORD					m_dwPoint;

	bool					m_bLoopFlag;		//!	LoopFlag
	bool					m_bFinFlag;			//!	FinishFlag
};

