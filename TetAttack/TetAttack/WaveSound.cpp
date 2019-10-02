#include "stdafx.h"
#include "WaveSound.h"
#include <dsound.h>
#include "DirectSound.h"
#include "SDKwavefile.h"

/**
* @brief	�R���X�g���N�^
* @param	[in]	pFilename	�t�@�C����
*/
CWaveSound::CWaveSound(const char* pFilename)
{
	m_pFilename = pFilename;
	m_pDSB = NULL;
	m_pSecondary = NULL;

	m_wfx = { 0 };

	m_dwSize = 0;

	this->LoadPcmData(pFilename);
	this->CreateSecondaryBuffer();
}


/**
* @brief	�f�X�g���N�^
*/
CWaveSound::~CWaveSound()
{
	SAFE_RELEASE(m_pSecondary);
	SAFE_RELEASE(m_pDSB);

	SAFE_DELETE_ARRAY(m_pBuffer);
	SAFE_DELETE(m_pBuffer);
}


/**
* @brief	�Đ�
* @param	[in]	dwPriority	�D��x
* @param	[in]	lVol		���ʁi-10000�`0�j
*/
void CWaveSound::Play(DWORD dwPriority, LONG lVol)
{
	if (m_pSecondary) {
		m_pSecondary->SetVolume(lVol);
		m_pSecondary->SetCurrentPosition(0);
		void *AP1 = 0, *AP2 = 0;
		DWORD AB1 = 0, AB2 = 0;
		m_pSecondary->Lock(0, m_dwSize, &AP1, &AB1, &AP2, &AB2, 0);
		AP1 = m_pBuffer;
		m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
		m_pSecondary->Play(0, dwPriority, 0);
	}
}


/**
* @brief	��~
*/
void CWaveSound::Stop()
{
	if (m_pSecondary)
		m_pSecondary->Stop();
}



/**
* @brief	PcmData�̓ǂݍ���
* @param	[in]	pFilename	�t�@�C����
*/
void CWaveSound::LoadPcmData(const char* pFilename)
{
	DWORD dwSize = 0;
	DWORD read = 0;
	
	CWaveFile	*pWave = new CWaveFile();

	//	WaveFileOpen
	pWave->Open((LPWSTR)pFilename, &m_wfx, WAVEFILE_READ);

	//	GetWaveFormat
	m_wfx = *pWave->GetFormat();

	//	GetWaveFileSize
	dwSize = pWave->GetSize();

	//	new Buffer to WaveFileSize
	m_pBuffer = new BYTE[dwSize];

	//	ReadWaveFile
	pWave->Read(m_pBuffer, dwSize, &read);

	//	CloseWaveFile
	pWave->Close();
}


/**
* @brief	�Z�J���_���o�b�t�@�̐���
*/
void CWaveSound::CreateSecondaryBuffer()
{
	IDirectSound8 *pDS8 = CDirectSound::GetDirectSound8();

	// �T�E���h�o�b�t�@�̍쐬
	DSBUFFERDESC DSBufferDesc;
	ZeroMemory(&DSBufferDesc, sizeof(DSBUFFERDESC));
	DSBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	DSBufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	DSBufferDesc.dwBufferBytes = sizeof(m_pBuffer);
	DSBufferDesc.lpwfxFormat = &m_wfx;
	DSBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
	pDS8->CreateSoundBuffer(&DSBufferDesc, &m_pDSB, NULL);
	m_pDSB->QueryInterface(IID_IDirectSoundBuffer, (void**)&m_pSecondary);
	m_dwSize = DSBufferDesc.dwSize;
}

