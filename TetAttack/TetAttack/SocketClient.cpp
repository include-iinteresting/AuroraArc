#include "stdafx.h"
#include "SocketClient.h"
//#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

/**
* @brief	�R���X�g���N�^
*/
CSocketClient::CSocketClient()
{
	winsockInit(&m_wsaData);
	CreateSocket(&m_Socket);
}


/**
* @brief	�f�X�g���N�^
*/
CSocketClient::~CSocketClient()
{
	closesocket(m_Socket);
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
* @param	[in]	pBuffer	��M����f�[�^
* @param	[in]	len		��M����f�[�^�̃T�C�Y
* @return	size_t	��M�����f�[�^�̃T�C�Y
*/
size_t CSocketClient::Receive(char *pBuffer, size_t len)
{
	return recv(m_Socket, pBuffer, len, 0);
}


/**
* @brief	�T�[�o�[�Ƀf�[�^�𑗐M����
* @param	[in]	pBuffer		���M����f�[�^
* @param	[in]	len			���M����f�[�^�̃T�C�Y
*/
void CSocketClient::Send(char * pBuffer, size_t len)
{
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
