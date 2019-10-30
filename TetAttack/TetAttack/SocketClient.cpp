#include "stdafx.h"
#include "SocketClient.h"
//#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

/**
* @brief	コンストラクタ
*/
CSocketClient::CSocketClient()
{
	winsockInit(&m_wsaData);
	CreateSocket(&m_Socket);
}


/**
* @brief	デストラクタ
*/
CSocketClient::~CSocketClient()
{
	closesocket(m_Socket);
	//	Winsock2の終了処理
	WSACleanup();
}


/**
* @brief	サーバーへの接続
*/
bool CSocketClient::ConnectServer()
{
	// サーバに接続
	connect(m_Socket, (struct sockaddr *)&server, sizeof(server));

	return true;
}


/**
* @brief	サーバーからデータを受信する
* @param	[in]	pBuffer	受信するデータ
* @param	[in]	len		受信するデータのサイズ
* @return	size_t	受信したデータのサイズ
*/
size_t CSocketClient::Receive(char *pBuffer, size_t len)
{
	return recv(m_Socket, pBuffer, len, 0);
}


/**
* @brief	サーバーにデータを送信する
* @param	[in]	pBuffer		送信するデータ
* @param	[in]	len			送信するデータのサイズ
*/
void CSocketClient::Send(char * pBuffer, size_t len)
{
}


/**
* @brief	Winsock2の初期化
* @param	[in]	wsaData	WSADATA
* @return	成功すればtrueを返す
*/
bool CSocketClient::winsockInit(WSADATA * wsaData)
{
	// Winsock2の初期化
	WSAStartup(MAKEWORD(2, 0), wsaData);

	return true;
}


/**
* @brief	ソケットの作成とサーバーへの接続
* @param	[in]	pSocket		SOCKET
* @return	成功すれば、trueを返す
*/
bool CSocketClient::CreateSocket(SOCKET * pSocket)
{
	// ソケットの作成
	*pSocket = socket(AF_INET, SOCK_STREAM, 0);

	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	return true;
}
