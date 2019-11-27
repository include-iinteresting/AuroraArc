#pragma once
class    CTetrisScene;
#define    PIECE_SIZE    4

class CPiece
{
public:
	CPiece(CTetrisScene *pt);
	virtual ~CPiece(void);
	void    resetPos();
	void    setPiece(int type, int angle);
	BOOL    down();
	void    draw(ID2D1RenderTarget *pRenderTarget);
	void    moveHorizontal(int amount);
	void    rotate();
	virtual void setJoystick(DIJOYSTATE2 *js);

protected:
	int    m_iBlocks[PIECE_SIZE*PIECE_SIZE];
	CTetrisScene    *m_pScene;
	int    m_iSize;
	int    m_iAngle;
	int    m_iType;
	int    m_iColX;
	int    m_iRowY;
	DIJOYSTATE2 m_js;

};

class shape {
public:
	static const int data[];
	static const int size[];


	static const int TYPE_PITCH = 64;
	static const int ROTATION_PITCH = 16;
};