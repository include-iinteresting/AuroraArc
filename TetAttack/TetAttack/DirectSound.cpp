#include "stdafx.h"
#include <dsound.h>
#include "DirectSound.h"

#pragma comment(lib, "dsound.lib")


/**
* @class	DirectSoundImpl
* @brief	DirectSound�N���X�̎���
*/
class DirectSoundImpl 
{
	friend CDirectSound;
private:
	DirectSoundImpl();
	~DirectSoundImpl();

	static	void	Initialize();
	static	void	Finalize();
	static	DirectSoundImpl	*GetInstance();

	HRESULT InitDirectSound(HWND hWnd);
	HRESULT	CreateDirectSound(HWND hWnd);
	HRESULT	CreatePrimaryBuffer(IDirectSound8 *pDS8);

	IDirectSound8		*GetDirectSound8();
	IDirectSoundBuffer	*GetPrimaryBuffer();
private:
	static	DirectSoundImpl	*m_pInstance;
	IDirectSound8			*m_pDS8;
	IDirectSoundBuffer		*m_pPrimary;
};

DirectSoundImpl	*DirectSoundImpl::m_pInstance = NULL;

/**
* @brief	�R���X�g���N�^
*/
DirectSoundImpl::DirectSoundImpl()
{
	m_pDS8 = NULL;
	m_pPrimary = NULL;
}


/**
* @brief	�f�X�g���N�^
*/
DirectSoundImpl::~DirectSoundImpl()
{
	SAFE_RELEASE(m_pPrimary);
	SAFE_RELEASE(m_pDS8);
}


/**
* @brief	������
*/
void DirectSoundImpl::Initialize()
{
	if (m_pInstance)
		return;

	m_pInstance = new DirectSoundImpl();
}


/**
* @brief	�I������
*/
void DirectSoundImpl::Finalize()
{
	SAFE_DELETE(m_pInstance);
}


/**
* @brief	�C���X�^���X�擾
* @return	�C���X�^���X
*/
DirectSoundImpl * DirectSoundImpl::GetInstance()
{
	if (!m_pInstance)
		DirectSoundImpl::Initialize();

	return m_pInstance;
}



/**
* @brief	DirectSound�̏�����
* @param	[in]	hWnd	WindowHandle
*/
HRESULT DirectSoundImpl::InitDirectSound(HWND hWnd)
{
	HRESULT hr = S_OK;

	hr = DirectSoundImpl::CreateDirectSound(hWnd);

	hr = DirectSoundImpl::CreatePrimaryBuffer(m_pDS8);

	return hr;
}

/**
* @brief	DirectSound�̐���
* @param	[in]	hWnd	WindowHandle
* @return	HRESULT
*/
HRESULT DirectSoundImpl::CreateDirectSound(HWND hWnd)
{
	HRESULT hr = S_OK;
	hr = DirectSoundCreate8(NULL, &m_pDS8, NULL);
	if (FAILED(hr))
		goto ERROR_EXIT;

	hr = m_pDS8->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE | DSSCL_PRIORITY);
	if (FAILED(hr))
		goto ERROR_EXIT;

ERROR_EXIT:
	return hr;
}


/**
* @brief	�v���C�}���[�o�b�t�@�̐���
* @param	[in]	pDS8	IDirectSound8
* @return	HRESULT
*/
HRESULT DirectSoundImpl::CreatePrimaryBuffer(IDirectSound8 *pDS8)
{
	HRESULT hr = S_OK;

	WAVEFORMATEX wfx;

	//	�v���C�}���T�E���h�o�b�t�@�̍쐬
	DSBUFFERDESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(dsdesc));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);
	dsdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsdesc.dwBufferBytes = 0;
	dsdesc.lpwfxFormat = NULL;
	hr = pDS8->CreateSoundBuffer(&dsdesc, &m_pPrimary, NULL);
	if (FAILED(hr))
		goto EXIT;

	// �v���C�}���o�b�t�@�̃X�e�[�^�X������
	wfx.cbSize = sizeof(WAVEFORMATEX);
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	hr = m_pPrimary->SetFormat(&wfx);
	if (FAILED(hr))
		goto EXIT;

EXIT:
	return hr;
}



/**
* @brief	DirectSound8�̎擾
*/
IDirectSound8 * DirectSoundImpl::GetDirectSound8()
{
	return m_pDS8;
}


/**
* @brief	�v���C�}���[�o�b�t�@�̎擾
*/
IDirectSoundBuffer * DirectSoundImpl::GetPrimaryBuffer()
{
	return m_pPrimary;
}


/**
* @brief	������
* @param	[in]	hWnd	WindowHandle
*/
void CDirectSound::Initialize(HWND hWnd)
{
	DirectSoundImpl *pObj = DirectSoundImpl::GetInstance();

	pObj->InitDirectSound(hWnd);
}


/**
* @brief	�I������
*/
void CDirectSound::Finalize()
{
	DirectSoundImpl::Finalize();
}


/**
* @brief	IDirectSound8�̎擾
*/
IDirectSound8 * CDirectSound::GetDirectSound8()
{
	DirectSoundImpl *pObj = DirectSoundImpl::GetInstance();

	return pObj->GetDirectSound8();
}


/**
* @brief	DirectSoundBuffer�̎擾
*/
IDirectSoundBuffer * CDirectSound::GetPrimaryBuffer()
{
	DirectSoundImpl *pObj = DirectSoundImpl::GetInstance();

	return pObj->GetPrimaryBuffer();
}
