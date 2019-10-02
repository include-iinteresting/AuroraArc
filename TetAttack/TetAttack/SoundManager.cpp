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
* @brief	�T�E���h�Ǘ��N���X�̒��g
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
* @brief	�R���X�g���N�^
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
* @brief	�f�X�g���N�^
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
* @brief	������
*/
void SoundManagerImpl::Initialize()
{
	if (m_pInstance)
		return;

	m_pInstance = new SoundManagerImpl();
}


/**
* @brief	�I������
*/
void SoundManagerImpl::Finalize()
{
	SAFE_DELETE(m_pInstance);
}


/**
* @brief	�C���X�^���X�擾
*/
SoundManagerImpl * SoundManagerImpl::GetInstance()
{
	if (!m_pInstance)
		SoundManagerImpl::Initialize();

	return m_pInstance;
}


/**
* @brief	�����V���b�g�Đ����鉹���t�@�C����ǂݍ���
* @param	[in]	pFilename	�t�@�C����
*/
void SoundManagerImpl::LoadOneshotSound(const char * pFilename)
{
	m_pOneshotSounds.push_back(new CWaveSound(pFilename));
}


/**
* @brief	�����V���b�g�Đ����鉹���t�@�C���̓ǂݍ��݂�j������
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
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
* @brief	�����V���b�g�T�E���h�̍Đ�
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[in]	dwPriority	�T�E���h�̗D��x
* @param	[in]	lVolume		�T�E���h�̉��ʁi-10000�`0�j
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
* @brief	�����V���b�g�T�E���h�̒�~
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
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

//!	�Ή��g���q�̐�
#define numExtension 2
//!	�Ή��g���q
static const char* extension[numExtension] =
{
	"ogg",
	"mp3"
};

/**
* @brief	�X�g���[�~���O�Đ����鉹���t�@�C����ǂݍ���
* @param	[in]	pFilename	�t�@�C����
* @param	[in]	bLoopFlag	���[�v�t���O
*/
void SoundManagerImpl::LoadStreamingSound(const char * pFilename, bool bLoopFlag)
{
	//!<	�g���q�̎擾
	char tmp[3];
	int len = strlen(pFilename);
	for (int i = 0; i < 3; ++i)
	{
		tmp[i] = pFilename[len - (3 - i)];
	}

	//!<	�g���q�ɉ������ԍ��̎擾
	int iExtensionNumber = -1;
	for (int i = 0; i < numExtension; ++i)
	{
		if (strstr(tmp, extension[i]) != NULL)
		{
			iExtensionNumber = i;
			break;
		}
	}
	//!<	�g���q�ɉ������I�u�W�F�N�g�̐���
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
* @brief	�X�g���[�~���O�Đ����鉹���t�@�C���̓ǂݍ��݂���������
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @details	�T�E���h�I�u�W�F�N�g�̔ԍ����l�߂��邱�Ƃɒ���
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
* @brief	�Đ�
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[in]	dwPriority	�D��x
* @param	[in]	dwFlag		�Đ��t���O
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
* @brief	��~
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
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
* @brief	�I��
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
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
* @brief	���߂ɉ������������Ă�
*/
void SoundManagerImpl::Command()
{
	SoundCommand command;	//!	Command
	unsigned int numSound;	//!	SoundObjectNumber
	DWORD dwPriority;       //! �D��x�i�Đ��p�j
	DWORD dwFlag;			//!	�Đ��t���O�i�Đ��p�j 
	//!	�L���[����f�[�^�̎��o��
	DeQueue(&numSound, &command, &dwPriority, &dwFlag);

	//	���߂ɉ���������
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
* @brief	�Đ�����
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[in]	dwPriority	�D��x
* @param	[in]	dwFlag		�Đ��t���O
*/
void SoundManagerImpl::StreamPlayCommand(unsigned int numSound, DWORD dwPriority, DWORD dwFlag)
{
	EnQueue(numSound, SOUND_PLAY, dwPriority, dwFlag);
}


/**
* @brief	��~����
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void SoundManagerImpl::StreamStopCommand(unsigned int numSound)
{
	EnQueue(numSound, SOUND_STOP, 0, 0);
}


/**
* @brief	�I������
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void SoundManagerImpl::StreamDoneCommand(unsigned int numSound)
{
	EnQueue(numSound, SOUND_DONE, 0, 0);
}


/**
* @brief	���b�N(�r������)
*/
void SoundManagerImpl::Lock()
{
	EnterCriticalSection(&m_CriticalSection);
}


/**
* @brief	�A�����b�N(�r������)
*/
void SoundManagerImpl::Unlock()
{
	LeaveCriticalSection(&m_CriticalSection);
}


/**
* @brief	_beginthread��ThreadProc���\�b�h���g�����߂̃��b�p�[
*/
void SoundManagerImpl::ThreadProcLauncher(void * arg)
{
	if (!arg)
		return;

	reinterpret_cast<SoundManagerImpl*>(arg)->ThreadProc();
}


/**
* @brief	�ʃX���b�h�Ŏ��ۂɌĂяo����郁�\�b�h
*/
void SoundManagerImpl::ThreadProc()
{
	while (ThreadFlags.m_bThreadActive) {
		switch (m_ePhase)
		{
		case THREAD_INIT:	//	������
			m_pStreamSounds.clear();
			m_ePhase = THREAD_RUN;
		case THREAD_RUN:	//	�X�V

			for (std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin(); it != m_pStreamSounds.end(); ++it) {
				(*it)->Update();
			}

			if (m_pStreamSounds.size() > 0)
				Command();

			if (ThreadFlags.m_bStopCommand) {
				m_ePhase = THREAD_WAIT;
			}

			break;
		case THREAD_WAIT:	//	�I���҂�
			Lock();
			for (std::list<IStreamingSound*>::iterator it = m_pStreamSounds.begin(); it != m_pStreamSounds.end();) {
				SAFE_DELETE(*it);
				it = m_pStreamSounds.erase(it);
			}
			Unlock();
			m_ePhase = THREAD_DONE;

			break;
		case THREAD_DONE:	//	�I������
			ThreadFlags.m_bThreadActive = false;
			break;
		}
		Sleep(16);
	}

	ThreadFlags.m_bThreadDone = true;
}


/**
* @brief	�L���[�Ƀf�[�^��ǉ�����
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[in]	Command		�ǉ����閽��
* @param	[in]	dwPriority	�D��x�i�Đ��p�j
* @param	[in]	dwFlag		�Đ��t���O�i�Đ��p�j
*/
void SoundManagerImpl::EnQueue(unsigned int numSound, SoundCommand Command, DWORD dwPriority, DWORD dwFlag)
{
	//	�L���[�����t���ǂ����̊m�F
	if (QUEUE_NEXT(m_Queue.tail) == m_Queue.head)
		return;

	//	�L���[�̖����Ƀf�[�^��}������
	m_Queue.Command[m_Queue.tail] = Command;
	m_Queue.numSound[m_Queue.tail] = numSound;
	m_Queue.dwPriority[m_Queue.tail] = dwPriority;
	m_Queue.dwFlag[m_Queue.tail] = dwFlag;

	//	�L���[�̎���}���ʒu�����肷��
	m_Queue.tail = QUEUE_NEXT(m_Queue.tail);
}


/**
* @brief	�L���[�̃f�[�^�����o��
* @param	[out]	pNumSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[out]	pCommand	����
* @param	[out]	pPriority	�D��x�i�Đ��p�j
* @param	[out]	pFlag		�Đ��t���O�i�Đ��p�j
*/
void SoundManagerImpl::DeQueue(unsigned int * pNumSound, SoundCommand * pCommand, DWORD * pPriority, DWORD * pFlag)
{
	//	�L���[�Ɏ��o���f�[�^�����݂��邩���m�F����
	if (m_Queue.head == m_Queue.tail)
		return;

	//	�L���[����f�[�^���擾����
	*pCommand = m_Queue.Command[m_Queue.head];
	*pNumSound = m_Queue.numSound[m_Queue.head];
	*pPriority = m_Queue.dwPriority[m_Queue.head];
	*pFlag = m_Queue.dwFlag[m_Queue.head];

	//	���̃f�[�^�擾�ʒu�����肷��
	m_Queue.head = QUEUE_NEXT(m_Queue.head);
}



/*****************************
* @class	CSoundManager
*****************************/

/**
* @brief	������
*/
void	CSoundManager::Initialize(HWND hWnd)
{
	CDirectSound::Initialize(hWnd);

	SoundManagerImpl::Initialize();
}


/**
* @brief	�I������
*/
void	CSoundManager::Finalize()
{
	SoundManagerImpl::Finalize();

	CDirectSound::Finalize();
}


/**
* @brief	�X�g���[�~���O�Đ����鉹����ǂݍ���
* @param	[in]	pFilename	�t�@�C����
* @param	[in]	bLoopFlag	���[�v�t���O
*/
void CSoundManager::LoadStreamingSound(const char * pFilename, bool bLoopFlag)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->LoadStreamingSound(pFilename, bLoopFlag);
}


/**
* @brief	�X�g���[�~���O�Đ����鉹���̓ǂݍ��݂���������
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void CSoundManager::UnloadStreamingSound(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->UnloadStreamingSound(numSound);
}


/**
* @brief	�Đ�
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[in]	dwPriority	�D��x
* @param	[in]	dwFlag		�Đ��t���O
*/
void CSoundManager::StreamPlay(unsigned int numSound, DWORD dwPriority, DWORD dwFlag)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->StreamPlayCommand(numSound, dwPriority, dwFlag);
}


/**
* @brief	��~
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void CSoundManager::StreamStop(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->StreamStopCommand(numSound);
}


/**
* @brief	�I��
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void CSoundManager::StreamDone(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->StreamDoneCommand(numSound);
}


/**
* @brief	�����V���b�g�T�E���h�̓ǂݍ���
* @param	[in]	pFilename	�t�@�C����
*/
void CSoundManager::LoadOneshotSound(const char * pFilename)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->LoadOneshotSound(pFilename);
}


/**
* @brief	�����V���b�g�T�E���h�̓ǂݍ��ݔj��
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void CSoundManager::UnloadOneshotSound(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->UnloadOneshotSound(numSound);
}


/**
* @brief	�����V���b�g�T�E���h�̍Đ�
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
* @param	[in]	dwPriority	�T�E���h�̗D��x
* @param	[in]	lVolume		�T�E���h�̉��ʁi-10000�`0�j
*/
void CSoundManager::OneshotPlay(unsigned int numSound, DWORD dwPriority, LONG lVolume)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->OneshotPlay(numSound, dwPriority, lVolume);
}


/**
* @brief	�����V���b�T�E���h�̒�~
* @param	[in]	numSound	�T�E���h�I�u�W�F�N�g�̔ԍ�
*/
void CSoundManager::OneshotStop(unsigned int numSound)
{
	SoundManagerImpl *pObj = SoundManagerImpl::GetInstance();

	pObj->OneshotStop(numSound);
}
