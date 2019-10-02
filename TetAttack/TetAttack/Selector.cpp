#include "stdafx.h"
#include <d2d1.h>
#include <dwrite.h>
#include "Selector.h"
#include "IGameScene.h"
#include "TetrisScene.h"

CSelector::CSelector(ID2D1RenderTarget *pRenderTarget)
{
	m_pRenderTarget = pRenderTarget;
	m_pRenderTarget->AddRef();

	m_iCount = 0;
	m_eGamePhase = GamePhase::GAMEPHASE_INIT;
	m_pScene = NULL;
	m_pWhiteBrush = NULL;

	//  Direct Write 初期化
	{
		HRESULT hr;
		IDWriteFactory *pFactory;
		// Create a DirectWrite factory.
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(pFactory),
			reinterpret_cast<IUnknown **>(&pFactory)
		);

		hr = pFactory->CreateTextFormat(
			_T("consolas"),
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			20,
			L"", //locale
			&m_pTextFormat
		);

		SAFE_RELEASE(pFactory);
	}
	//  塗りつぶし用ブラシの生成
	if (FAILED(m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&m_pWhiteBrush
	))) {
		SAFE_RELEASE(m_pWhiteBrush);
	}
}

CSelector::~CSelector(void)
{
	SAFE_DELETE(m_pScene);
	SAFE_RELEASE(m_pWhiteBrush);
	SAFE_RELEASE(m_pRenderTarget);
	SAFE_RELEASE(m_pTextFormat);
}

void CSelector::doAnim() {

	switch (m_eGamePhase) {
	case    GAMEPHASE_INIT:
		m_eGamePhase = GAMEPHASE_RESET;
	case    GAMEPHASE_RESET:
		m_pScene = new CTetrisScene(this);
		//m_pScene = new CTetrisScene(this);
		m_eGamePhase = GAMEPHASE_GAME;
		//++m_iWaitCount;
	case    GAMEPHASE_GAME:
		if (m_pScene != NULL) {
			m_pScene->move();
		}
		break;
	}
	m_iCount++;
}

void CSelector::doDraw(ID2D1RenderTarget *pRenderTarget) {
	TCHAR    str[256];

	if (m_pScene != NULL) {
		m_pScene->draw(pRenderTarget);
	}
	D2D1_SIZE_F size;
	size = pRenderTarget->GetSize();

	_stprintf_s(str, _countof(str), _T("%08d"), m_iCount);
	D2D1_RECT_F    rc;
	rc.left = 0;
	rc.right = size.width;
	rc.top = 0;
	rc.bottom = size.height;
	if (m_pWhiteBrush) {
		pRenderTarget->DrawText(str, _tcslen(str), m_pTextFormat, &rc, m_pWhiteBrush);
	}
}

//
//!> Method Name : GetRenderTarget
//  @Desc : ゲーム画面用のRenderTarget を返す
//  @Note : このメソッドが返した ID2D1RenderTarget は必ずRelease すること
//
ID2D1RenderTarget *CSelector::GetRenderTaget() {
	m_pRenderTarget->AddRef();
	return m_pRenderTarget;
}

//
//!> Method Name : GetSystemTextFormat
//  @Desc : デバッグ用のTextFormat を返す
//  @Note : このメソッドが返した IDWriteTextFormat は必ずRelease すること
//
IDWriteTextFormat *CSelector::GetSystemTextFormat() {
	m_pTextFormat->AddRef();
	return m_pTextFormat;
}