

#include "stdafx.h"

CMainScreen g_MainScreen;

CMainScreen::CMainScreen()
    : CBaseScreen(m_MainGump)
    , m_Account(NULL)
    , m_Password(NULL)
    , m_SavePassword(NULL)
    , m_AutoLogin(NULL)
{
    WISPFUN_DEBUG("c165_f1");
    m_Password = new CEntryText(32, 0, 300);
}

CMainScreen::~CMainScreen()
{
    WISPFUN_DEBUG("c165_f2");
    delete m_Password;
}

void CMainScreen::Init()
{
    WISPFUN_DEBUG("c165_f3");
    g_ConfigLoaded = false;
    g_GlobalScale = 1.0;

    g_OrionWindow.SetSize(WISP_GEOMETRY::CSize(640, 480));
    g_OrionWindow.NoResize = true;
    g_OrionWindow.SetTitle("Ultima Online");
    g_GL.UpdateRect();

    if (!m_SavePassword->Checked)
    {
        m_Password->SetText(L"");
        m_MainGump.m_PasswordFake->SetText(L"");
    }

    g_EntryPointer = m_MainGump.m_PasswordFake;

    g_AnimationManager.ClearUnusedTextures(g_Ticks + 100000);

    g_QuestArrow.Enabled = false;

    g_TotalSendSize = 0;
    g_TotalRecvSize = 0;

    g_LightLevel = 0;
    g_EffectiveLightLevel = 0;

    g_ScreenEffectManager.UseSunrise();
    SmoothScreenAction = 0;

    m_Gump.PrepareTextures();
}

void CMainScreen::ProcessSmoothAction(uchar action)
{
    WISPFUN_DEBUG("c165_f4");
    if (action == 0xFF)
        action = SmoothScreenAction;

    if (action == ID_SMOOTH_MS_CONNECT)
        g_Orion.Connect();
    else if (action == ID_SMOOTH_MS_QUIT)
        g_OrionWindow.Destroy();
}

void CMainScreen::SetAccountName(const string &account)
{
    m_Account->SetText(account);
    m_Account->SetPos((int)account.length());
}

void CMainScreen::SetPassword(const string &password)
{
    m_Password->SetText(password);

    size_t len = password.length();
    m_MainGump.m_PasswordFake->Clear();

    IFOR (i, 0, len)
        m_MainGump.m_PasswordFake->Insert(L'*');

    m_Password->SetPos((int)len);
}

void CMainScreen::SetEncryptedPassword(const string &password)
{
    size_t len = password.length();

    if (len)
    {
        m_Password->SetText(DecryptPW(password.c_str(), (int)len));

        IFOR (zv, 0, len)
            m_MainGump.m_PasswordFake->Insert(L'*');

        m_Password->SetPos((int)len);
    }
}

string CMainScreen::GetEncryptedPassword()
{
    return CryptPW(m_Password->c_str(), (int)m_Password->Length());
}

void CMainScreen::Paste()
{
    WISPFUN_DEBUG("c165_f6");
    if (g_EntryPointer == m_MainGump.m_PasswordFake)
    {
        m_Password->Paste();

        size_t len = m_Password->Length();
        g_EntryPointer->Clear();

        IFOR (i, 0, len)
            g_EntryPointer->Insert(L'*');
    }
    else
        g_EntryPointer->Paste();
}

void CMainScreen::OnCharPress(const WPARAM &wParam, const LPARAM &lParam)
{
    WISPFUN_DEBUG("c165_f7");
    if (wParam >= 0x0100 || !g_FontManager.IsPrintASCII((uchar)wParam))
        return;
    else if (g_EntryPointer == NULL)
        g_EntryPointer = m_MainGump.m_PasswordFake;

    if (g_EntryPointer->Length() < 16)
    {
        if (g_EntryPointer == m_MainGump.m_PasswordFake)
        {
            if (g_EntryPointer->Insert(L'*'))
                m_Password->Insert((wchar_t)wParam);
        }
        else
            g_EntryPointer->Insert((wchar_t)wParam);
    }

    m_Gump.WantRedraw = true;
}

void CMainScreen::OnKeyDown(const WPARAM &wParam, const LPARAM &lParam)
{
    WISPFUN_DEBUG("c165_f8");
    if (g_EntryPointer == NULL)
        g_EntryPointer = m_MainGump.m_PasswordFake;

    switch (wParam)
    {
        case VK_TAB:
        {
            if (g_EntryPointer == m_Account)
                g_EntryPointer = m_MainGump.m_PasswordFake;
            else
                g_EntryPointer = m_Account;

            break;
        }
        case VK_RETURN:
        {
            CreateSmoothAction(ID_SMOOTH_MS_CONNECT);

            break;
        }
        default:
        {
            if (g_EntryPointer == m_MainGump.m_PasswordFake)
                m_Password->OnKey(NULL, wParam);

            g_EntryPointer->OnKey(NULL, wParam);

            break;
        }
    }

    m_Gump.WantRedraw = true;
}

string CMainScreen::CryptPW(const char *buf, int len)
{
    WISPFUN_DEBUG("c165_f12");
    char ret[50] = { 0 };

    IFOR (i, 0, len)
    {
        char c = buf[i];
        c += 13;

        if (c > 126)
            c -= 95;
        if (c == 32)
            c = 127;

        ret[i] = c;
    }

    return ret;
}

string CMainScreen::DecryptPW(const char *buf, int len)
{
    WISPFUN_DEBUG("c165_f13");
    char ret[50] = { 0 };

    IFOR (i, 0, len)
    {
        char c = buf[i];
        if (c == 127)
            c = 32;

        c -= 13;
        if (c < 33)
            c += 95;

        ret[i] = c;
    }

    return ret;
}
