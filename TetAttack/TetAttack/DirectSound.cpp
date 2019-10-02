#include "stdafx.h"
#include <dsound.h>
#include "DirectSound.h"

#pragma comment(lib, "dsound.lib")


/**
* @class	DirectSoundImpl
* @brief	DirectSoundクラスの実装
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
* @brief	コンストラクタ
*/
DirectSoundImpl::DirectSoundImpl()
{
	m_pDS8 = NULL;
	m_pPrimary = NULL;
}


/**
* @brief	デストラクタ
*/
DirectSoundImpl::~DirectSoundImpl()
{
	SAFE_RELEASE(m_pPrimary);
	SAFE_RELEASE(m_pDS8);
}


/**
* @brief	初期化
*/
void DirectSoundImpl::Initialize()
{
	if (m_pInstance)
		return;

	m_pInstance = new DirectSoundImpl();
}


/**
* @brief	終了処理
*/
void DirectSoundImpl::Finalize()
{
	SAFE_DELETE(m_pInstance);
}


/**
* @brief	インスタンス取得
* @return	インスタンス
*/
DirectSoundImpl * DirectSoundImpl::GetInstance()
{
	if (!m_pInstance)
		DirectSoundImpl::Initialize();

	return m_pInstance;
}



/**
* @brief	DirectSoundの初期化
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
* @brief	DirectSoundの生成
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
* @brief	プライマリーバッファの生成
* @param	[in]	pDS8	IDirectSound8
* @return	HRESULT
*/
HRESULT DirectSoundImpl::CreatePrimaryBuffer(IDirectSound8 *pDS8)
{
	HRESULT hr = S_OK;

	WAVEFORMATEX wfx;

	//	プライマリサウンドバッファの作成
	DSBUFFERDESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(dsdesc));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);
	dsdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsdesc.dwBufferBytes = 0;
	dsdesc.lpwfxFormat = NULL;
	hr = pDS8->CreateSoundBuffer(&dsdesc, &m_pPrimary, NULL);
	if (FAILED(hr))
		goto EXIT;

	// プライマリバッファのステータスを決定
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
* @brief	DirectSound8の取得
*/
IDirectSound8 * DirectSoundImpl::GetDirectSound8()
{
	return m_pDS8;
}


/**
* @brief	プライマリーバッファの取得
*/
IDirectSoundBuffer * DirectSoundImpl::GetPrimaryBuffer()
{
	return m_pPrimary;
}


/**
* @brief	初期化
* @param	[in]	hWnd	WindowHandle
*/
void CDirectSound::Initialize(HWND hWnd)
{
	DirectSoundImpl *pObj = DirectSoundImpl::GetInstance();

	pObj->InitDirectSound(hWnd);
}


/**
* @brief	終了処理
*/
void CDirectSound::Finalize()
{
	DirectSoundImpl::Finalize();
}


/**
* @brief	IDirectSound8の取得
*/
IDirectSound8 * CDirectSound::GetDirectSound8()
{
	DirectSoundImpl *pObj = DirectSoundImpl::GetInstance();

	return pObj->GetDirectSound8();
}


/**
* @brief	DirectSoundBufferの取得
*/
IDirectSoundBuffer * CDirectSound::GetPrimaryBuffer()
{
	DirectSoundImpl *pObj = DirectSoundImpl::GetInstance();

	return pObj->GetPrimaryBuffer();
}
