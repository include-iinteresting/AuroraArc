/**
*@file  XBoxController.cpp
*@brief XBox360 �p�R���g���[���������N���X�̎���
*/
#include "stdafx.h"
#include <vector>
#include "XBoxController.h"
#include "XInput.h"

#pragma comment(lib,"Xinput.lib")	//  XInput

///
//  �B�؂��N���X�̐錾
//
typedef struct _xinput_device {
	DWORD dwInput;				//!< ���ۂ̃f�o�C�XID
	XINPUT_CAPABILITIES cap;	//!< DeviceCaps
	XINPUT_STATE state;			//!< DoIO �Ŏ擾�������߂̏��
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
*@brief �R���X�g���N�^
*/
CXBoxControllerImpl::CXBoxControllerImpl()
{
	m_pXInputDevices.clear();
}

/**
*@brief	�f�X�g���N�^
*/
CXBoxControllerImpl::~CXBoxControllerImpl()
{
	m_pXInputDevices.clear();
}

/**
*@brief  �V���O���g���C���X�^���X���擾����B�Ȃ���ΐ�������B
*@note �X���b�h�Z�[�t�ł͂Ȃ�
*@return instance of CXBoxContoller object
*/
CXBoxControllerImpl *CXBoxControllerImpl::GetInstance() {
	if (m_pInstance == NULL)
		m_pInstance = new CXBoxControllerImpl();
	return m_pInstance;
}

/**
*@brief  �V���O���g���C���X�^���X���폜���ă��������������
*/
void CXBoxControllerImpl::Finalize() {
	if (m_pInstance != NULL) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

/**
*@brief  �f�o�C�X���C�e���[�V�������āA�f�o�C�X�����X�g�A�b�v����
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
*@brief  �f�o�C�X���C�e���[�V�������āAI/O�����s����
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
*@brief  �f�o�C�X���̎擾
*@return �ڑ����ꂽXBox�R���g���[����
*/
UINT CXBoxControllerImpl::GetNumDevices() {
	return (UINT)m_pXInputDevices.size();
}
/**
//@brief �p�b�h�̍��{�^����Ԃ̎擾
//@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
//@return       true:������Ă��� / false:������Ă��Ȃ�
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
* @brief �p�b�h�̉E�{�^����Ԃ̎擾
* @param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
* @return       true:������Ă��� / false:������Ă��Ȃ�
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
*@brief  �p�b�h�̏�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
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
*@brief �p�b�h�̉��{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
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
*@brief �{�^����Ԃ̎擾
*@param [in] id   �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@param [in] mask �{�^�����w�肷��}�X�N
*@return       true:������Ă��� / false:������Ă��Ȃ�
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
*@brief �V���O���g���C���X�^���X���폜���ă��������������
*/
void CXBoxController::Finalize() {
	CXBoxControllerImpl::Finalize();
}

/**
*@brief  �f�o�C�X���C�e���[�V�������āA�f�o�C�X�����X�g�A�b�v����
*/
void CXBoxController::RebuildDevices() {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->RebuildDevices();
}

/**
*@brief  �f�o�C�X���C�e���[�V�������āA�f�o�C�X��IO�����s����
*/
void CXBoxController::DoIO() {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->DoIO();
}
/**
*@brief  �f�o�C�X���̎擾
*@return �f�o�C�X�̐�
*/
UINT CXBoxController::GetNumDevices() {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetNumDevices();
}
/**
*@brief  �p�b�h�̍��{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetPadLeftState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadLeftState(id);
}

/**
*@brief �p�b�h�̉E�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetPadRightState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadRightState(id);
}

/**
*@brief  �p�b�h�̏�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetPadUpState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadUpState(id);
}

/**
*@method
*@brief �p�b�h�̉��{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return     true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetPadDownState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetPadDownState(id);
}

/**
*@method
*@brief �X�^�[�g�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetStartButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_START);
}

/**
*@method
*@brief �`�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetAButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_A);
}

/**
*@method
*@brief �a�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetBButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_B);
}

/**
*@method
*@brief  �w�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return       true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetXButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_X);
}

/**
*@method
*@brief  �x�{�^����Ԃ̎擾
*@param [in] id  �f�o�C�X�ԍ�(0�`4)�������ACXBoxController::ANY �őS��
*@return true:������Ă��� / false:������Ă��Ȃ�
*/
BOOL CXBoxController::GetYButtonState(UINT id) {
	CXBoxControllerImpl *obj = CXBoxControllerImpl::GetInstance();
	return obj->GetButtonState(id, XINPUT_GAMEPAD_Y);
}
