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
* @brief	コンストラクタ
* @param	[in]	pFilename	ファイル名
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
* @brief	デストラクタ
*/
COggSound::~COggSound()
{
	SAFE_RELEASE(m_pSecondary);
	SAFE_RELEASE(m_pDSB);
	ov_clear(&m_ovf);
}


/**
* @brief	更新
*/
void COggSound::Update()
{
	void* AP1 = 0, *AP2 = 0;
	DWORD AB1 = 0, AB2 = 0;
	
	m_pSecondary->GetCurrentPosition(&m_dwPoint, 0);
	if (m_uiWriteFlag == 0 && m_dwPoint >= m_uiBufSize)
	{
		//	前半に書き込み
		if (SUCCEEDED(m_pSecondary->Lock(0, m_uiBufSize, &AP1, &AB1, &AP2, &AB2, 0)))
		{
			this->GetPcmBufferFromOgg(&m_ovf, (char*)AP1, AB1, m_bLoopFlag, &m_bFinFlag);
			m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
			m_uiWriteFlag = 1;
		}
	}
	else if( m_uiWriteFlag == 1 && m_dwPoint < m_uiBufSize)
	{
		//	後半に書き込み
		if (SUCCEEDED(m_pSecondary->Lock(m_uiBufSize, m_uiBufSize << 1, &AP1, &AB1, &AP2, &AB2, 0))) {
			this->GetPcmBufferFromOgg(&m_ovf, (char*)AP1, AB1, m_bLoopFlag, &m_bFinFlag);
			m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
			m_uiWriteFlag = 0;
		}
	}
}


/**
* @brief	再生
* @param	[in]	dwPriority	優先度
* @param	[in]	dwFlag		再生フラグ
*/
void COggSound::Play(DWORD dwPriority, DWORD dwFlag)
{
	if (m_pSecondary)
		m_pSecondary->Play(0, dwPriority, dwFlag);
}


/**
* @brief	停止
*/
void COggSound::Stop()
{
	if (m_pSecondary)
		m_pSecondary->Stop();
}


/**
* @brief	終了
*/
void COggSound::Done()
{
	if (m_pSecondary) {
		m_pSecondary->Stop();
		//	読み込み位置を最初に戻す
		m_uiWriteFlag = 0;
		m_pSecondary->SetCurrentPosition(0);
		ov_time_seek(&m_ovf, 0.0);

		//	音声データクリア
		void* AP1 = 0, *AP2 = 0;
		DWORD AB1 = 0, AB2 = 0;
		m_pSecondary->Lock(0, m_uiBufSize << 1, &AP1, &AB1, &AP2, &AB2, DSBLOCK_ENTIREBUFFER);
		m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
	}
}


/**
* @brief	ループフラグの設定
* @param	[in]	bFlag	設定するフラグ
*/
void COggSound::SetLoopFlag(bool bFlag)
{
	if (m_bLoopFlag != bFlag)
		m_bLoopFlag = bFlag;
}


/**
* @brief	音量の設定
* @param	[in]	lVolume		設定する音量の値
*/
void COggSound::SetVolume(LONG lVolume)
{
	if (m_pSecondary)
		m_pSecondary->SetVolume(lVolume);
}


/**
* @brief	音量の取得
* @return	音量
*/
LONG COggSound::GetVolume()
{
	LONG lVolume = 0;

	if (m_pSecondary)
		m_pSecondary->GetVolume(&lVolume);

	return lVolume;
}



/**
* @brief	セカンダリーバッファの作成
*/
void COggSound::CreateSecondaryBuffer()
{
	IDirectSound8 *pDS8 = CDirectSound::GetDirectSound8();

	vorbis_info *info = ov_info(&m_ovf, -1);

	// WAVE情報
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = info->channels;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = info->channels * 16 / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// DirectSoundBuffer情報
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
* @brief	Oggファイルのデコード
* @param	[in]	ovf			OggVorbis_File
* @param	[in]	buffer		PcmBuffer
* @param	[in]	bufferSize	PcmBufferSize
* @param	[in]	isLoop		LoopFlag
* @param	[in]	isEnd		終了フラグ
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
		isAdjust = true;	//	調整段階
	}

	while (true)
	{
		readSize = ov_read(ovf, (char*)(buffer + comSize), requestSize, 0, 2, 1, &bitstream);
		if (readSize == 0)
		{
			//	ファイルエンドに到達した
			if (isLoop == true)
			{
				//	ループする場合、読み込み位置を最初に戻す
				ov_time_seek(ovf, 0.0);
			}
			else
			{
				//	ループしない場合、終了
				if (isEnd)
					*isEnd = true;
				return comSize;
			}
		}

		comSize += readSize;

		if (comSize >= bufferSize)
		{
			//	バッファを埋め尽くしたので終了
			return comSize;
		}

		if (bufferSize - comSize < requestSize) {
			isAdjust = true; // 調整段階
			requestSize = bufferSize - comSize;
		}

	}

	return 0;	//	謎のエラー（ここでreturnすることは基本的にない）
}
