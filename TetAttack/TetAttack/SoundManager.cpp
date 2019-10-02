#include "stdafx.h"
#include "SoundManager.h"
#include "DirectSound.h"
#include "OggSound.h"
#include "Mp3Sound.h"
#include "WaveSound.h"
#include <list>
#include <process.h>

//	macro
#undef SAFE_RELEASE
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY
#define SAFE_RELEASE(o)	if(o) { (o)->Release(); o = NULL; };
#define SAFE_DELETE(o) if(o) { delete(o); o = NULL; };
#define SAFE_DELETE_ARRAY(o) if(o) { delete[](o); o = NULL; };

//	ThreadPhase
enum ThreadPhase : short {
	THREAD_INIT = 0x00,
	THREAD_RUN = 0x01,
	THREAD_WAIT = 0x02,
	THREAD_DONE = 0x03
};


//	SoundCommand
enum SoundCommand
{
	NO_COMMAND = 0x00,
	SOUND_PLAY = 0x01,
	SOUND_STOP = 0x02,
	SOUND_DONE = 0x03
};

#define QUEUE_SIZE 100
#define QUEUE_NEXT(o) (((o) + 1) % QUEUE_SIZE)
//	RingBuffer
typedef struct {
	SoundCommand Command[QUEUE_SIZE];
	unsigned int numSound[QUEUE_SIZE];
	DWORD dwPriority[QUEUE_SIZE];
	DWORD dwFlag[QUEUE_SIZE];
	short head;
	short tail;
}CommandQueue;


/**
* @class	SoundManagerImpl
* @brief	サウンド管理クラスの中身
*/
class SoundManagerImpl
{
	friend CSoundManager;
private:
	SoundManagerImpl();
	~SoundManagerImpl();

	static	void	Initialize();
	static	void	Finalize();
	static	SoundManagerImpl	*GetInstance();
	
	//	OneshotSound
	void	LoadOneshotSound(const char* pFilename);
	void	UnloadOneshotSound(unsigned int numSound);
	void	OneshotPlay(unsigned int numSound, DWORD dwPriority, LONG lVolume);
	void	OneshotStop(unsigned int numSound);


	//	StreamSound
	void	LoadStreamingSound(const char* pFilename, bool bLoopFlag);
	void	UnloadStreamingSound(unsigned int numSound);
	void	StreamPlay(unsigned int numSound, DWORD dwPriority, DWORD dwFlag);
	void	StreamStop(unsigned int numSound);
	void	StreamDone(unsigned int numSound);
	void	Command();
	void	StreamPlayCommand(unsigned int numSound, DWORD dwPriority, DWORD dwFlag);
	void	StreamStopCommand(unsigned int numSound);
	void	StreamDoneCommand(unsigned int numSound);

	//	StreamThread
	void	Lock();
	void	Unlock();
	static	void	ThreadProcLauncher(void* arg);
	void			ThreadProc();
	
	//	RingBuffer
	void			EnQueue(unsigned int numSound, SoundCommand Command, DWORD dwPriority, DWORD dwFlag);
	void			DeQueue(unsigned int *pNumSound, SoundCommand *pCommand, DWORD *pPriority, DWORD *pFlag);
private:
	static	SoundManagerImpl	*m_pInstance;
	std::list<IStreamingSound*>	m_pStreamSounds;
	std::list<IOneshotSound*>	m_pOneshotSounds;

	CRITICAL_SECTION	m_CriticalSection;

	volatile	CommandQueue	m_Queue;			//!	CommandQueueRingBuffer

	volatile	ThreadPhase		m_ePhase;
	volatile struct
	{
		volatile	BOOL m_bThreadActive;
		volatile	BOOL m_bThreadDone;
		volatile	BOOL m_bStopCommand;
	}ThreadFlags;
};

SoundManagerImpl	*SoundManagerImpl::m_pInstance = NULL;

/**
* @brief	コンストラクタ
*/
SoundManagerImpl::SoundManagerImpl()
{
	InitializeCriticalSection(&m_CriticalSection);
	
	m_ePhase = ThreadPhase::THREAD_INIT;
	ThreadFlags.m_bThreadActive = true;
	ThreadFlags.m_bStopCommand = false;
	ThreadFlags.m_bThreadDone = false;

	m_pOneshotSounds.clear();

	_beginthread(ThreadProcLauncher, 0, this);
}


/**
* @brief	デストラクタ
*/
SoundManagerImpl::~SoundManagerImpl()
{
	for (std::list<IOneshotSound*>::iterator it = m_pOneshotSounds.begin(); it != m_pOneshotSounds.end();) {
		SAFE_DELETE(*it);
		it = m_pOneshotSounds.erase(it);
	}

	while (!ThreadFlags.m_bThreadDone) {
		if (!ThreadFlags.m_bStopCommand) {
			Lock();
			ThreadFlags.m_bStopCommand = true;
			Unlock();
		}
	}
	DeleteCriticalSection(&m_CriticalSection);
}


/**
* @brief	初期化
*/
void SoundManagerImpl::Initialize()
{
	if (m_pInstance)
		return;

	m_pInstance = new SoundManagerImpl();
}


/**
* @brief	終了処理
*/
void SoundManagerImpl::Finalize()
{
	SAFE_DELETE(m_pInstance);
}


/**
* @brief	インスタンス取得
*/
SoundManagerImpl * SoundManagerImpl::GetInstance()
{
	if (!m_pInstance)
		SoundManagerImpl::Initialize();

	return m_pInstance;
}


/**
* @brief	ワンショット再生する音声ファイルを読み込む
* @param	[in]	pFilename	ファイル名
*/
void SoundManagerImpl::LoadOneshotSound(const char * pFilename)
{
	m_pOneshotSounds.push_back(new CWaveSound(pFilename));
}


/**
* @brief	ワンショット再生する音声ファイルの読み込みを破棄する
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void SoundManagerImpl::UnloadOneshotSound(unsigned int numSound)
{
	if (numSound < m_pOneshotSounds.size()) {
		std::list<IOneshotSound*>::iterator it = m_pOneshotSounds.begin();
		for (unsigned int i = 0; i < numSound; ++i) {
			it++;
		}
		SAFE_DELETE(*it);
		it = m_pOneshotSounds.erase(it);
	}
}


/**
* @brief	ワンショットサウンドの再生
* @param	[in]	numSound	サウンドオブジェクトの番号
* @param	[in]	dwPriority	サウンドの優先度
* @param	[in]	lVolume		サウンドの音量（-10000～0）
*/
void SoundManagerImpl::OneshotPlay(unsigned int numSound, DWORD dwPriority, LONG lVolume)
{
	if (numSound < m_pOneshotSounds.size()) {
		std::list<IOneshotSound*>::iterator it = m_pOneshotSounds.begin();
		for (unsigned int i = 0; i < numSound; ++i) {
			it++;
		}
		(*it)->Play(dwPriority, lVolume);
	}
}


/**
* @brief	ワンショットサウンドの停止
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void SoundManagerImpl::OneshotStop(unsigned int numSound)
{
	if (numSound < m_pOneshotSounds.size()) {
		std::list<IOneshotSound*>::iterator it = m_pOneshotSounds.begin();
		for (unsigned int i = 0; i < numSound; ++i) {
			it++;
		}
		(*it)->Stop();
	}
}

//!	対応拡張子の数
#define numExtension 2
//!	対応拡張子
static const char* extension[numExtension] =
{
	"ogg",
	"mp3"
};

/**
* @brief	ストリーミング再生する音声ファイルを読み込む
* @param	[in]	pFilename	ファイル名
* @param	[in]	bLoopFlag	ループフラグ
*/
void SoundManagerImpl::LoadStreamingSound(const char * pFilename, bool bLoopFlag)
{
	//!<	拡張子の取得
	char tmp[3];
	int len = strlen(pFilename);
	for (int i = 0; i < 3; ++i)
	{
		tmp[i] = pFilename[len - (3 - i)];
	}

	//!<	拡張子に応じた番号の取得
	int iExtensionNumber = -1;
	for (int i = 0; i < numExtension; ++i)
	{
		if (strstr(tmp, extension[i]) != NULL)
		{
			iExtensionNumber = i;
			break;
		}
	}
	//!<	拡張子に応じたオブジェクトの生成
	switch (iExtensionNumber)
	{
	case 0:
		Lock();
		m_pStreamSounds.push_back(new COggSound(pFilename, bLoopFlag));
		Unlock();
		break;
	case 1:
		Lock();
		m_pStreamSounds.push_back(new CMp3Sound(pFilename, bLoopFlag));
		Unlock();
		break;
	default:
		break;
	}
	
	
}


/**
* @brief	ストリーミング再生する音声ファイルの読み込みを解除する
* @param	[in]	numSound	サウンドオブジェクトの番号
* @details	サウンドオブジェクトの番号が詰められることに注意
*/
void SoundManagerImpl::UnloadStreamingSound(unsigned int numSound)
{
	Lock();
	std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin();
	for (unsigned int i = 0; i < numSound; ++i) {
		++it;
	}
	(*it)->Stop();
	SAFE_DELETE(*it);
	it = m_pStreamSounds.erase(it);
	Unlock();
}


/**
* @brief	再生
* @param	[in]	numSound	サウンドオブジェクトの番号
* @param	[in]	dwPriority	優先度
* @param	[in]	dwFlag		再生フラグ
*/
void SoundManagerImpl::StreamPlay(unsigned int numSound, DWORD dwPriority, DWORD dwFlag)
{
	Lock();
	std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin();
	for (unsigned int i = 0; i < numSound; ++i) {
		++it;
	}
	(*it)->Play(dwPriority, dwFlag);
	Unlock();
}


/**
* @brief	停止
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void SoundManagerImpl::StreamStop(unsigned int numSound)
{
	Lock();
	std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin();
	for (unsigned int i = 0; i < numSound; ++i) {
		++it;
	}
	(*it)->Stop();
	Unlock();
}


/**
* @brief	終了
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void SoundManagerImpl::StreamDone(unsigned int numSound)
{
	Lock();
	std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin();
	for (unsigned int i = 0; i < numSound; ++i) {
		++it;
	}
	(*it)->Done();
	Unlock();
}


/**
* @brief	命令に応じた処理を呼ぶ
*/
void SoundManagerImpl::Command()
{
	SoundCommand command;	//!	Command
	unsigned int numSound;	//!	SoundObjectNumber
	DWORD dwPriority;       //! 優先度（再生用）
	DWORD dwFlag;			//!	再生フラグ（再生用） 
	//!	キューからデータの取り出し
	DeQueue(&numSound, &command, &dwPriority, &dwFlag);

	//	命令に応じた処理
	switch (command) {
	case SOUND_PLAY:
		this->StreamPlay(numSound, dwPriority, dwFlag);
		break;
	case SOUND_STOP:
		this->StreamStop(numSound);
		break;
	case SOUND_DONE:
		this->StreamDone(numSound);
		break;
	default:
		break;
	}
}


/**
* @brief	再生命令
* @param	[in]	numSound	サウンドオブジェクトの番号
* @param	[in]	dwPriority	優先度
* @param	[in]	dwFlag		再生フラグ
*/
void SoundManagerImpl::StreamPlayCommand(unsigned int numSound, DWORD dwPriority, DWORD dwFlag)
{
	EnQueue(numSound, SOUND_PLAY, dwPriority, dwFlag);
}


/**
* @brief	停止命令
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void SoundManagerImpl::StreamStopCommand(unsigned int numSound)
{
	EnQueue(numSound, SOUND_STOP, 0, 0);
}


/**
* @brief	終了命令
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void SoundManagerImpl::StreamDoneCommand(unsigned int numSound)
{
	EnQueue(numSound, SOUND_DONE, 0, 0);
}


/**
* @brief	ロック(排他制御)
*/
void SoundManagerImpl::Lock()
{
	EnterCriticalSection(&m_CriticalSection);
}


/**
* @brief	アンロック(排他制御)
*/
void SoundManagerImpl::Unlock()
{
	LeaveCriticalSection(&m_CriticalSection);
}


/**
* @brief	_beginthreadでThreadProcメソッドを使うためのラッパー
*/
void SoundManagerImpl::ThreadProcLauncher(void * arg)
{
	if (!arg)
		return;

	reinterpret_cast<SoundManagerImpl*>(arg)->ThreadProc();
}


/**
* @brief	別スレッドで実際に呼び出されるメソッド
*/
void SoundManagerImpl::ThreadProc()
{
	while (ThreadFlags.m_bThreadActive) {
		switch (m_ePhase)
		{
		case THREAD_INIT:	//	初期化
			m_pStreamSounds.clear();
			m_ePhase = THREAD_RUN;
		case THREAD_RUN:	//	更新

			for (std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin(); it != m_pStreamSounds.end(); ++it) {
				(*it)->Update();
			}

			if (m_pStreamSounds.size() > 0)
				Command();

			if (ThreadFlags.m_bStopCommand) {
				m_ePhase = THREAD_WAIT;
			}

			break;
		case THREAD_WAIT:	//	終了待ち
			Lock();
			for (std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin(); it != m_pStreamSounds.end();) {
				SAFE_DELETE(*it);
				it = m_pStreamSounds.erase(it);
			}
			Unlock();
			m_ePhase = THREAD_DONE;

			break;
		case THREAD_DONE:	//	終了処理
			ThreadFlags.m_bThreadActive = false;
			break;
		}
		Sleep(16);
	}

	ThreadFlags.m_bThreadDone = true;
}


/**
* @brief	キューにデータを追加する
* @param	[in]	numSound	サウンドオブジェクトの番号
* @param	[in]	Command		追加する命令
* @param	[in]	dwPriority	優先度（再生用）
* @param	[in]	dwFlag		再生フラグ（再生用）
*/
void SoundManagerImpl::EnQueue(unsigned int numSound, SoundCommand Command, DWORD dwPriority, DWORD dwFlag)
{
	//	キューが満杯かどうかの確認
	if (QUEUE_NEXT(m_Queue.tail) == m_Queue.head)
		return;

	//	キューの末尾にデータを挿入する
	m_Queue.Command[m_Queue.tail] = Command;
	m_Queue.numSound[m_Queue.tail] = numSound;
	m_Queue.dwPriority[m_Queue.tail] = dwPriority;
	m_Queue.dwFlag[m_Queue.tail] = dwFlag;

	//	キューの次回挿入位置を決定する
	m_Queue.tail = QUEUE_NEXT(m_Queue.tail);
}


/**
* @brief	キューのデータを取り出す
* @param	[out]	pNumSound	サウンドオブジェクトの番号
* @param	[out]	pCommand	命令
* @param	[out]	pPriority	優先度（再生用）
* @param	[out]	pFlag		再生フラグ（再生用）
*/
void SoundManagerImpl::DeQueue(unsigned int * pNumSound, SoundCommand * pCommand, DWORD * pPriority, DWORD * pFlag)
{
	//	キューに取り出すデータが存在するかを確認する
	if (m_Queue.head == m_Queue.tail)
		return;

	//	キューからデータを取得する
	*pCommand = m_Queue.Command[m_Queue.head];
	*pNumSound = m_Queue.numSound[m_Queue.head];
	*pPriority = m_Queue.dwPriority[m_Queue.head];
	*pFlag = m_Queue.dwFlag[m_Queue.head];

	//	次のデータ取得位置を決定する
	m_Queue.head = QUEUE_NEXT(m_Queue.head);
}



/*****************************
* @class	CSoundManager
*****************************/

/**
* @brief	初期化
*/
void	CSoundManager::Initialize(HWND hWnd)
{
	CDirectSound::Initialize(hWnd);

	SoundManagerImpl::Initialize();
}


/**
* @brief	終了処理
*/
void	CSoundManager::Finalize()
{
	SoundManagerImpl::Finalize();

	CDirectSound::Finalize();
}


/**
* @brief	ストリーミング再生する音声を読み込む
* @param	[in]	pFilename	ファイル名
* @param	[in]	bLoopFlag	ループフラグ
*/
void CSoundManager::LoadStreamingSound(const char * pFilename, bool bLoopFlag)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->LoadStreamingSound(pFilename, bLoopFlag);
}


/**
* @brief	ストリーミング再生する音声の読み込みを解除する
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void CSoundManager::UnloadStreamingSound(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->UnloadStreamingSound(numSound);
}


/**
* @brief	再生
* @param	[in]	numSound	サウンドオブジェクトの番号
* @param	[in]	dwPriority	優先度
* @param	[in]	dwFlag		再生フラグ
*/
void CSoundManager::StreamPlay(unsigned int numSound, DWORD dwPriority, DWORD dwFlag)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->StreamPlayCommand(numSound, dwPriority, dwFlag);
}


/**
* @brief	停止
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void CSoundManager::StreamStop(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->StreamStopCommand(numSound);
}


/**
* @brief	終了
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void CSoundManager::StreamDone(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->StreamDoneCommand(numSound);
}


/**
* @brief	ワンショットサウンドの読み込み
* @param	[in]	pFilename	ファイル名
*/
void CSoundManager::LoadOneshotSound(const char * pFilename)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->LoadOneshotSound(pFilename);
}


/**
* @brief	ワンショットサウンドの読み込み破棄
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void CSoundManager::UnloadOneshotSound(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->UnloadOneshotSound(numSound);
}


/**
* @brief	ワンショットサウンドの再生
* @param	[in]	numSound	サウンドオブジェクトの番号
* @param	[in]	dwPriority	サウンドの優先度
* @param	[in]	lVolume		サウンドの音量（-10000～0）
*/
void CSoundManager::OneshotPlay(unsigned int numSound, DWORD dwPriority, LONG lVolume)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->OneshotPlay(numSound, dwPriority, lVolume);
}


/**
* @brief	ワンショッサウンドの停止
* @param	[in]	numSound	サウンドオブジェクトの番号
*/
void CSoundManager::OneshotStop(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->OneshotStop(numSound);
}
