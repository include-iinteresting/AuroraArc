#include "stdafx.h"
#include "SocketServer.h"
//#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

/**
* @brief	コンストラクタ
*/
CSocketServer::CSocketServer()
{
	InitializeCriticalSection(&m_CriticalSection);

	winsockInit(&m_wsaData);
	
}


/**
* @brief	デストラクタ
*/
CSocketServer::~CSocketServer()
{
	// TCPセッションの終了
	closesocket(m_Socket);

	//!	Winsock2の終了処理
	WSACleanup();

	DeleteCriticalSection(&m_CriticalSection);
}


/**
* @brief	更新
*/
void CSocketServer::Update()
{
	if (m_PlayersSockets.sock1 != NULL && m_PlayersSockets.sock2 != NULL)
	{
		send(m_PlayersSockets.sock1, "HELLO", 5, 0);
		send(m_PlayersSockets.sock2, "HELLO", 5, 0);
	}
}


/**
* @brief	WinScok2の初期化
* @param	[in]	wsaData		WSADATA
* @return	初期化に成功したらtrueを返す
*/
bool CSocketServer::winsockInit(WSADATA *wsaData)
{
	int error = WSAStartup(MAKEWORD(2, 0), wsaData);

	if (error != 0)
	{
		return false;
	}

	CreateSocket(&m_Socket);

	_beginthread(ConnectThreadLauncher, 0, this);

	
	return true;
}


/**
* @brief	ソケットの作成
* @param	[in]	pSocket		SOCKET
* @return	ソケットの作成に成功したらtrueを返す
* @dateils	TCPを使う
*/
bool CSocketServer::CreateSocket(SOCKET *pSocket)
{
	*pSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*pSocket == INVALID_SOCKET) {
		return false;
	}

	// ソケットの設定
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(12345);
	m_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(*pSocket, (struct sockaddr *)&m_addr, sizeof(m_addr));


	return true;
}


/**
* @brief	接続スレッド
*/
void CSocketServer::ConnectThread()
{
	while (true)
	{
		listen(m_Socket, 5);

		int len = sizeof(m_client);
		SOCKET sock = accept(m_Socket, (struct sockaddr *)&m_client, &len);
		
		if (sock != INVALID_SOCKET)
		{
			if (m_PlayersSockets.sock1 != NULL)
			{
				Lock();
				m_PlayersSockets.sock1 = sock;
				Unlock();
				send(m_PlayersSockets.sock1, "CONNECT", 7, 0);
			}
			else if (m_PlayersSockets.sock2 != NULL)
			{
				Lock();
				m_PlayersSockets.sock2 = sock;
				Unlock();
				send(m_PlayersSockets.sock2, "CONNECT", 7, 0);
			}
		}

	}
}


/**
* @brief	接続スレッドの確認
*/
void CSocketServer::ConnectThreadLauncher(void * arg)
{
	if (!arg)
		return;

	reinterpret_cast<CSocketServer*>(arg)->ConnectThread();
}


/**
* @brief	排他制御(ロック)
*/
void CSocketServer::Lock()
{
	EnterCriticalSection(&m_CriticalSection);
}


/**
* @brief	排他制御（解放）
*/
void CSocketServer::Unlock()
{
	LeaveCriticalSection(&m_CriticalSection);
}


