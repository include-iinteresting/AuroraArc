#pragma once


class CSocketClient
{
public:
	CSocketClient();
	virtual ~CSocketClient();

	bool	ConnectServer();
	void	Receive(char *pBuffer, size_t len);
private:
	bool	winsockInit(WSADATA *wsaData);	//!<	Winsock�̏�����
	bool	CreateSocket(SOCKET *pSocket);	//!<	�\�P�b�g�̍쐬
private:
	WSADATA	m_wsaData;
	SOCKET	m_Socket;
	struct sockaddr_in server;
};

