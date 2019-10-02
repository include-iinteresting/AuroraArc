/**
*@file  XBoxController.cpp
*@brief XBox360 用コントローラを扱うクラスの実装
*/
#include "stdafx.h"
#include <vector>
#include "XBoxController.h"
#include "XInput.h"

#pragma comment(lib,"Xinput.lib")	//  XInput

///
//  隠ぺいクラスの宣言
//
typedef struct _xinput_device {
	DWORD dwInput;				//!< 実際のデバイスID
	XINPUT_CAPABILITIES cap;	//!< DeviceCaps
	XINPUT_STATE state;			//!< DoIO で取得した直近の状態
}	XINPUT_DEVICE;

class CXBoxControllerImpl : public CXBoxController
{
	friend class CXBoxController;
private:
	static void Finalize();
	void DoIO();
	void RebuildDevices();
	UINT GetNumDevices();
	BOOL GetPadLeftState(UINT id);
	BOOL GetPadUpState(UINT id);
	BOOL GetPadRightState(UINT id);
	BOOL GetPadDownState(UINT id);
	static CXBoxControllerImpl *GetInstance();
	BOOL GetButtonState(UINT id, USHORT mask);
	CXBoxControllerImpl();
	virtual ~CXBoxControllerImpl();
	static CXBoxControllerImpl * m_pInstance;
	std::vector<XINPUT_DEVICE> m_pXInputDevices;
};
CXBoxControllerImpl *CXBoxControllerImpl::m_pInstance = NULL;

/**
*@brief コンストラクタ
*/
CXBoxControllerImpl::CXBoxControllerImpl()
{
	m_pXInputDevices.clear();
}

/**
*@brief	デストラクタ
*/
CXBoxControllerImpl::~CXBoxControllerImpl()
{
	m_pXInputDevices.clear();
}

/**
*@brief  シングルトンインスタンスを取得する。なければ生成する。
*@note スレッドセーフではない
*@return instance of CXBoxContoller object
*/
CXBoxControllerImpl *CXBoxControllerImpl::GetInstance() {
	if (m_pInstance == NULL)
		m_pInstance = new CXBoxControllerImpl();
	return m_pInstance;
}

/**
*@brief  シングルトンインスタンスを削除してメモリを解放する
*/
void CXBoxControllerImpl::Finalize() {
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

/**
*@brief  デバイスをイテレーションして、デバイスをリストアップする
*/
void CXBoxControllerImpl::RebuildDevices() {
	XINPUT_DEVICE cap;
	if (m_pXInputDevices.size() > 0) {
		m_pXInputDevices.clear();
	}
	for (int i = 0; i < 4; ++i) {
		if (ERROR_SUCCESS == XInputGetCapabilities((DWORD)i, XINPUT_FLAG_GAMEPAD, &cap.cap)) {
			cap.dwInput = (DWORD)i;
			m_pXInputDevices.push_back(cap);
		}
	}
	_RPT1(_CRT_WARN, "%d inputs found.\n", m_pXInputDevices.size());
}

/**
*@brief  デバイスをイテレーションして、I/Oを実行する
*/
void CXBoxControllerImpl::DoIO() {
	for (int i = 0; i < m_pXInputDevices.size(); ++i) {
		if (ERROR_SUCCESS != XInputGetState(
				m_pXInputDevices[i].dwInput,
				&m_pXInputDevices[i].state)){
			memset((void*)&m_pXInputDevices[i].state,0, sizeof(XINPUT_STATE));
		}
	}
}
/**
*@fn
*@brief  デバイス数の取得
*@return 接続されたXBoxコントローラの
*/
UINT CXBoxControllerImpl::GetNumDevices() {
	return (UINT)m_pXInputDevices.size();
}
/**
//@brief パッドの左ボタン状態の取得
//@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
//@return       true:押されている / false:押されていない
*/
BOOL CXBoxControllerImpl::GetPadLeftState(UINT id) {
	BOOL b = false;
	XINPUT_DEVICE *pDev = NULL;
	if (id < m_pXInputDevices.size()) {
		pDev = &m_pXInputDevices[id];
		b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
		b |= (pDev->state.Gamepad.sThumbLX < -0x1000);
	}
	else  if (id == CXBoxController::ANY) {
		for (int i = 0; i < m_pXInputDevices.size(); ++i) {
			pDev = &m_pXInputDevices[i];
			b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
			b |= (pDev->state.Gamepad.sThumbLX < -0x1000);
		}
	}
	return b;
}

/**
* @brief パッドの右ボタン状態の取得
* @param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
* @return       true:押されている / false:押されていない
*/
BOOL CXBoxControllerImpl::GetPadRightState(UINT id) {
	BOOL b = false;
	XINPUT_DEVICE *pDev = NULL;
	if (id < m_pXInputDevices.size()) {
		pDev = &m_pXInputDevices[id];
		b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
		b |= (pDev->state.Gamepad.sThumbLX > 0x1000);
	}
	else  if (id == CXBoxController::ANY) {
		for (int i = 0; i < m_pXInputDevices.size(); ++i) {
			pDev = &m_pXInputDevices[i];
			b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
			b |= (pDev->state.Gamepad.sThumbLX > 0x1000);
		}
	}

	return b;
}

/**
*@brief  パッドの上ボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxControllerImpl::GetPadUpState(UINT id) {
	BOOL b = false;
	XINPUT_DEVICE *pDev = NULL;
	if (id < m_pXInputDevices.size()) {
		pDev = &m_pXInputDevices[id];
		b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
		b |= (pDev->state.Gamepad.sThumbLY > 0x1000);
	}
	else if (id == CXBoxController::ANY){
		for (int i = 0; i < m_pXInputDevices.size(); ++i) {
			pDev = &m_pXInputDevices[i];
			b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
			b |= (pDev->state.Gamepad.sThumbLY > 0x1000);
		}
	}
	return b;
}

/**
*@brief パッドの下ボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxControllerImpl::GetPadDownState(UINT id) {
	BOOL b = false;
	XINPUT_DEVICE *pDev = NULL;
	if (id < m_pXInputDevices.size()) {
		pDev = &m_pXInputDevices[id];
		b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
		b |= (pDev->state.Gamepad.sThumbLY < -0x1000);
	}
	else  if (id == CXBoxController::ANY) {
		for (int i = 0; i < m_pXInputDevices.size(); ++i) {
			pDev = &m_pXInputDevices[i];
			b = pDev->state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
			b |= (pDev->state.Gamepad.sThumbLY < -0x1000);
		}
	}
	return b;
}

/**
*@brief ボタン状態の取得
*@param [in] id   デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@param [in] mask ボタンを指定するマスク
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxControllerImpl::GetButtonState(UINT id, USHORT mask) {
	BOOL b = false;
	XINPUT_DEVICE *pDev = NULL;
	if (id < m_pXInputDevices.size()) {
		pDev = &m_pXInputDevices[id];
		b = pDev->state.Gamepad.wButtons & mask;
	}
	else if (id == CXBoxController::ANY){
		for (int i = 0; i < m_pXInputDevices.size(); ++i) {
			pDev = &m_pXInputDevices[i];
			b |= pDev->state.Gamepad.wButtons & mask;
		}
	}
	return b;

}



/**
*@brief シングルトンインスタンスを削除してメモリを解放する
*/
void CXBoxController::Finalize() {
	CXBoxControllerImpl::Finalize();
}

/**
*@brief  デバイスをイテレーションして、デバイスをリストアップする
*/
void CXBoxController::RebuildDevices() {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->RebuildDevices();
}

/**
*@brief  デバイスをイテレーションして、デバイスのIOを実行する
*/
void CXBoxController::DoIO() {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->DoIO();
}
/**
*@brief  デバイス数の取得
*@return デバイスの数
*/
UINT CXBoxController::GetNumDevices() {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetNumDevices();
}
/**
*@brief  パッドの左ボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetPadLeftState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadLeftState(id);
}

/**
*@brief パッドの右ボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetPadRightState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadRightState(id);
}

/**
*@brief  パッドの上ボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetPadUpState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadUpState(id);
}

/**
*@method
*@brief パッドの下ボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return     true:押されている / false:押されていない
*/
BOOL CXBoxController::GetPadDownState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadDownState(id);
}

/**
*@method
*@brief スタートボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetStartButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_START);
}

/**
*@method
*@brief Ａボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetAButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_A);
}

/**
*@method
*@brief Ｂボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetBButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_B);
}

/**
*@method
*@brief  Ｘボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return       true:押されている / false:押されていない
*/
BOOL CXBoxController::GetXButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_X);
}

/**
*@method
*@brief  Ｙボタン状態の取得
*@param [in] id  デバイス番号(0～4)ただし、CXBoxController::ANY で全て
*@return true:押されている / false:押されていない
*/
BOOL CXBoxController::GetYButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_Y);
}
