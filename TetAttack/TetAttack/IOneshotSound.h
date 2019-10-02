#pragma once

struct IDirectSoundBuffer;
struct IDirectSoundBuffer8;

/**
* @class	IOneshotSound
* @brief	�����V���b�g�T�E���h�̃C���^�[�t�F�[�X
*/
class IOneshotSound
{
public:
	virtual ~IOneshotSound() = 0;
	virtual	void	Play(DWORD dwPriority, LONG lVolume) = 0;
	virtual	void	Stop() = 0;
};

