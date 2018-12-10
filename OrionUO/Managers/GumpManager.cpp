

#include "stdafx.h"

CGumpManager g_GumpManager;

int CGumpManager::GetNonpartyStatusbarsCount()
{
    WISPFUN_DEBUG("c144_f1");
    int count = 0;

    QFOR(gump, m_Items, CGump *)
    {
        if (gump->GumpType == GT_STATUSBAR && gump->Serial != g_PlayerSerial &&
            !g_Party.Contains(gump->Serial))
            count++;
    }

    return count;
}

void CGumpManager::AddGump(CGump *obj)
{
    WISPFUN_DEBUG("c144_f2");
    if (m_Items == NULL)
    {
        m_Items = obj;
        obj->m_Next = NULL;
        obj->m_Prev = NULL;
    }
    else
    {
        bool canCheck =
            (obj->GumpType != GT_GENERIC && obj->GumpType != GT_MENU && obj->GumpType != GT_TRADE &&
             obj->GumpType != GT_BULLETIN_BOARD_ITEM);

        QFOR(gump, m_Items, CGump *)
        {
            GUMP_TYPE gumpType = gump->GumpType;

            if (gumpType == obj->GumpType && canCheck)
            {
                if (gump->Serial == obj->Serial)
                {
                    if (gump->CanBeMoved())
                    {
                        if (gumpType == GT_DRAG || gumpType == GT_ABILITY)
                        {
                            gump->SetX(obj->GetX());
                            gump->SetY(obj->GetY());
                        }
                        else if (gumpType == GT_STATUSBAR)
                        {
                            CGumpStatusbar *sb = (CGumpStatusbar *)gump;

                            int gx = obj->GetX();
                            int gy = obj->GetY();

                            if (obj->Minimized)
                            {
                                gx = obj->MinimizedX;
                                gy = obj->MinimizedY;
                            }

                            if (sb->InGroup())
                                sb->UpdateGroup(-(gump->MinimizedX - gx), -(gump->MinimizedY - gy));

                            if (gump->Minimized)
                            {
                                gump->MinimizedX = gx;
                                gump->MinimizedY = gy;
                            }
                            else
                            {
                                gump->SetX(gx);
                                gump->SetY(gy);
                            }
                        }
                        else if (gumpType == GT_SPELL)
                        {
                            CGumpSpell *spell = (CGumpSpell *)gump;

                            int gx = obj->GetX();
                            int gy = obj->GetY();

                            if (spell->InGroup())
                                spell->UpdateGroup(-(gump->GetX() - gx), -(gump->GetY() - gy));

                            gump->SetX(gx);
                            gump->SetY(gy);
                        }
                    }

                    obj->m_Next = NULL;
                    delete obj;

                    if (gumpType == GT_WORLD_MAP && !((CGumpWorldMap *)gump)->Called)
                        ((CGumpWorldMap *)gump)->Called = true;
                    else if (gump->GumpType == GT_POPUP_MENU)
                        g_PopupMenu = (CGumpPopupMenu *)gump;
                    else if (
                        gumpType == GT_CONTAINER || gumpType == GT_JOURNAL || gumpType == GT_SKILLS)
                    {
                        gump->Minimized = false;
                        gump->Page = 2;
                    }
                    else if (gumpType == GT_MINIMAP)
                    {
                        gump->Minimized = !gump->Minimized;
                        ((CGumpMinimap *)gump)->LastX = 0;
                    }
                    else if (gumpType == GT_SPELLBOOK && gump->Minimized)
                    {
                        gump->Minimized = false;
                        gump->WantUpdateContent = true;
                    }

                    MoveToBack(gump);

                    gump->FrameCreated = false;

                    return;
                }
            }

            if (gump->m_Next == NULL)
            {
                gump->m_Next = obj;
                obj->m_Prev = gump;
                obj->m_Next = NULL;

                break;
            }
        }
    }

    if (obj->GumpType == GT_STATUSBAR && !g_Party.Contains(obj->Serial) &&
        GetNonpartyStatusbarsCount() > 10)
    {
        QFOR(gump, m_Items, CGump *)
        {
            if (gump->GumpType == GT_STATUSBAR && gump->Serial != g_PlayerSerial &&
                !g_Party.Contains(gump->Serial))
            {
                RemoveGump(gump);

                g_Orion.CreateTextMessage(
                    TT_SYSTEM, 0xFFFFFFFF, 3, 0, "You have too many non-party status bars");

                break;
            }
        }

        obj->PrepareTextures();
    }
    else
    {
        switch (obj->GumpType)
        {
            case GT_CONTAINER:
            {
                ushort sound = g_ContainerOffset[obj->Graphic].OpenSound;

                if (sound)
                    g_Orion.PlaySoundEffect(sound);

                break;
            }
            case GT_SPELLBOOK:
            {
                break;
            }
            case GT_SKILLS:
            {
                ((CGumpSkills *)obj)->Init();
                break;
            }
            case GT_OPTIONS:
            {
                ((CGumpOptions *)obj)->Init();
                break;
            }
            case GT_WORLD_MAP:
            {
                if (g_ConfigManager.GetUseGlobalMapLayer())
                    MoveToFront(obj);

                break;
            }
            default:
                break;
        }

        obj->PrepareTextures();
    }
}

CGump *CGumpManager::GetTextEntryOwner()
{
    WISPFUN_DEBUG("c144_f3");
    QFOR(gump, m_Items, CGump *)
    {
        if (gump->EntryPointerHere())
            return gump;
    }

    return NULL;
}

CGump *CGumpManager::GumpExists(uintptr_t gumpID)
{
    WISPFUN_DEBUG("c144_f4");
    QFOR(gump, m_Items, CGump *)
    {
        if (gumpID == (uintptr_t)gump)
            return gump;
    }

    return NULL;
}

CGump *CGumpManager::UpdateContent(int serial, int id, const GUMP_TYPE &type)
{
    WISPFUN_DEBUG("c144_f5");
    CGump *gump = GetGump(serial, id, type);

    if (gump != NULL)
        gump->WantUpdateContent = true;

    return gump;
}

CGump *CGumpManager::UpdateGump(int serial, int id, const GUMP_TYPE &type)
{
    WISPFUN_DEBUG("c144_f6");
    CGump *gump = GetGump(serial, id, type);

    if (gump != NULL)
        gump->WantRedraw = true;

    return gump;
}

CGump *CGumpManager::GetGump(int serial, int id, const GUMP_TYPE &type)
{
    WISPFUN_DEBUG("c144_f7");
    CGump *gump = (CGump *)m_Items;

    while (gump != NULL)
    {
        if (gump->GumpType == type)
        {
            if (type == GT_SHOP || type == GT_TARGET_SYSTEM || type == GT_CUSTOM_HOUSE)
                break;
            else if (type == GT_TRADE)
            {
                if (gump->Serial == serial)
                    break;
                else if (!serial && (gump->ID == id || ((CGumpSecureTrading *)gump)->ID2 == id))
                    break;
            }
            else if (gump->Serial == serial)
            {
                if (type == GT_CONTAINER || type == GT_SPELLBOOK || type == GT_BULLETIN_BOARD_ITEM)
                    break;
                else if (gump->ID == id)
                    break;
            }
        }

        gump = (CGump *)gump->m_Next;
    }

    return gump;
}

void CGumpManager::CloseGump(uint serial, uint id, GUMP_TYPE type)
{
    WISPFUN_DEBUG("c144_f8");
    for (CGump *gump = (CGump *)m_Items; gump != NULL;)
    {
        CGump *next = (CGump *)gump->m_Next;

        if (gump->GumpType == type)
        {
            if (type == GT_TARGET_SYSTEM)
                RemoveGump(gump);
            else if (type == GT_GENERIC && gump->Serial == serial && gump->ID == id)
                RemoveGump(gump);
            else if (type == GT_TRADE && gump->ID == serial)
                RemoveGump(gump);
            else if (type == GT_BULLETIN_BOARD_ITEM && gump->ID == id)
            {
                if (serial == 0xFFFFFFFF || gump->Serial == serial)
                    RemoveGump(gump);
            }
            else if (type == GT_SHOP || type == GT_SPELLBOOK || type == GT_DRAG)
            {
                if (serial)
                {
                    if (serial == gump->Serial)
                        RemoveGump(gump);
                }
                else
                    RemoveGump(gump);
            }
            else if (gump->Serial == serial)
                RemoveGump(gump);
        }

        gump = next;
    }
}

void CGumpManager::RemoveGump(CGump *obj)
{
    WISPFUN_DEBUG("c144_f8");
    Unlink(obj);

    if (g_World != NULL)
    {
        CGameItem *selobj = g_World->FindWorldItem(obj->Serial);

        if (selobj != NULL)
        {
            selobj->Dragged = false;
            selobj->Opened = false;
        }
    }

    if (obj->GumpType == GT_CONTAINER && obj->Graphic > 0 &&
        obj->Graphic < g_ContainerOffset.size())
    {
        ushort sound = g_ContainerOffset[obj->Graphic].CloseSound;

        if (sound)
            g_Orion.PlaySoundEffect(sound);
    }

    obj->m_Next = NULL;
    obj->m_Prev = NULL;
    delete obj;
}

void CGumpManager::OnDelete()
{
    WISPFUN_DEBUG("c144_f10");
    CGump *gump = (CGump *)m_Items;

    while (gump != NULL)
    {
        CGump *tmp = (CGump *)gump->m_Next;

        if (gump->GumpType == GT_STATUSBAR)
            RemoveGump(gump);

        gump = tmp;
    }
}

void CGumpManager::RemoveRangedGumps()
{
    WISPFUN_DEBUG("c144_f11");
    if (g_World != NULL)
    {
        CGump *gump = (CGump *)m_Items;

        while (gump != NULL)
        {
            CGump *tmp = (CGump *)gump->m_Next;

            switch (gump->GumpType)
            {
                case GT_PAPERDOLL:
                case GT_MAP:
                case GT_SPELLBOOK:
                case GT_DRAG:
                {
                    if (g_World->FindWorldObject(gump->Serial) == NULL)
                        RemoveGump(gump);
                    break;
                }
                case GT_TRADE:
                case GT_SHOP:
                {
                    if (GetTopObjDistance(g_Player, g_World->FindWorldObject(gump->Serial)) >
                        MAX_VIEW_RANGE)
                        RemoveGump(gump);
                    break;
                }
                case GT_CONTAINER:
                {
                    if (GetTopObjDistance(g_Player, g_World->FindWorldObject(gump->Serial)) >
                        REMOVE_CONTAINER_GUMP_RANGE)
                        RemoveGump(gump);
                    break;
                }
                default:
                    break;
            }

            gump = tmp;
        }
    }
}

void CGumpManager::PrepareContent()
{
    WISPFUN_DEBUG("c144_f12");
    for (CGump *gump = (CGump *)m_Items; gump != NULL;)
    {
        CGump *next = (CGump *)gump->m_Next;

        if (!gump->RemoveMark)
            gump->PrepareContent();

        if (gump->RemoveMark)
            RemoveGump(gump);

        gump = next;
    }
}

void CGumpManager::RemoveMarked()
{
    WISPFUN_DEBUG("c144_f13");
    for (CGump *gump = (CGump *)m_Items; gump != NULL;)
    {
        CGump *next = (CGump *)gump->m_Next;

        if (gump->RemoveMark)
            RemoveGump(gump);

        gump = next;
    }
}

void CGumpManager::PrepareTextures()
{
    WISPFUN_DEBUG("c144_f14");
    QFOR(gump, m_Items, CGump *)
    {
        g_CurrentCheckGump = gump;
        gump->PrepareTextures();
    }

    g_CurrentCheckGump = NULL;
}

void CGumpManager::Draw(bool blocked)
{
    WISPFUN_DEBUG("c144_f15");
    CGump *gump = (CGump *)m_Items;
    CGump *menuBarGump = NULL;

    while (gump != NULL)
    {
        CGump *next = (CGump *)gump->m_Next;

        if (blocked == gump->Blocked)
        {
            if (gump->CanBeDisplayed())
            {
                gump->Draw();
            }

            if (gump->GumpType == GT_MENUBAR)
                menuBarGump = gump;
        }

        gump = next;
    }

    if (menuBarGump != NULL)
        menuBarGump->Draw();
}

void CGumpManager::Select(bool blocked)
{
    WISPFUN_DEBUG("c144_f16");
    CGump *gump = (CGump *)m_Items;
    CGump *menuBarGump = NULL;

    while (gump != NULL)
    {
        CGump *next = (CGump *)gump->m_Next;

        if (blocked == gump->Blocked)
        {
            if (gump->CanBeDisplayed())
            {
                gump->Select();
            }

            if (gump->GumpType == GT_MENUBAR)
                menuBarGump = gump;
        }

        gump = next;
    }

    if (menuBarGump != NULL && blocked == menuBarGump->Blocked)
        menuBarGump->Select();
}

void CGumpManager::InitToolTip()
{
    WISPFUN_DEBUG("c144_f17");
    CGump *gump = g_SelectedObject.Gump;

    if (gump != NULL)
    {
        if (!g_ConfigManager.UseToolTips &&
            (!g_TooltipsEnabled || (gump->GumpType != GT_CONTAINER &&
                                    gump->GumpType != GT_PAPERDOLL && gump->GumpType != GT_TRADE)))
            return;

        gump->InitToolTip();
    }
}

void CGumpManager::RedrawAll()
{
    WISPFUN_DEBUG("c144_f18");
    QFOR(gump, m_Items, CGump *)
    gump->WantRedraw = true;
}

void CGumpManager::OnLeftMouseButtonDown(bool blocked)
{
    WISPFUN_DEBUG("c144_f19");
    if (g_SelectedObject.Object != NULL && g_SelectedObject.Object->IsText())
        return;

    QFOR(gump, m_Items, CGump *)
    {
        if (g_SelectedObject.Gump == gump && !gump->NoProcess)
        {
            if (gump->GumpType == GT_STATUSBAR && ((CGumpStatusbar *)gump)->InGroup())
                ((CGumpStatusbar *)gump)->UpdateGroup(0, 0);
            else if (gump->GumpType == GT_SPELL && ((CGumpSpell *)gump)->InGroup())
                ((CGumpSpell *)gump)->UpdateGroup(0, 0);
            else
                MoveToBack(gump);

            gump->OnLeftMouseButtonDown();

            break;
        }
    }

    RemoveMarked();
}

bool CGumpManager::OnLeftMouseButtonUp(bool blocked)
{
    WISPFUN_DEBUG("c144_f20");
    if (g_SelectedObject.Object != NULL && g_SelectedObject.Object->IsText())
        return false;

    QFOR(gump, m_Items, CGump *)
    {
        if (g_PressedObject.LeftGump == gump && !gump->NoProcess)
        {
            bool canMove = false;

            if (g_PressedObject.LeftObject != NULL)
            {
                if (!g_PressedObject.LeftObject->IsText())
                {
                    if (!g_PressedObject.LeftObject->Serial)
                        canMove = true;
                    else if (
                        g_PressedObject.LeftObject->IsGUI() &&
                        ((CBaseGUI *)g_PressedObject.LeftObject)->MoveOnDrag)
                        canMove = true;
                }
            }
            else
                canMove = true;

            if (canMove && gump->CanBeMoved() && !gump->NoMove && !g_ObjectInHand.Enabled)
            {
                WISP_GEOMETRY::CPoint2Di offset = g_MouseManager.LeftDroppedOffset();

                if (gump->GumpType == GT_STATUSBAR)
                {
                    CGumpStatusbar *sb = (CGumpStatusbar *)gump;

                    if (!gump->Minimized)
                    {
                        sb->RemoveFromGroup();
                        gump->SetX(gump->GetX() + offset.X);
                        gump->SetY(gump->GetY() + offset.Y);

                        gump->FixCoordinates();
                    }
                    else
                    {
                        gump->MinimizedX = gump->MinimizedX + offset.X;
                        gump->MinimizedY = gump->MinimizedY + offset.Y;

                        gump->FixCoordinates();

                        if (sb->InGroup())
                            sb->UpdateGroup(offset.X, offset.Y);
                        else
                        {
                            int testX = g_MouseManager.Position.X;
                            int testY = g_MouseManager.Position.Y;

                            CGumpStatusbar *nearBar = sb->GetNearStatusbar(testX, testY);

                            if (nearBar != NULL)
                            {
                                gump->MinimizedX = testX;
                                gump->MinimizedY = testY;

                                nearBar->AddStatusbar(sb);
                            }
                        }
                    }
                }
                else if (gump->GumpType == GT_SPELL)
                {
                    CGumpSpell *spell = (CGumpSpell *)gump;

                    gump->SetX(gump->GetX() + offset.X);
                    gump->SetY(gump->GetY() + offset.Y);

                    gump->FixCoordinates();

                    if (spell->InGroup())
                        spell->UpdateGroup(offset.X, offset.Y);
                    else
                    {
                        int testX = g_MouseManager.Position.X;
                        int testY = g_MouseManager.Position.Y;

                        CGumpSpell *nearSpell = spell->GetNearSpell(testX, testY);

                        if (nearSpell != NULL)
                        {
                            gump->SetX(testX);
                            gump->SetY(testY);

                            nearSpell->AddSpell(spell);
                        }
                    }
                }
                else if (gump->Minimized && gump->GumpType != GT_MINIMAP)
                {
                    gump->MinimizedX = gump->MinimizedX + offset.X;
                    gump->MinimizedY = gump->MinimizedY + offset.Y;

                    gump->FixCoordinates();
                }
                else
                {
                    gump->SetX(gump->GetX() + offset.X);
                    gump->SetY(gump->GetY() + offset.Y);

                    gump->FixCoordinates();
                }

                g_MouseManager.CancelDoubleClick = true;
            }

            if (g_ObjectInHand.Enabled)
            {
                if (g_SelectedObject.Gump == gump)
                    gump->OnLeftMouseButtonUp();
            }
            else
                gump->OnLeftMouseButtonUp();

            if (gump->GumpType == GT_WORLD_MAP && g_ConfigManager.GetUseGlobalMapLayer())
                MoveToFront(gump);

            RemoveMarked();

            return true;
        }
        else if (
            g_SelectedObject.Gump == gump && g_ObjectInHand.Enabled &&
            (gump->GumpType == GT_PAPERDOLL || gump->GumpType == GT_CONTAINER ||
             gump->GumpType == GT_TRADE))
        {
            gump->OnLeftMouseButtonUp();

            RemoveMarked();
            return true;
        }
    }

    return false;
}

bool CGumpManager::OnLeftMouseButtonDoubleClick(bool blocked)
{
    WISPFUN_DEBUG("c144_f21");
    bool result = false;

    if (g_SelectedObject.Object != NULL && g_SelectedObject.Object->IsText())
        return result;

    QFOR(gump, m_Items, CGump *)
    {
        if (g_SelectedObject.Gump == gump && !gump->NoProcess)
        {
            if (gump->OnLeftMouseButtonDoubleClick())
                result = true;

            break;
        }
    }

    RemoveMarked();

    return result;
}

void CGumpManager::OnRightMouseButtonDown(bool blocked)
{
    WISPFUN_DEBUG("c144_f22");
    if (g_SelectedObject.Object != NULL && g_SelectedObject.Object->IsText())
        return;

    QFOR(gump, m_Items, CGump *)
    {
        if (g_SelectedObject.Gump == gump && !gump->NoProcess)
        {
            if (gump->GumpType == GT_STATUSBAR && ((CGumpStatusbar *)gump)->InGroup())
                ((CGumpStatusbar *)gump)->UpdateGroup(0, 0);
            else if (gump->GumpType == GT_SPELL && ((CGumpSpell *)gump)->InGroup())
                ((CGumpSpell *)gump)->UpdateGroup(0, 0);
            else
                MoveToBack(gump);

            g_MouseManager.CancelDoubleClick = true;

            break;
        }
    }

    RemoveMarked();
}

void CGumpManager::OnRightMouseButtonUp(bool blocked)
{
    WISPFUN_DEBUG("c144_f23");
    if (g_SelectedObject.Object != NULL && g_SelectedObject.Object->IsText())
        return;

    QFOR(gump, m_Items, CGump *)
    {
        if (g_PressedObject.RightGump == gump && !gump->NoProcess && !gump->NoClose &&
            (gump->CanBeMoved() || gump->GumpType == GT_GENERIC))
        {
            switch (gump->GumpType)
            {
                case GT_CONTAINER:
                {
                    CGameItem *obj = g_World->FindWorldItem(gump->Serial);

                    if (obj != NULL)
                    {
                        if (obj->IsCorpse())
                        {
                            for (CGameItem *gi = (CGameItem *)obj->m_Items; gi != NULL;)
                            {
                                CGameItem *next = (CGameItem *)gi->m_Next;

                                if (gi->Layer == OL_NONE)
                                    g_World->RemoveObject(gi);

                                gi = next;
                            }
                        }
                        else
                            obj->Clear();

                        obj->Opened = false;
                        CloseGump(gump->Serial, 0, GT_CONTAINER);
                    }

                    break;
                }
                case GT_MENUBAR:
                {
                    gump->SetX(0);
                    gump->SetY(0);
                    gump->FrameCreated = false;

                    break;
                }
                case GT_BUFF:
                {
                    gump->FrameCreated = false;
                    g_ConfigManager.ToggleBufficonWindow = false;

                    break;
                }
                case GT_MENU:
                {
                    ((CGumpMenu *)gump)->SendMenuResponse(0);

                    break;
                }
                case GT_PROPERTY_ICON:
                {
                    if (g_ConfigManager.GetItemPropertiesMode() == OPM_ALWAYS_UP)
                        g_ConfigManager.SetItemPropertiesMode(OPM_FOLLOW_MOUSE);

                    break;
                }
                case GT_TRADE:
                {
                    ((CGumpSecureTrading *)gump)->SendTradingResponse(1);

                    break;
                }
                case GT_GENERIC:
                {
                    ((CGumpGeneric *)gump)->SendGumpResponse(0);

                    break;
                }
                case GT_OPTIONS:
                {
                    g_OptionsMacroManager.Clear();

                    CloseGump(gump->Serial, gump->ID, gump->GumpType);

                    break;
                }
                case GT_STATUSBAR:
                {
                    CGumpStatusbar *sb = ((CGumpStatusbar *)gump)->GetTopStatusbar();

                    if (sb != NULL)
                    {
                        while (sb != NULL)
                        {
                            CGumpStatusbar *next = sb->m_GroupNext;

                            sb->RemoveFromGroup();
                            RemoveGump(sb);

                            sb = next;
                        }
                    }
                    else
                        RemoveGump(gump);

                    break;
                }

                case GT_SPELLBOOK:
                {
                    g_Orion.PlaySoundEffect(0x0055);
                    CloseGump(gump->Serial, gump->ID, gump->GumpType);

                    break;
                }
                case GT_BOOK:
                {
                    ((CGumpBook *)gump)->ChangePage(0);
                    CloseGump(gump->Serial, gump->ID, gump->GumpType);

                    break;
                }
                default:
                {
                    CloseGump(gump->Serial, gump->ID, gump->GumpType);

                    break;
                }
            }

            break;
        }
    }

    RemoveMarked();
}

void CGumpManager::OnMidMouseButtonScroll(bool up, bool blocked)
{
    WISPFUN_DEBUG("c144_f24");
    QFOR(gump, m_Items, CGump *)
    {
        if (g_SelectedObject.Gump == gump && !gump->NoProcess)
        {
            gump->OnMidMouseButtonScroll(up);

            break;
        }
    }
}

void CGumpManager::OnDragging(bool blocked)
{
    WISPFUN_DEBUG("c144_f25");
    QFOR(gump, m_Items, CGump *)
    {
        if (g_PressedObject.LeftGump == gump && !gump->NoProcess)
        {
            gump->OnDragging();

            break;
        }
    }
}

bool CGumpManager::OnCharPress(const WPARAM &wParam, const LPARAM &lParam, bool blocked)
{
    WISPFUN_DEBUG("c144_f26");
    CGump *gump = GetTextEntryOwner();
    bool result = false;

    if (gump != NULL && !gump->NoProcess)
    {
        if (g_GameState == GS_GAME_BLOCKED)
        {
            if (gump->GumpType == GT_TEXT_ENTRY_DIALOG)
            {
                gump->OnCharPress(wParam, lParam);

                result = true;
            }
        }
        else
        {
            gump->OnCharPress(wParam, lParam);

            result = true;
        }
    }

    RemoveMarked();

    return result;
}

bool CGumpManager::OnKeyDown(const WPARAM &wParam, const LPARAM &lParam, bool blocked)
{
    WISPFUN_DEBUG("c144_f27");
    bool result = false;

    if (g_EntryPointer != NULL && g_EntryPointer != &g_GameConsole)
    {
        CGump *gump = GetTextEntryOwner();

        if (gump != NULL && !gump->NoProcess)
        {
            switch (gump->GumpType)
            {
                case GT_STATUSBAR:
                case GT_DRAG:
                case GT_OPTIONS:
                case GT_SKILLS:
                case GT_PROFILE:
                case GT_BULLETIN_BOARD_ITEM:
                case GT_GENERIC:
                case GT_BOOK:
                {
                    gump->OnKeyDown(wParam, lParam);

                    result = true;

                    break;
                }
            }
        }
        else if (wParam == VK_DELETE)
        {
            gump = GetGump(0, 0, GT_SKILLS);
            if (gump != NULL && !gump->NoProcess)
                gump->OnKeyDown(wParam, lParam);
        }
    }
    else if (wParam == VK_DELETE)
    {
        CGump *gump = GetGump(0, 0, GT_SKILLS);

        if (gump != NULL && !gump->NoProcess)
            gump->OnKeyDown(wParam, lParam);
    }

    RemoveMarked();

    return result;
}

void CGumpManager::Load(const string &path)
{
    WISPFUN_DEBUG("c144_f28");
    WISP_FILE::CMappedFile file;

    bool paperdollRequested = false;
    bool menubarFound = false;
    bool bufficonWindowFound = false;
    bool minimizedConsoleType = false;
    bool showFullTextConsoleType = false;

    if (file.Load(path) && file.Size)
    {
        uchar version = file.ReadUInt8();

        puchar oldPtr = file.Ptr;

        short count = 0;
        short spellGroupsCount = 0;

        if (version)
        {
            file.Ptr = (puchar)file.Start + (file.Size - 8);
            spellGroupsCount = file.ReadInt16LE();
            count = file.ReadInt16LE();
        }
        else
        {
            file.Ptr = (puchar)file.Start + (file.Size - 6);
            count = file.ReadInt16LE();
        }

        file.Ptr = oldPtr;
        bool menubarLoaded = false;

        IFOR (i, 0, count)
        {
            CGump *gump = NULL;

            puchar next = file.Ptr;
            uchar size = file.ReadUInt8();
            next += size;

            GUMP_TYPE gumpType = (GUMP_TYPE)file.ReadUInt8();
            ushort gumpX = file.ReadUInt16LE();
            ushort gumpY = file.ReadUInt16LE();
            uchar gumpMinimized = file.ReadUInt8();
            ushort gumpMinimizedX = file.ReadUInt16LE();
            ushort gumpMinimizedY = file.ReadUInt16LE();
            uchar gumpLockMoving = file.ReadUInt8();

            switch ((GUMP_TYPE)gumpType)
            {
                case GT_PAPERDOLL:
                {
                    gump = new CGumpPaperdoll(g_PlayerSerial, gumpX, gumpY, gumpMinimized);

                    g_Orion.PaperdollReq(g_PlayerSerial);
                    paperdollRequested = true;

                    break;
                }
                case GT_STATUSBAR:
                {
                    gump = new CGumpStatusbar(g_PlayerSerial, gumpX, gumpY, gumpMinimized);
                    break;
                }
                case GT_MINIMAP:
                {
                    gump = new CGumpMinimap(gumpX, gumpY, gumpMinimized);
                    break;
                }
                case GT_SKILLS:
                {
                    gump = new CGumpSkills(gumpX, gumpY, gumpMinimized, file.ReadInt16LE());
                    gump->Visible = false;

                    g_SkillsManager.SkillsRequested = true;
                    CPacketSkillsRequest(g_PlayerSerial).Send();

                    break;
                }
                case GT_JOURNAL:
                {
                    gump = new CGumpJournal(gumpX, gumpY, gumpMinimized, file.ReadInt16LE());

                    break;
                }
                case GT_WORLD_MAP:
                {
                    break;
                }
                case GT_MENUBAR:
                {
                    if (!g_ConfigManager.DisableMenubar)
                    {
                        menubarFound = true;
                        gump = new CGumpMenubar(gumpX, gumpY);
                        ((CGumpMenubar *)gump)->SetOpened(gumpMinimized);
                    }

                    break;
                }
                case GT_BUFF:
                {
                    bufficonWindowFound = true;
                    gump = new CGumpBuff(gumpX, gumpY);
                    gump->Graphic = file.ReadUInt16LE();

                    break;
                }
                case GT_ABILITY:
                {
                    gump = new CGumpAbility(file.ReadUInt32LE(), gumpX, gumpY);

                    break;
                }
                case GT_PROPERTY_ICON:
                {
                    gump = new CGumpPropertyIcon(gumpX, gumpY);

                    break;
                }
                case GT_COMBAT_BOOK:
                {
                    gump = new CGumpCombatBook(gumpX, gumpY);
                    gump->Minimized = gumpMinimized;

                    break;
                }
                case GT_CONTAINER:
                case GT_SPELLBOOK:
                case GT_SPELL:
                {
                    uint serial = file.ReadUInt32LE();

                    if (gumpType == GT_SPELL)
                    {
                        ushort graphic = file.ReadUInt16LE();

                        if (size > 18)
                            file.ReadUInt8();

                        gump = new CGumpSpell(serial, gumpX, gumpY, graphic);
                    }
                    else
                    {
                        g_ContainerStack.push_back(CContainerStackItem(
                            serial,
                            gumpX,
                            gumpY,
                            gumpMinimizedX,
                            gumpMinimizedY,
                            gumpMinimized,
                            gumpLockMoving));

                        g_UseItemActions.Add(serial);
                    }

                    break;
                }
                case GT_CONSOLE_TYPE:
                {
                    minimizedConsoleType = gumpMinimized;
                    showFullTextConsoleType = file.ReadUInt8();
                    break;
                }
                case GT_SKILL:
                {
                    uint serial = file.ReadUInt32LE();

                    gump = new CGumpSkill(serial, gumpX, gumpY);

                    break;
                }
                default:
                    break;
            }

            if (gump != NULL)
            {
                if (gumpType == GT_MENUBAR)
                {
                    if (menubarLoaded)
                    {
                        delete gump;
                        continue;
                    }

                    menubarLoaded = true;
                }

                gump->MinimizedX = gumpMinimizedX;
                gump->MinimizedY = gumpMinimizedY;
                gump->LockMoving = gumpLockMoving;

                AddGump(gump);

                gump->WantUpdateContent = true;
            }

            file.Ptr = next;
        }

        IFOR (i, 0, spellGroupsCount)
        {
            CGumpSpell *topSpell = NULL;

            ushort spellsCount = file.ReadUInt16LE();

            IFOR (j, 0, spellsCount)
            {
                puchar next = file.Ptr;
                uchar size = file.ReadUInt8();
                next += size;

                GUMP_TYPE gumpType = (GUMP_TYPE)file.ReadUInt8();
                ushort gumpX = file.ReadUInt16LE();
                ushort gumpY = file.ReadUInt16LE();
                file.Move(5);
                uchar gumpLockMoving = file.ReadUInt8();

                uint serial = file.ReadUInt32LE();
                ushort graphic = file.ReadUInt16LE();

                if (size > 18)
                    file.ReadUInt8();

                CGumpSpell *spell = new CGumpSpell(serial, gumpX, gumpY, graphic);
                spell->LockMoving = gumpLockMoving;

                AddGump(spell);

                if (topSpell == NULL)
                    topSpell = spell;
                else
                    topSpell->AddSpell(spell);

                spell->WantUpdateContent = true;

                file.Ptr = next;
            }
        }

        file.Unload();
    }
    else
    {
        if (!g_ConfigManager.DisableMenubar && !g_ConfigManager.GameWindowX &&
            !g_ConfigManager.GameWindowY)
            g_ConfigManager.GameWindowY = 40;

        WISP_GEOMETRY::CSize windowSize = g_OrionWindow.GetSize();

        int x = g_ConfigManager.GameWindowX + g_ConfigManager.GameWindowWidth;
        int y = g_ConfigManager.GameWindowY;

        if (x + 260 >= windowSize.Width)
            x = windowSize.Width - 260;

        if (y + 320 >= windowSize.Height)
            y = windowSize.Height - 320;

        AddGump(new CGumpPaperdoll(g_PlayerSerial, x, y, false));

        x = g_ConfigManager.GameWindowX + g_ConfigManager.GameWindowWidth;
        y = g_ConfigManager.GameWindowY + g_ConfigManager.GameWindowHeight - 50;

        if (x + 150 >= windowSize.Width)
            x = windowSize.Width - 150;

        if (y + 60 >= windowSize.Height)
            y = windowSize.Height - 60;

        AddGump(new CGumpStatusbar(g_PlayerSerial, x, y, false));
        AddGump(new CGumpMinimap(g_ConfigManager.GameWindowX, g_ConfigManager.GameWindowY, true));

        if (g_Player != NULL)
        {
            CGameItem *backpack = g_Player->FindLayer(OL_BACKPACK);

            if (backpack != NULL)
            {
                g_ContainerStack.push_back(CContainerStackItem(
                    backpack->Serial,
                    g_ConfigManager.GameWindowX,
                    g_ConfigManager.GameWindowY,
                    g_ConfigManager.GameWindowX,
                    g_ConfigManager.GameWindowY,
                    false,
                    false));

                g_UseItemActions.Add(backpack->Serial);
            }
        }
    }

    if (!g_ConfigManager.DisableMenubar && !menubarFound)
    {
        CGumpMenubar *mbg = new CGumpMenubar(0, 0);
        mbg->SetOpened(true);
        AddGump(mbg);
    }

    if (!bufficonWindowFound)
    {
        WISP_GEOMETRY::CSize windowSize = g_OrionWindow.GetSize();

        int x = g_ConfigManager.GameWindowX + (int)(g_ConfigManager.GameWindowWidth * 0.7f);
        int y = g_ConfigManager.GameWindowY + g_ConfigManager.GameWindowHeight;

        if (x + 100 >= windowSize.Width)
            x = windowSize.Width - 100;

        if (y + 60 >= windowSize.Height)
            y = windowSize.Height - 60;

        AddGump(new CGumpBuff(x, y));
    }

    if (!paperdollRequested)
        g_Orion.PaperdollReq(g_PlayerSerial);

    AddGump(new CGumpConsoleType(minimizedConsoleType, showFullTextConsoleType));
}

void CGumpManager::SaveDefaultGumpProperties(
    WISP_FILE::CBinaryFileWriter &writer, CGump *gump, int size)
{
    WISPFUN_DEBUG("c144_f29");
    writer.WriteInt8(size);
    writer.WriteInt8(gump->GumpType);
    writer.WriteUInt16LE(gump->GetX());
    writer.WriteUInt16LE(gump->GetY());

    if (gump->GumpType == GT_MENUBAR)
        writer.WriteInt8(((CGumpMenubar *)gump)->GetOpened());
    else
        writer.WriteInt8(gump->Minimized);

    writer.WriteUInt16LE(gump->MinimizedX);
    writer.WriteUInt16LE(gump->MinimizedY);
    writer.WriteInt8(gump->LockMoving);
};

void CGumpManager::Save(const string &path)
{
    WISPFUN_DEBUG("c144_f30");
    WISP_FILE::CBinaryFileWriter writer;

    writer.Open(path);

    writer.WriteInt8(1);
    writer.WriteBuffer();

    short count = 0;

    vector<CGump *> containerList;
    vector<CGump *> spellInGroupList;

    QFOR(gump, m_Items, CGump *)
    {
        switch (gump->GumpType)
        {
            case GT_PAPERDOLL:
            case GT_STATUSBAR:
                if (gump->Serial != g_PlayerSerial)
                    break;
            case GT_MINIMAP:
            case GT_SKILLS:
            case GT_JOURNAL:
            case GT_WORLD_MAP:
            case GT_PROPERTY_ICON:
            {
                uchar size = 12;

                if (gump->GumpType == GT_JOURNAL)
                    size += 2;
                else if (gump->GumpType == GT_SKILLS)
                    size += 2;
                else if (gump->GumpType == GT_WORLD_MAP)
                    size += 11;

                SaveDefaultGumpProperties(writer, gump, size);

                if (gump->GumpType == GT_JOURNAL)
                    writer.WriteUInt16LE(((CGumpJournal *)gump)->Height);
                else if (gump->GumpType == GT_SKILLS)
                    writer.WriteUInt16LE(((CGumpSkills *)gump)->Height);
                else if (gump->GumpType == GT_WORLD_MAP)
                {
                    CGumpWorldMap *wmg = (CGumpWorldMap *)gump;

                    writer.WriteUInt8(wmg->GetMap());
                    writer.WriteUInt8(wmg->GetScale());
                    writer.WriteUInt8(wmg->GetLinkWithPlayer());

                    writer.WriteInt16LE(wmg->Width);
                    writer.WriteInt16LE(wmg->Height);

                    writer.WriteInt16LE(wmg->OffsetX);
                    writer.WriteInt16LE(wmg->OffsetY);
                }

                writer.WriteBuffer();
                count++;

                break;
            }
            case GT_MENUBAR:
            case GT_COMBAT_BOOK:
            {
                SaveDefaultGumpProperties(writer, gump, 12);

                writer.WriteBuffer();
                count++;

                break;
            }
            case GT_BUFF:
            {
                SaveDefaultGumpProperties(writer, gump, 14);

                writer.WriteUInt16LE(gump->Graphic);

                writer.WriteBuffer();
                count++;

                break;
            }
            case GT_CONTAINER:
            case GT_SPELLBOOK:
            case GT_SPELL:
            {
                if (gump->GumpType != GT_SPELL)
                {
                    CGameObject *topobj = g_World->FindWorldObject(gump->Serial);

                    if (topobj == NULL || ((CGameItem *)topobj)->Layer == OL_BANK)
                        break;

                    topobj = topobj->GetTopObject();

                    if (topobj->Serial != g_PlayerSerial)
                        break;

                    containerList.push_back(gump);
                    break;
                }
                else if (((CGumpSpell *)gump)->InGroup())
                {
                    spellInGroupList.push_back(gump);
                    break;
                }

                SaveDefaultGumpProperties(writer, gump, 19);

                writer.WriteUInt32LE(gump->Serial);
                writer.WriteUInt16LE(gump->Graphic);
                writer.WriteUInt8(0);
                writer.WriteBuffer();

                count++;

                break;
            }
            case GT_CONSOLE_TYPE:
            {
                SaveDefaultGumpProperties(writer, gump, 13);

                writer.WriteUInt8(((CGumpConsoleType *)gump)->GetShowFullText());

                writer.WriteBuffer();
                count++;
            }
            case GT_SKILL:
            case GT_ABILITY:
            {
                SaveDefaultGumpProperties(writer, gump, 16);

                writer.WriteUInt32LE(gump->Serial);
                writer.WriteBuffer();

                count++;

                break;
            }
            default:
                break;
        }
    }

    UINT_LIST playerContainers;
    playerContainers.push_back(g_PlayerSerial);

    while (!playerContainers.empty() && !containerList.empty())
    {
        uint containerSerial = playerContainers.front();
        playerContainers.erase(playerContainers.begin());

        for (vector<CGump *>::iterator it = containerList.begin(); it != containerList.end();)
        {
            CGump *gump = *it;

            if (gump->Serial == containerSerial)
            {
                SaveDefaultGumpProperties(writer, gump, 16);

                writer.WriteUInt32LE(gump->Serial);
                writer.WriteBuffer();

                count++;

                it = containerList.erase(it);

                break;
            }
            else
                ++it;
        }

        CGameObject *owner = g_World->FindWorldObject(containerSerial);

        if (owner != NULL)
        {
            QFOR(item, owner->m_Items, CGameItem *)
            {
                if (item->Opened)
                    playerContainers.push_back(item->Serial);
            }
        }
    }

    int spellGroupsCount = 0;

    if (spellInGroupList.size())
    {
        vector<CGump *> spellGroups;

        while (spellInGroupList.size())
        {
            CGumpSpell *spell = (CGumpSpell *)spellInGroupList[0];
            CGumpSpell *topSpell = spell->GetTopSpell();
            spellGroups.push_back(topSpell);
            spellGroupsCount++;

            for (spell = topSpell; spell != NULL; spell = spell->m_GroupNext)
            {
                for (vector<CGump *>::iterator it = spellInGroupList.begin();
                     it != spellInGroupList.end();
                     ++it)
                {
                    if (*it == spell)
                    {
                        spellInGroupList.erase(it);
                        break;
                    }
                }
            }
        }

        IFOR (i, 0, spellGroupsCount)
        {
            CGumpSpell *spell = (CGumpSpell *)spellGroups[i];
            int spellsCount = 0;

            for (CGumpSpell *spell = (CGumpSpell *)spellGroups[i]; spell != NULL;
                 spell = spell->m_GroupNext)
                spellsCount++;

            writer.WriteInt16LE(spellsCount);

            for (CGumpSpell *spell = (CGumpSpell *)spellGroups[i]; spell != NULL;
                 spell = spell->m_GroupNext)
            {
                SaveDefaultGumpProperties(writer, spell, 19);

                writer.WriteUInt32LE(spell->Serial);
                writer.WriteUInt16LE(spell->Graphic);
                writer.WriteUInt8(0);
                writer.WriteBuffer();
            }
        }
    }

    writer.WriteInt16LE(spellGroupsCount);
    writer.WriteInt16LE(count);
    writer.WriteUInt32LE(0);
    writer.WriteBuffer();

    writer.Close();
}
