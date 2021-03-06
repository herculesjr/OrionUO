

#ifndef CHARACTERLISTSCREEN_H
#define CHARACTERLISTSCREEN_H

class CCharacterListScreen : public CBaseScreen
{
private:
    CGumpScreenCharacterList m_CharacterListGump;

public:
    CCharacterListScreen();
    virtual ~CCharacterListScreen();

    static const uchar ID_SMOOTH_CLS_QUIT = 1;
    static const uchar ID_SMOOTH_CLS_CONNECT = 2;
    static const uchar ID_SMOOTH_CLS_SELECT_CHARACTER = 3;
    static const uchar ID_SMOOTH_CLS_GO_SCREEN_PROFESSION_SELECT = 4;
    static const uchar ID_SMOOTH_CLS_GO_SCREEN_DELETE = 5;

    void Init();

    void ProcessSmoothAction(uchar action = 0xFF);

    virtual void OnKeyDown(const WPARAM &wParam, const LPARAM &lParam);
};

extern CCharacterListScreen g_CharacterListScreen;

#endif
