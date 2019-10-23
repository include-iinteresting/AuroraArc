#pragma once



typedef struct PLAYERSSOCKET
{
	SOCKET sock1;
	SOCKET sock2;
};

/**
* @brief	�\�P�b�g�ʐM�i�T�[�o�[���j�Q�[�����ł͓������Ȃ�
*/
class CSocketServer
{
public:
	CSocketServer();
	virtual ~CSocketServer();
	void	Update();
private:
	bool	winsockInit(WSADATA *wsaData);	//!<	Winsock�̏�����
	bool	CreateSocket(SOCKET *pSocket);	//!<	�\�P�b�g�̍쐬

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

