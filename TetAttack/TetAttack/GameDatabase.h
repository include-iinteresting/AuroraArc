#pragma once
#include <dinput.h>
class CGameDatabase
{
public:
	static	char*	GetGameData(char *pDataName);
	virtual void setJoystick(DIJOYSTATE2 *js);

protected:
	DIJOYSTATE2 m_js;
};

