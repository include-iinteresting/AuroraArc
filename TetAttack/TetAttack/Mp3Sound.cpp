#include "stdafx.h"
#include "Mp3Sound.h"
#include <dsound.h>
#include "DirectSound.h"



#pragma comment ( lib, "dxguid.lib" )


#define PLAY_TIME 1

/**
* @brief	コンストラクタ
*/
CMp3Sound::CMp3Sound(const char *pFilename, bool bLoopFlag)
{
	m_pFilename = pFilename;
	m_bLoopFlag = bLoopFlag;
	m_bFinFlag = false;

	m_uiWriteFlag = 0;
	m_dwPoint = 0;

	m_has = { 0 };
	m_ash = { 0 };

	m_uiBufSize = 0;
	m_lStartBlockPos = 0;

	this->DecodePrepare();
}


/**
* @brief	デストラクタ
*/
CMp3Sound::~CMp3Sound()
{
	// ACMの後始末
	acmStreamUnprepareHeader(m_has, &m_ash, 0);
	acmStreamClose(m_has, 0);

	// ファイルを閉じる
	CloseHandle(m_hFile);

	// 動的確保したデータを開放
	delete[] m_ash.pbSrc;
	delete[] m_ash.pbDst;

	SAFE_RELEASE(m_pSecondary);
	SAFE_RELEASE(m_pDSB);
}


/**
* @brief	更新
*/
void CMp3Sound::Update()
{
	void* AP1 = 0, *AP2 = 0;
	DWORD AB1 = 0, AB2 = 0;

	m_pSecondary->GetCurrentPosition(&m_dwPoint, 0);
	if (m_uiWriteFlag == 0 && m_dwPoint >= m_uiBufSize)
	{
		//	前半に書き込み
		if (SUCCEEDED(m_pSecondary->Lock(0, m_uiBufSize, &AP1, &AB1, &AP2, &AB2, 0)))
		{
			this->GetPcmBufferFromMp3(m_hFile, m_has, m_ash, (char*)AP1, m_uiBufSize, m_bLoopFlag, &m_bFinFlag);
			m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
			m_uiWriteFlag = 1;
		}
	}
	else if (m_uiWriteFlag == 1 && m_dwPoint < m_uiBufSize)
	{
		//	後半に書き込み
		if (SUCCEEDED(m_pSecondary->Lock(m_uiBufSize, m_uiBufSize << 1, &AP1, &AB1, &AP2, &AB2, 0))) {
			this->GetPcmBufferFromMp3(m_hFile, m_has, m_ash, (char*)AP1, m_uiBufSize, m_bLoopFlag, &m_bFinFlag);
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
void CMp3Sound::Play(DWORD dwPriority, DWORD dwFlag)
{
	if (m_pSecondary)
		m_pSecondary->Play(0, dwPriority, dwFlag);
}


/**
* @brief	停止
*/
void CMp3Sound::Stop()
{
	if (m_pSecondary)
		m_pSecondary->Stop();
}


/**
* @brief	終了
*/
void CMp3Sound::Done()
{
	if (m_pSecondary) 
	{

		if (m_pSecondary) {
			m_pSecondary->Stop();
			//	読み込み位置を最初に戻す
			m_uiWriteFlag = 0;
			m_pSecondary->SetCurrentPosition(0);
			SetFilePointer(m_hFile, m_lStartBlockPos, NULL, FILE_BEGIN);

			//	音声データクリア
			void* AP1 = 0, *AP2 = 0;
			DWORD AB1 = 0, AB2 = 0;
			m_pSecondary->Lock(0, m_uiBufSize << 1, &AP1, &AB1, &AP2, &AB2, DSBLOCK_ENTIREBUFFER);
			m_pSecondary->Unlock(AP1, AB1, AP2, AB2);
		}
	}
}


/**
* @brief	ループフラグの設定
* @param	[in]	bFlag	設定するフラグ
*/
void CMp3Sound::SetLoopFlag(bool bFlag)
{
	if (m_bLoopFlag != bFlag)
		m_bLoopFlag = bFlag;
}


/**
* @brief	音量の設定
* @param	[in]	lVolume		設定する音量
*/
void CMp3Sound::SetVolume(LONG lVolume)
{
	if (m_pSecondary)
		m_pSecondary->SetVolume(lVolume);
}


/**
* @brief	音量の取得
*/
LONG CMp3Sound::GetVolume()
{
	LONG lVolume = 0;

	if (m_pSecondary)
		m_pSecondary->GetVolume(&lVolume);
	
	return lVolume;
}


/**
* @brief	セカンダリバッファの生成
*/
void CMp3Sound::CreateSecondaryBuffer(WAVEFORMATEX wfx)
{
	IDirectSound8 *pDS8 = CDirectSound::GetDirectSound8();

	// DirectSoundBuffer情報
	DSBUFFERDESC DSBufferDesc;
	DSBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	DSBufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	DSBufferDesc.dwBufferBytes = wfx.nAvgBytesPerSec * PLAY_TIME;
	DSBufferDesc.dwReserved = 0;
	DSBufferDesc.lpwfxFormat = &wfx;
	DSBufferDesc.guid3DAlgorithm = GUID_NULL;
	m_uiBufSize = DSBufferDesc.dwBufferBytes >> 1;

	pDS8->CreateSoundBuffer(&DSBufferDesc, &m_pDSB, NULL);
	m_pDSB->QueryInterface(IID_IDirectSoundBuffer, (void**)&m_pSecondary);
}


/**
* @brief	デコードする準備
*/
void CMp3Sound::DecodePrepare()
{
	//!	文字列型の変換
	wchar_t *tmp = new wchar_t[strlen(m_pFilename) + 1];
	size_t len;
	mbstowcs_s(&len, tmp, strlen(m_pFilename) + 1, m_pFilename, strlen(m_pFilename) + 1);

	// ファイルを開く
	m_hFile = CreateFile(
		tmp,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return; // エラー

					  // ファイルサイズ取得
	DWORD fileSize = GetFileSize(m_hFile, NULL);


	BYTE header1[10];
	DWORD readSize1;

	// ヘッダの読み込み
	ReadFile(m_hFile, header1, 10, &readSize1, NULL);

	DWORD offset; // MP3データの位置
	DWORD size; // MP3データのサイズ

	// 先頭３バイトのチェック
	if (memcmp(header1, "ID3", 3) == 0) {
		// タグサイズを取得
		DWORD tagSize =
			((header1[6] << 21) |
			(header1[7] << 14) |
				(header1[8] << 7) |
				(header1[9])) + 10;

		// データの位置、サイズを計算
		offset = tagSize;
		size = fileSize - offset;
	}
	else {
		// 末尾のタグに移動
		BYTE tag[3];
		SetFilePointer(m_hFile, fileSize - 128, NULL, FILE_BEGIN);
		ReadFile(m_hFile, tag, 3, &readSize1, NULL);

		// データの位置、サイズを計算
		offset = 0;
		if (memcmp(tag, "TAG", 3) == 0)
			size = fileSize - 128; // 末尾のタグを省く
		else
			size = fileSize; // ファイル全体がMP3データ
	}

	// ファイルポインタをMP3データの開始位置に移動
	SetFilePointer(m_hFile, offset, NULL, FILE_BEGIN);
	
	BYTE header2[4];
	DWORD readSize2;

	ReadFile(m_hFile, header2, 4, &readSize2, NULL);

	//!<	MP3BlockDataのスタート位置を取得
	m_lStartBlockPos += offset;
	m_lStartBlockPos += readSize2;

	//!<	Check bitA FrameHeader
	if (!(header2[0] == 0xFF && (header2[1] & 0xE0) == 0xE0))
		return;

	//!<	Check bitB MPEGVersion
	BYTE version = (header2[1] >> 3) & 0x03;

	//!<	Check bitC numLayer
	BYTE layer = (header2[1] >> 1) & 0x03;

	//!<	Check bitE bitLate
	// ビットレートのテーブル
	const WORD bitRateTable[][16] = {
	// MPEG1, Layer1
	{ 0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,-1 },
	// MPEG1, Layer2
	{ 0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,-1 },
	// MPEG1, Layer3
	{ 0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,-1 },
	// MPEG2/2.5, Layer1,2
	{ 0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,-1 },
	// MPEG2/2.5, Layer3
	{ 0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,-1 }
	};

	INT index;
	if (version == 3) {
		index = 3 - layer;
	}
	else {
		if (layer == 3)
			index = 3;
		else
			index = 4;
	}

	WORD bitRate = bitRateTable[index][header2[2] >> 4];

	//!<	Check bitF SamplingFrequency	
	// サンプリングレートのテーブル
	const WORD sampleRateTable[][4] = {
	{ 44100, 48000, 32000, -1 }, // MPEG1
	{ 22050, 24000, 16000, -1 }, // MPEG2
	{ 11025, 12000, 8000, -1 } // MPEG2.5
	};

	switch (version) {
	case 0:
		index = 2;
		break;
	case 2:
		index = 1;
		break;
	case 3:
		index = 0;
		break;
	}
	WORD sampleRate =
		sampleRateTable[index][(header2[2] >> 2) & 0x03];

	//!<	Load Padding And ChannelMode
	BYTE padding = header2[2] >> 1 & 0x01;
	BYTE channel = header2[3] >> 6;


	// サイズ取得
	WORD blockSize = ((144 * bitRate * 1000) / sampleRate) + padding;

	// フォーマット取得
	MPEGLAYER3WAVEFORMAT mf;
	mf.wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
	mf.wfx.nChannels = channel == 3 ? 1 : 2;
	mf.wfx.nSamplesPerSec = sampleRate;
	mf.wfx.nAvgBytesPerSec = (bitRate * 1000) / 8;
	mf.wfx.nBlockAlign = 1;
	mf.wfx.wBitsPerSample = 0;
	mf.wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;

	mf.wID = MPEGLAYER3_ID_MPEG;
	mf.fdwFlags =
		padding ?
		MPEGLAYER3_FLAG_PADDING_ON :
		MPEGLAYER3_FLAG_PADDING_OFF;
	mf.nBlockSize = blockSize;
	mf.nFramesPerBlock = 1;
	mf.nCodecDelay = 1393;

	WAVEFORMATEX wfx = { 0 };

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	acmFormatSuggest(
		NULL,
		&mf.wfx,
		&wfx,
		sizeof(WAVEFORMATEX),
		ACM_FORMATSUGGESTF_WFORMATTAG
	);

	
	//!<	Open AcmStream
	acmStreamOpen(&m_has, NULL, &mf.wfx, &wfx, NULL, 0, 0, 0);

	//!<	MP3BlockSize Change Wave
	DWORD waveBlockSize;
	acmStreamSize(
		m_has,
		blockSize,
		&waveBlockSize,
		ACM_STREAMSIZEF_SOURCE
	);

	//!<	Set AcmStreamStruct
	m_ash.cbStruct = sizeof(ACMSTREAMHEADER);
	m_ash.pbSrc = new BYTE[blockSize];
	m_ash.cbSrcLength = blockSize;
	m_ash.pbDst = new BYTE[waveBlockSize];
	m_ash.cbDstLength = waveBlockSize;

	//!<	AcmStreamDecodePrepare
	acmStreamPrepareHeader(m_has, &m_ash, 0);

	this->CreateSecondaryBuffer(wfx);
}


/**
* @brief	MP3からPCMデータに変換する
* @param	[in]	hFile		FILEHANDLE
* @param	[in]	has			HACMSTREAM
* @param	[in]	ash			ACMSTREAMHEADER
* @param	[out]	buffer		PCMBuffer
* @param	[in]	bufferSize	バッファのサイズ
* @param	[in]	isLoop		ループフラグ
* @param	[out]	isEnd		終了フラグ
*/
unsigned int CMp3Sound::GetPcmBufferFromMp3(HANDLE hFile, HACMSTREAM has, ACMSTREAMHEADER ash, char * buffer, int bufferSize, bool isLoop, bool * isEnd)
{
	if (buffer == 0) {
		if (isEnd)
			*isEnd = true;
		return 0;
	}

	if (isEnd)
		*isEnd = false;

	memset(buffer, 0, bufferSize);
	int requestSize = ash.cbSrcLength;

	DWORD readSize = 0;
	int comSize = 0;
	bool isAdjust = false;

	if (bufferSize < requestSize)
	{
		requestSize = bufferSize;
		isAdjust = true;	//	調整段階
	}

	while (true)
	{
		ReadFile(hFile, ash.pbSrc, requestSize, &readSize, NULL);
		acmStreamConvert(has, &ash, ACM_STREAMCONVERTF_BLOCKALIGN);
		// デコードしたWAVEデータをそのまま格納
		CopyMemory(
			(LPBYTE)buffer + comSize,
			ash.pbDst,
			ash.cbDstLengthUsed
		);

		if (readSize == 0)
		{
			//	ファイルエンドに到達した
			if (isLoop == true)
			{
				//	ループする場合、読み込み位置を最初に戻す
				SetFilePointer(hFile, m_lStartBlockPos, NULL, FILE_BEGIN);
			}
			else
			{
				//	ループしない場合、終了
				if (isEnd)
					*isEnd = true;
				return comSize;
			}
		}

		comSize += ash.cbDstLengthUsed;

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
