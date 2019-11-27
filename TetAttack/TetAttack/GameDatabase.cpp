#include "stdafx.h"
#include "GameDatabase.h"
#include <vector>
#include "SocketClient.h"


//!	����肷��Q�[���f�[�^�p�̍\����
typedef struct GameData
{
	char *pDataName;	//!	�f�[�^�̖��O
	char *pData;		//!	�f�[�^�̒��g
};

//--- macros ---//
#undef SAFE_DELETE(o)
#undef SAFE_DELETE_ARRAY(o)
#undef SAFE_RELEASE(o)
#define SAFE_DELETE(o) if(o) { delete(o); o = NULL; }
#define SAFE_DELETE_ARRAY(o) if(o) { delete[](o); o = NULL; }
#define SAFE_RELEASE(o)if (o) { (o)->Release(); o = NULL; }

#define GAMEDATA_MAX_BUFFER_SIZE	1024
//--------------//


/**
* @brief	�Q�[���̃f�[�^���Ǘ�����N���X�̒��g
*/
class CGameDatabaseImpl
{
	friend CGameDatabase;
private:
	CGameDatabaseImpl();
	~CGameDatabaseImpl();

	static	void	Initialize();
	static	void	Finalize();
	static	CGameDatabaseImpl	*GetInstance();

	void	ConnectServer();
	void	UpdateWithServer();

	char*	GetGameData(char *pDataName);
	void	SetGameData(char *pDataName, char *pData);

private:
	static	CGameDatabaseImpl	*m_pInstance;		//!	�C���X�^���X
	CSocketClient				*m_pSocket;			//!	�\�P�b�g�ʐM�̃N���X�i�N���C�A���g���j
	std::vector<GameData*>		m_GameData;			//!	�Q�[���f�[�^
};



void CGameDatabase::setJoystick(DIJOYSTATE2 * js)//GameScene�ŌĂяo�����Stage�Ŏ��s�����
{
	m_js = *js;
}



CGameDatabaseImpl	*CGameDatabaseImpl::m_pInstance = NULL;


/**
* @brief	�R���X�g���N�^
*/
CGameDatabaseImpl::CGameDatabaseImpl()
{
	m_pSocket = new CSocketClient();
}


/**
* @brief	�f�X�g���N�^
*/
CGameDatabaseImpl::~CGameDatabaseImpl()
{
	SAFE_DELETE(m_pSocket);
}


/**
* @brief	������
*/
void CGameDatabaseImpl::Initialize()
{
	if (m_pInstance)
		return;

	m_pInstance = new CGameDatabaseImpl();
}


/**
* @brief	�I������
*/
void CGameDatabaseImpl::Finalize()
{
	SAFE_DELETE(m_pInstance);
}


/**
* @brief	�C���X�^���X�擾
*/
CGameDatabaseImpl * CGameDatabaseImpl::GetInstance()
{
	if (!m_pInstance)
		CGameDatabaseImpl::Initialize();

	return m_pInstance;
}



/**
* @brief	�T�[�o�[�Ɛڑ�����
*/
void CGameDatabaseImpl::ConnectServer()
{
	if (m_pSocket)
		m_pSocket->ConnectServer();
}


/**
* @brief	�T�[�o�[�Ƃ̒ʐM���s���̂ŁA�������Ƃ肷��K�v������
*/
void CGameDatabaseImpl::UpdateWithServer()
{
	if (!m_pSocket)
		return;

	for (std::vector<GameData*>::iterator it = m_GameData.begin(); it != m_GameData.end(); it++)
	{
		m_pSocket->Send((char*)(*it), GAMEDATA_MAX_BUFFER_SIZE);
	}
}


/**
* @brief	�Q�[���f�[�^�̎擾
* @param	[in]	pDataName	�擾����f�[�^�̖��O
*/
char * CGameDatabaseImpl::GetGameData(char * pDataName)
{
	for (std::vector<GameData*>::iterator it = m_GameData.begin(); it != m_GameData.end(); it++)
	{
		//	���O���ꏏ�Ȃ�
		if ((*it)->pDataName == pDataName) 
		{
			//	�f�[�^��Ԃ�
			return (*it)->pData;
		}
	}

	char cReturnMessage[64] = "No data with that name";
	return cReturnMessage;
}


/**
* @brief	�N���C�A���g���ŃQ�[���f�[�^���Z�b�g����
*/
void CGameDatabaseImpl::SetGameData(char *pDataName, char *pData)
{
	bool bFlag = false;

	//	���łɂ��̖��O�̃f�[�^������Ă��Ȃ����ǂ����̊m�F
	for (std::vector<GameData*>::iterator it = m_GameData.begin(); it != m_GameData.end(); it++)
	{
		//	����Ă���΃f�[�^�����������āA�t���O��true��
		if ((*it)->pDataName == pDataName)
		{
			(*it)->pData = pData;
			bFlag = true;
		}
	}

	//	�f�[�^���܂�����Ă��Ȃ���΁A�f�[�^���쐬
	if (!bFlag)
	{
		GameData Data = { pDataName, pData };
		m_GameData.push_back(new GameData(Data));
	}
}



/********************************
*	class	:	CGameDatabase	*
********************************/

/**
* @brief	�Q�[���f�[�^�̎擾
* @param	[in]	pDataName	�f�[�^�̖��O
* @return	char*	�擾���ăf�[�^�̒��g
*/
char* CGameDatabase::GetGameData(char * pDataName)
{
	char cReturnMessage[64] = "Not GameData it DataName";





	return cReturnMessage;
}
