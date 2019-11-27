#include "stdafx.h"
#include "GameDatabase.h"
#include <vector>
#include "SocketClient.h"


//!	やり取りするゲームデータ用の構造体
typedef struct GameData
{
	char *pDataName;	//!	データの名前
	char *pData;		//!	データの中身
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
* @brief	ゲームのデータを管理するクラスの中身
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
	static	CGameDatabaseImpl	*m_pInstance;		//!	インスタンス
	CSocketClient				*m_pSocket;			//!	ソケット通信のクラス（クライアント側）
	std::vector<GameData*>		m_GameData;			//!	ゲームデータ
};



void CGameDatabase::setJoystick(DIJOYSTATE2 * js)//GameSceneで呼び出されてStageで実行される
{
	m_js = *js;
}



CGameDatabaseImpl	*CGameDatabaseImpl::m_pInstance = NULL;


/**
* @brief	コンストラクタ
*/
CGameDatabaseImpl::CGameDatabaseImpl()
{
	m_pSocket = new CSocketClient();
}


/**
* @brief	デストラクタ
*/
CGameDatabaseImpl::~CGameDatabaseImpl()
{
	SAFE_DELETE(m_pSocket);
}


/**
* @brief	初期化
*/
void CGameDatabaseImpl::Initialize()
{
	if (m_pInstance)
		return;

	m_pInstance = new CGameDatabaseImpl();
}


/**
* @brief	終了処理
*/
void CGameDatabaseImpl::Finalize()
{
	SAFE_DELETE(m_pInstance);
}


/**
* @brief	インスタンス取得
*/
CGameDatabaseImpl * CGameDatabaseImpl::GetInstance()
{
	if (!m_pInstance)
		CGameDatabaseImpl::Initialize();

	return m_pInstance;
}



/**
* @brief	サーバーと接続する
*/
void CGameDatabaseImpl::ConnectServer()
{
	if (m_pSocket)
		m_pSocket->ConnectServer();
}


/**
* @brief	サーバーとの通信を行うので、随時やりとりする必要がある
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
* @brief	ゲームデータの取得
* @param	[in]	pDataName	取得するデータの名前
*/
char * CGameDatabaseImpl::GetGameData(char * pDataName)
{
	for (std::vector<GameData*>::iterator it = m_GameData.begin(); it != m_GameData.end(); it++)
	{
		//	名前が一緒なら
		if ((*it)->pDataName == pDataName) 
		{
			//	データを返す
			return (*it)->pData;
		}
	}

	char cReturnMessage[64] = "No data with that name";
	return cReturnMessage;
}


/**
* @brief	クライアント側でゲームデータをセットする
*/
void CGameDatabaseImpl::SetGameData(char *pDataName, char *pData)
{
	bool bFlag = false;

	//	すでにその名前のデータが作られていないかどうかの確認
	for (std::vector<GameData*>::iterator it = m_GameData.begin(); it != m_GameData.end(); it++)
	{
		//	作られていればデータを書き換えて、フラグをtrueへ
		if ((*it)->pDataName == pDataName)
		{
			(*it)->pData = pData;
			bFlag = true;
		}
	}

	//	データがまだ作られていなければ、データを作成
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
* @brief	ゲームデータの取得
* @param	[in]	pDataName	データの名前
* @return	char*	取得してデータの中身
*/
char* CGameDatabase::GetGameData(char * pDataName)
{
	char cReturnMessage[64] = "Not GameData it DataName";





	return cReturnMessage;
}
