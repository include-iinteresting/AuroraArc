/**************************************************
*@file  XBoxController.h
*@brief XBox360 �p�R���g���[���������N���X�̐錾
***************************************************/
#pragma once

//
//	���J�N���X
//
class CXBoxController
{
public:
	static void Finalize();
	static void DoIO();
	static void RebuildDevices();
	static UINT GetNumDevices();
	static BOOL GetPadLeftState(UINT id);
	static BOOL GetPadUpState(UINT id);
	static BOOL GetPadRightState(UINT id);
	static BOOL GetPadDownState(UINT id);
	static BOOL GetStartButtonState(UINT id);
	static BOOL GetAButtonState(UINT id);
	static BOOL GetBButtonState(UINT id);
	static BOOL GetXButtonState(UINT id);
	static BOOL GetYButtonState(UINT id);
	static const UINT ANY = 0xffffffffL;
};

