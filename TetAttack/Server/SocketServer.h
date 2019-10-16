#pragma once



typedef struct PLAYERSSOCKET
{
	SOCKET sock1;
	SOCKET sock2;
};

/**
* @brief	ソケット通信（サーバー側）ゲーム側では動かさない
*/
class CSocketServer
{
public:
	CSocketServer();
	virtual ~CSocketServer();
	void	Update();
private:
	bool	winsockInit(WSADATA *wsaData);	//!<	Winsockの初期化
	bool	CreateSocket(SOCKET *pSocket);	//!<	ソケットの作成

	void	ConnectThread();
	static void		ConnectThreadLauncher(void *arg);

	void	Lock();
	void	Unlock();
private:
	WSADATA m_wsaData;
	SOCKET	m_Socket;

	struct sockaddr_in m_addr;
	struct sockaddr_in m_client;

	PLAYERSSOCKET	m_PlayersSockets;

	CRITICAL_SECTION	m_CriticalSection;
};

