#include "stdafx.h"
#include "SocketClient.h"
//#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

/**
* @brief	�R���X�g���N�^
*/
CSocketClient::CSocketClient()
{
}


/**
* @brief	�f�X�g���N�^
*/
CSocketClient::~CSocketClient()
{

	//	Winsock2�̏I������
	WSACleanup();
}


/**
* @brief	�T�[�o�[�ւ̐ڑ�
*/
bool CSocketClient::ConnectServer()
{
	// �T�[�o�ɐڑ�
	connect(m_Socket, (struct sockaddr *)&server, sizeof(server));

	return true;
}


/**
* @brief	�T�[�o�[����f�[�^����M����
*/
void CSocketClient::Receive(char *pBuffer)
{
	// �T�[�o����f�[�^����M
	memset(pBuffer, 0, sizeof(*pBuffer));
	int n = recv(m_Socket, pBuffer, sizeof(*pBuffer), 0);
}


/**
* @brief	Winsock2�̏�����
* @param	[in]	wsaData	WSADATA
* @return	���������true��Ԃ�
*/
bool CSocketClient::winsockInit(WSADATA * wsaData)
{
	// Winsock2�̏�����
	WSAStartup(MAKEWORD(2, 0), wsaData);

	return true;
}


/**
* @brief	�\�P�b�g�̍쐬�ƃT�[�o�[�ւ̐ڑ�
* @param	[in]	pSocket		SOCKET
* @return	��������΁Atrue��Ԃ�
*/
bool CSocketClient::CreateSocket(SOCKET * pSocket)
{
	// �\�P�b�g�̍쐬
	*pSocket = socket(AF_INET, SOCK_STREAM, 0);

	// �ڑ���w��p�\���̂̏���
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	return true;
}
