// Server.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "SocketServer.h"


CSocketServer *pServer = NULL;

int main()
{
	pServer = new CSocketServer();

	pServer->Update();

	while (true)
	{
		if (GetAsyncKeyState(VK_ESCAPE))
			break;
	}

    return 0;
}

