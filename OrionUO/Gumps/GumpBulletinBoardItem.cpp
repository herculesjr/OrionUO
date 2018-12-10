

#include "stdafx.h"

CGumpBulletinBoardItem::CGumpBulletinBoardItem(
    int serial,
    int x,
    int y,
    uchar variant,
    int id,
    const wstring &poster,
    const wstring &subject,
    const wstring &dataTime,
    const wstring &data)
    : CGumpBaseScroll(GT_BULLETIN_BOARD_ITEM, serial, 0x0820, 250, x, y, false, 70)
    , m_Variant(variant)
{
    WISPFUN_DEBUG("c90_f1");
    ID = id;
    m_MinHeight = 200;

    int unicodeFontIndex = 1;
    int unicodeHeightOffset = -6;
    ushort textColor = 0;

    CGUIText *text = (CGUIText *)Add(new CGUIText(0, 30, 40));

    text->CreateTextureW(unicodeFontIndex, L"Author:");

    CGUIText *text2 = (CGUIText *)Add(
        new CGUIText(textColor, 30 + text->m_Texture.Width, 46 + unicodeHeightOffset));

    text2->CreateTextureW(unicodeFontIndex, poster);

    text = (CGUIText *)Add(new CGUIText(0, 30, 56));

    text->CreateTextureW(unicodeFontIndex, L"Time:");

    text2 = (CGUIText *)Add(
        new CGUIText(textColor, 30 + text->m_Texture.Width, 62 + unicodeHeightOffset));

    text2->CreateTextureW(unicodeFontIndex, dataTime);

    text = (CGUIText *)Add(new CGUIText(0, 30, 72));

    text->CreateTextureW(unicodeFontIndex, L"Subject:");

    ushort subjectColor = textColor;

    if (!m_Variant)
        subjectColor = 0x0008;

    Add(new CGUIHitBox(ID_GBBI_SUBJECT_TEXT_FIELD, 30 + text->m_Texture.Width, 78, 160, 16));

    m_EntrySubject = (CGUITextEntry *)Add(new CGUITextEntry(
        ID_GBBI_SUBJECT_TEXT_FIELD,
        subjectColor,
        subjectColor,
        subjectColor,
        30 + text->m_Texture.Width,
        78 + unicodeHeightOffset,
        150,
        1,
        unicodeFontIndex));
    m_EntrySubject->m_Entry.SetText(subject);

    if (!m_Variant)
    {
        CGump *gumpEntry = g_GumpManager.GetTextEntryOwner();

        if (gumpEntry != NULL)
            gumpEntry->WantRedraw = true;

        g_EntryPointer = &m_EntrySubject->m_Entry;
    }

    Add(new CGUIGumppicTiled(0x0835, 30, 100, 204, 0));

    m_Entry = (CGUITextEntry *)m_HTMLGump->Add(new CGUITextEntry(
        ID_GBBI_TEXT_FIELD, textColor, textColor, textColor, 3, 3, 220, 1, unicodeFontIndex));
    m_Entry->m_Entry.MaxWidth = 0;
    m_Entry->m_Entry.SetText(data);
    m_Entry->m_Entry.CreateTextureA(9, m_Entry->m_Entry.c_str(), textColor, 220, TS_LEFT, 0);
    m_HitBox = (CGUIHitBox *)m_HTMLGump->Add(
        new CGUIHitBox(ID_GBBI_TEXT_FIELD, 3, 3, 220, m_Entry->m_Entry.m_Texture.Height));

    if (m_HitBox->Height < 14)
        m_HitBox->Height = 14;

    m_HTMLGump->CalculateDataSize();

    m_ButtonPost = NULL;
    m_ButtonRemove = NULL;
    m_ButtonReply = NULL;

    switch (m_Variant)
    {
        case 0:
        {
            Add(new CGUIGumppic(0x0883, 97, 12));
            m_ButtonPost = (CGUIButton *)Add(
                new CGUIButton(ID_GBBI_POST, 0x0886, 0x0886, 0x0886, 37, Height - 22));
            m_ButtonPost->CheckPolygon = true;

            break;
        }
        case 2:
            m_ButtonRemove = (CGUIButton *)Add(
                new CGUIButton(ID_GBBI_REMOVE, 0x0885, 0x0885, 0x0885, 235, Height - 22));
            m_ButtonRemove->CheckPolygon = true;
        case 1:
        {
            m_ButtonReply = (CGUIButton *)Add(
                new CGUIButton(ID_GBBI_REPLY, 0x0884, 0x0884, 0x0884, 37, Height - 22));
            m_ButtonReply->CheckPolygon = true;

            m_EntrySubject->ReadOnly = true;
            m_Entry->ReadOnly = true;

            break;
        }
        default:
            break;
    }
}

CGumpBulletinBoardItem::~CGumpBulletinBoardItem()
{
}

void CGumpBulletinBoardItem::UpdateHeight()
{
    WISPFUN_DEBUG("c90_f2");
    CGumpBaseScroll::UpdateHeight();

    if (m_ButtonPost != NULL)
        m_ButtonPost->SetY(Height - 22);

    if (m_ButtonRemove != NULL)
        m_ButtonRemove->SetY(Height - 22);

    if (m_ButtonReply != NULL)
        m_ButtonReply->SetY(Height - 22);
}

void CGumpBulletinBoardItem::RecalculateHeight()
{
    WISPFUN_DEBUG("c90_f3");
    if (g_EntryPointer == &m_Entry->m_Entry)
    {
        m_Entry->m_Entry.CreateTextureA(9, m_Entry->m_Entry.c_str(), 0x0386, 220, TS_LEFT, 0);
        m_HitBox->Height = m_Entry->m_Entry.m_Texture.Height;

        if (m_HitBox->Height < 14)
            m_HitBox->Height = 14;

        m_HTMLGump->CalculateDataSize();
    }
}

void CGumpBulletinBoardItem::GUMP_BUTTON_EVENT_C
{
    WISPFUN_DEBUG("c90_f4");
    if (m_EntrySubject != NULL)
    {
        if (serial == ID_GBBI_POST)
        {
            CPacketBulletinBoardPostMessage(
                ID, 0, m_EntrySubject->m_Entry.c_str(), m_Entry->m_Entry.c_str())
                .Send();

            RemoveMark = true;
        }
        else if (serial == ID_GBBI_REPLY)
        {
            wstring subj(L"RE: ");
            subj += m_EntrySubject->m_Entry.Data();

            CGumpBulletinBoardItem *gump = new CGumpBulletinBoardItem(
                0, 0, 0, 0, ID, ToWString(g_Player->GetName()), subj, L"Date/Time", L"");

            g_GumpManager.AddGump(gump);
        }
        else if (serial == ID_GBBI_REMOVE)
        {
            CPacketBulletinBoardRemoveMessage(ID, Serial).Send();
        }
    }
}

void CGumpBulletinBoardItem::OnCharPress(const WPARAM &wParam, const LPARAM &lParam)
{
    WISPFUN_DEBUG("c90_f5");
    g_EntryPointer->Insert((wchar_t)wParam);

    RecalculateHeight();
    WantRedraw = true;
}

void CGumpBulletinBoardItem::OnKeyDown(const WPARAM &wParam, const LPARAM &lParam)
{
    WISPFUN_DEBUG("c90_f6");
    if (wParam == VK_RETURN && m_Entry != NULL && g_EntryPointer == &m_Entry->m_Entry)
    {
        g_EntryPointer->Insert(L'\n');

        RecalculateHeight();
        WantRedraw = true;
    }
    else
    {
        g_EntryPointer->OnKey(this, wParam);

        if (WantRedraw)
            RecalculateHeight();
    }
}
