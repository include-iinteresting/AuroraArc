#pragma once


class CSocketClient
{
public:
	CSocketClient();
	virtual ~CSocketClient();

	bool	ConnectServer();
	size_t	Receive(char *pBuffer, size_t len);
	void	Send(char *pBuffer, size_t len);

private:
	bool	winsockInit(WSADATA *wsaData);	//!<	Winsockの初期化
	bool	CreateSocket(SOCKET *pSocket);	//!<	ソケットの作成
private:
	WSADATA	m_wsaData;
	SOCKET	m_Socket;
	struct sockaddr_in server;
};

