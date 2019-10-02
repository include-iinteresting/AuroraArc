#include "stdafx.h"
#include "OggSound.h"
#include "DirectSound.h"
#include <dsound.h>

#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")


#pragma comment ( lib, "dxguid.lib" )

#define PLAY_TIME 1

/**
* @brief	�R���X�g���N�^
* @param	[in]	pFilename	�t�@�C����
*/
COggSound::COggSound(const char *pFilename, bool bLoopFlag)
{
	m_pFilename = pFilename;
	m_pDSB = NULL;
	m_pSecondary = NULL;

	m_uiWriteFlag = 0;
	m_dwPoint = 0;

	ov_fopen(m_pFilename, &m_ovf);
	
	CreateSecondaryBuffer();
	
	m_bLoopFlag = bLoopFlag;
	m_bFinFlag = false;
}


/**
* @brief	�f�X�g���N�^
*/
COggSound::~COggSound()
{
	SAFE_RELEASE(m_pSecondary);
	SAFE_RELEASE(m_pDSB);
	ov_clear(&m_ovf);
}


/**
* @brief	�X�V
*/
void COggSound::Update()
{
	void* AP1 = 0, *AP2 = 0;
	DWORD AB1 = 0, AB2 = 0;
	
	m_pSecondary->GetCurrentPosition(&m_dwPoint, 0);
	if (m_uiWriteFlag == 0 && m_dwPoint >= m_uiBufSize)
	{
		//	�O���ɏ�������
		if (SUCCEEDED(m_pSecondary->Lock(0, m_uiBufSize, &AP1, &AB1, &AP2, &AB2, 0)))
		{
			this->GetPcmBufferFromOgg(&m_ovf, (char*)AP1, AB1, m_bLoopFlag, &m_bFinFlag);
			m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
			m_uiWriteFlag = 1;
		}
	}
	else if( m_uiWriteFlag == 1 && m_dwPoint < m_uiBufSize)
	{
		//	�㔼�ɏ�������
		if (SUCCEEDED(m_pSecondary->Lock(m_uiBufSize, m_uiBufSize << 1, &AP1, &AB1, &AP2, &AB2, 0))) {
			this->GetPcmBufferFromOgg(&m_ovf, (char*)AP1, AB1, m_bLoopFlag, &m_bFinFlag);
			m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
			m_uiWriteFlag = 0;
		}
	}
}


/**
* @brief	�Đ�
* @param	[in]	dwPriority	�D��x
* @param	[in]	dwFlag		�Đ��t���O
*/
void COggSound::Play(DWORD dwPriority, DWORD dwFlag)
{
	if (m_pSecondary)
		m_pSecondary->Play(0, dwPriority, dwFlag);
}


/**
* @brief	��~
*/
void COggSound::Stop()
{
	if (m_pSecondary)
		m_pSecondary->Stop();
}


/**
* @brief	�I��
*/
void COggSound::Done()
{
	if (m_pSecondary) {
		m_pSecondary->Stop();
		//	�ǂݍ��݈ʒu���ŏ��ɖ߂�
		m_uiWriteFlag = 0;
		m_pSecondary->SetCurrentPosition(0);
		ov_time_seek(&m_ovf, 0.0);

		//	�����f�[�^�N���A
		void* AP1 = 0, *AP2 = 0;
		DWORD AB1 = 0, AB2 = 0;
		m_pSecondary->Lock(0, m_uiBufSize << 1, &AP1, &AB1, &AP2, &AB2, DSBLOCK_ENTIREBUFFER);
		m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
	}
}


/**
* @brief	���[�v�t���O�̐ݒ�
* @param	[in]	bFlag	�ݒ肷��t���O
*/
void COggSound::SetLoopFlag(bool bFlag)
{
	if (m_bLoopFlag != bFlag)
		m_bLoopFlag = bFlag;
}


/**
* @brief	���ʂ̐ݒ�
* @param	[in]	lVolume		�ݒ肷�鉹�ʂ̒l
*/
void COggSound::SetVolume(LONG lVolume)
{
	if (m_pSecondary)
		m_pSecondary->SetVolume(lVolume);
}


/**
* @brief	���ʂ̎擾
* @return	����
*/
LONG COggSound::GetVolume()
{
	LONG lVolume = 0;

	if (m_pSecondary)
		m_pSecondary->GetVolume(&lVolume);

	return lVolume;
}



/**
* @brief	�Z�J���_���[�o�b�t�@�̍쐬
*/
void COggSound::CreateSecondaryBuffer()
{
	IDirectSound8 *pDS8 = CDirectSound::GetDirectSound8();

	vorbis_info *info = ov_info(&m_ovf, -1);

	// WAVE���
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = info->channels;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = info->channels * 16 / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// DirectSoundBuffer���
	DSBUFFERDESC DSBufferDesc;
	DSBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	DSBufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	DSBufferDesc.dwBufferBytes = waveFormat.nAvgBytesPerSec * PLAY_TIME;
	DSBufferDesc.dwReserved = 0;
	DSBufferDesc.lpwfxFormat = &waveFormat;
	DSBufferDesc.guid3DAlgorithm = GUID_NULL;
	m_uiBufSize = DSBufferDesc.dwBufferBytes >> 1;

	pDS8->CreateSoundBuffer(&DSBufferDesc, &m_pDSB, NULL);
	m_pDSB->QueryInterface(IID_IDirectSoundBuffer, (void**)&m_pSecondary);
}


/**
* @brief	Ogg�t�@�C���̃f�R�[�h
* @param	[in]	ovf			OggVorbis_File
* @param	[in]	buffer		PcmBuffer
* @param	[in]	bufferSize	PcmBufferSize
* @param	[in]	isLoop		LoopFlag
* @param	[in]	isEnd		�I���t���O
*/
unsigned int COggSound::GetPcmBufferFromOgg(OggVorbis_File *ovf, char* buffer, int bufferSize, bool isLoop, bool* isEnd)
{
	if (buffer == 0) {
		if (isEnd)
			*isEnd = true;
		return 0;
	}

	if (isEnd) 
		*isEnd = false;

	memset(buffer, 0, bufferSize);
	int requestSize = 4096;
	int bitstream = 0;
	int readSize = 0;
	int comSize = 0;
	bool isAdjust = false;

	if (bufferSize < requestSize)
	{
		requestSize = bufferSize;
		isAdjust = true;	//	�����i�K
	}

	while (true)
	{
		readSize = ov_read(ovf, (char*)(buffer + comSize), requestSize, 0, 2, 1, &bitstream);
		if (readSize == 0)
		{
			//	�t�@�C���G���h�ɓ��B����
			if (isLoop == true)
			{
				//	���[�v����ꍇ�A�ǂݍ��݈ʒu���ŏ��ɖ߂�
				ov_time_seek(ovf, 0.0);
			}
			else
			{
				//	���[�v���Ȃ��ꍇ�A�I��
				if (isEnd)
					*isEnd = true;
				return comSize;
			}
		}

		comSize += readSize;

		if (comSize >= bufferSize)
		{
			//	�o�b�t�@�𖄂ߐs�������̂ŏI��
			return comSize;
		}

		if (bufferSize - comSize < requestSize) {
			isAdjust = true; // �����i�K
			requestSize = bufferSize - comSize;
		}

	}

	return 0;	//	��̃G���[�i������return���邱�Ƃ͊�{�I�ɂȂ��j
}
