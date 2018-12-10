

#include "stdafx.h"

CPlayer *g_Player = NULL;

CPlayer::CPlayer(int serial)
    : CGameCharacter(serial)
{
    WISPFUN_DEBUG("c21_f1");
    CPacketSkillsRequest(Serial).Send();
}

CPlayer::~CPlayer()
{
    WISPFUN_DEBUG("c21_f2");
}

void CPlayer::RequestWarMode(WarModeState state)
{
    WISPFUN_DEBUG("c194_f106");

    if (SetWarMode(state))
    {
        CPacketChangeWarmode((uchar)WarMode).Send();
        m_WarModeRequests.push_back(WarMode);
    }
}

bool CPlayer::SetWarMode(WarModeState state)
{
    if (state == WarModeState::Toggle)
    {
        /* Toggle warmode */
        if (WarMode == WarModeState::Peace)
        {
            state = WarModeState::War;
        }
        else
        {
            state = WarModeState::Peace;
        }
    }

    if (WarMode == state)
    {
        return false;
    }

    WarMode = state;
    g_GumpManager.UpdateContent(g_PlayerSerial, 0, GT_STATUSBAR);

    CGumpPaperdoll *gump = (CGumpPaperdoll *)g_GumpManager.GetGump(g_PlayerSerial, 0, GT_PAPERDOLL);

    if (gump != NULL && gump->m_ButtonWarmode != NULL)
    {
        ushort graphic = 0x07E5;

        if (WarMode == WarModeState::War)
            graphic += 3;

        gump->m_ButtonWarmode->Graphic = graphic;
        gump->m_ButtonWarmode->GraphicSelected = graphic + 2;
        gump->m_ButtonWarmode->GraphicPressed = graphic + 1;

        gump->WantRedraw = true;
    }

    if (WarMode == WarModeState::War)
    {
        /* Entering warmode */
        if (g_ConfigManager.GetMusic())
        {
            g_Orion.PlayMusic(rand() % 3 + 38, true);
        }
    }
    else
    {
        /* Exiting warmode */
        g_SoundManager.StopWarMusic();
    }

    g_World->MoveToTop(this);

    return true;
}

bool CPlayer::Walk(Direction direction, bool run)
{
    WISPFUN_DEBUG("c177_f7");
    if (m_NextAllowedStepTime > g_Ticks || g_DeathScreenTimer || g_GameState != GS_GAME)
    {
        return false;
    }

    if (m_StepsOutstanding > MAX_STEPS_COUNT)
    {
        if ((m_NextAllowedStepTime + 1000) > g_Ticks)
        {
            /* We've been waiting for a response for a full second. Resync. */
            Resynchronize();
        }
        return false;
    }

    if (g_SpeedMode >= CST_CANT_RUN)
        run = false;
    else if (!run)
        run = g_ConfigManager.AlwaysRun;

    bool onMount = (FindLayer(OL_MOUNT) != NULL);
    ushort walkTime;

    Direction newDirection = direction;
    int newX = m_MovementX;
    int newY = m_MovementY;
    char newZ = m_MovementZ;

    if (m_MovementDir == newDirection)
    {
        if (!g_PathFinder.CanWalk(newDirection, newX, newY, newZ))
        {
            return false;
        }

        if (newDirection != direction)
        {
            direction = newDirection;
            walkTime = TURN_DELAY;
        }
        else
        {
            direction = newDirection;
            m_MovementX = newX;
            m_MovementY = newY;
            m_MovementZ = newZ;
            walkTime = GetWalkSpeed(run, onMount);
        }
    }
    else
    {
        if (!g_PathFinder.CanWalk(newDirection, newX, newY, newZ))
        {
            if (newDirection == m_MovementDir)
            {
                return false;
            }
        }

        if (m_MovementDir == newDirection)
        {
            direction = newDirection;
            m_MovementX = newX;
            m_MovementY = newY;
            m_MovementZ = newZ;
            walkTime = GetWalkSpeed(run, onMount);
        }
        else
        {
            direction = newDirection;
            walkTime = TURN_DELAY;
        }
    }

    CloseBank();

    m_MovementDir = direction;

    g_RemoveRangeXY.X = m_MovementX;
    g_RemoveRangeXY.Y = m_MovementY;

    QueueStep(m_MovementX, m_MovementY, m_MovementZ, m_MovementDir, run);

    CPacketWalkRequest(direction, m_SequenceNumber, run).Send();

    if (m_SequenceNumber == 0xFF)
        m_SequenceNumber = 1;
    else
        m_SequenceNumber++;

    g_World->MoveToTop(this);

    m_NextAllowedStepTime = g_Ticks + walkTime;
    m_StepsOutstanding++;

    GetAnimationGroup();

    return true;
}

void CPlayer::ConfirmWalk()
{
    if (m_StepsOutstanding == 0)
    {
        LOG("MOVEMENT: Received Walk Confirmation, but no steps pending.\n");
        Resynchronize();
        return;
    }

    m_StepsOutstanding--;
}

void CPlayer::ForcePosition(int x, int y, char z, Direction dir)
{
    m_NextAllowedStepTime = g_Ticks;
    m_SequenceNumber = 0;
    m_StepsOutstanding = 0;
    m_MovementX = x;
    m_MovementY = y;
    m_MovementZ = z;
    m_MovementDir = dir;
    m_Resynchronizing = 0;

    g_RemoveRangeXY.X = x;
    g_RemoveRangeXY.Y = y;

    CloseBank();
    g_World->MoveToTop(this);

    CGameCharacter::ForcePosition(x, y, z, dir);
}

void CPlayer::Resynchronize()
{
    if (m_Resynchronizing > 0)
    {
        /* Already resynchronizing. Wait at least 1 second between resyncs. */
        if ((m_NextAllowedStepTime + (m_Resynchronizing * 1000)) > g_Ticks)
        {
            return;
        }
    }

	LOG("MOVEMENT: Resynchronizing.\n");
    m_Resynchronizing++;
    CPacketResend().Send();
}

void CPlayer::CloseBank()
{
    CGameItem *bank = FindLayer(OL_BANK);

    if (bank != NULL && bank->Opened)
    {
        bank->Clear();
        bank->Opened = false;

        g_GumpManager.CloseGump(bank->Serial, 0, GT_CONTAINER);
    }
}

CGameItem *CPlayer::FindBandage()
{
    WISPFUN_DEBUG("c21_f11");
    CGameItem *item = FindLayer(OL_BACKPACK);

    if (item != NULL)
        item = item->FindItem(0x0E21);

    return item;
}

void CPlayer::UpdateAbilities()
{
    WISPFUN_DEBUG("c21_f12");
    ushort equippedGraphic = 0;

    CGameItem *layerObject = FindLayer(OL_1_HAND);

    if (layerObject != NULL)
    {
        equippedGraphic = layerObject->Graphic;
    }
    else
    {
        layerObject = FindLayer(OL_2_HAND);

        if (layerObject != NULL)
            equippedGraphic = layerObject->Graphic;
    }

    g_Ability[0] = 0xFF;
    g_Ability[1] = 0xFF;

    if (equippedGraphic)
    {
        ushort graphics[2] = { equippedGraphic, 0 };
        ushort imageID = layerObject->GetStaticData()->AnimID;
        int count = 1;

        ushort testGraphic = equippedGraphic - 1;

        if (g_Orion.m_StaticData[testGraphic].AnimID == imageID)
        {
            graphics[1] = testGraphic;
            count = 2;
        }
        else
        {
            testGraphic = equippedGraphic + 1;

            if (g_Orion.m_StaticData[testGraphic].AnimID == imageID)
            {
                graphics[1] = testGraphic;
                count = 2;
            }
        }

        IFOR (i, 0, count)
        {
            switch (graphics[i])
            {
                case 0x0901:
                    g_Ability[0] = AT_MOVING_SHOT;
                    g_Ability[1] = AT_INFUSED_THROW;
                    break;
                case 0x0902:
                    g_Ability[0] = AT_INFECTING;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x0905:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x0906:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x090C:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x0DF0:
                case 0x0DF1:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x0DF2:
                case 0x0DF3:
                case 0x0DF4:
                case 0x0DF5:
                    g_Ability[0] = AT_DISMOUNT;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x0E81:
                case 0x0E82:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x0E85:
                case 0x0E86:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x0E87:
                case 0x0E88:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x0E89:
                case 0x0E8A:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x0EC2:
                case 0x0EC3:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x0EC4:
                case 0x0EC5:
                    g_Ability[0] = AT_SHADOW_STRIKE;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x0F43:
                case 0x0F44:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x0F45:
                case 0x0F46:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x0F47:
                case 0x0F48:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x0F49:
                case 0x0F4A:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x0F4B:
                case 0x0F4C:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_WHIRLWIND_ATTACK;
                    break;
                case 0x0F4D:
                case 0x0F4E:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x0F4F:
                case 0x0F50:
                    g_Ability[0] = AT_CONCUSSION_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x0F51:
                case 0x0F52:
                    g_Ability[0] = AT_INFECTING;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x0F5C:
                case 0x0F5D:
                    g_Ability[0] = AT_CONCUSSION_BLOW;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x0F5E:
                case 0x0F5F:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_ARMOR_IGNORE;
                    break;
                case 0x0F60:
                case 0x0F61:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x0F62:
                case 0x0F63:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x0FB5:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x13AF:
                case 0x13B0:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_BLEED_ATTACK;
                    break;
                case 0x13B1:
                case 0x13B2:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x13B3:
                case 0x13B4:
                    g_Ability[0] = AT_SHADOW_STRIKE;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x13B7:
                case 0x13B8:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x13B9:
                case 0x13BA:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x13FD:
                    g_Ability[0] = AT_MOVING_SHOT;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x13E3:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x13F6:
                    g_Ability[0] = AT_INFECTING;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x13F8:
                    g_Ability[0] = AT_CONCUSSION_BLOW;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x13FB:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_BLEED_ATTACK;
                    break;
                case 0x13FF:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_ARMOR_IGNORE;
                    break;
                case 0x1401:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x1402:
                case 0x1403:
                    g_Ability[0] = AT_SHADOW_STRIKE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x1404:
                case 0x1405:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x1406:
                case 0x1407:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_BLEED_ATTACK;
                    break;
                case 0x1438:
                case 0x1439:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x143A:
                case 0x143B:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x143C:
                case 0x143D:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x143E:
                case 0x143F:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x1440:
                case 0x1441:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x1442:
                case 0x1443:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x26BA:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x26BB:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x26BC:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x26BD:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x26BE:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x26BF:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x26C0:
                    g_Ability[0] = AT_DISMOUNT;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x26C1:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x26C2:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_MOVING_SHOT;
                    break;
                case 0x26C3:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_MOVING_SHOT;
                    break;
                case 0x26C4:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x26C5:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x26C6:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x26C7:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x26C8:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x26C9:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x26CA:
                    g_Ability[0] = AT_DISMOUNT;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x26CB:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x26CC:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_MOVING_SHOT;
                    break;
                case 0x26CD:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_MOVING_SHOT;
                    break;
                case 0x27A2:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_RIDING_SWIPE;
                    break;
                case 0x27A3:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_BLOCK;
                    break;
                case 0x27A4:
                    g_Ability[0] = AT_FRENZIED_WHIRLWIND;
                    g_Ability[1] = AT_DOUBLE_STRIKE;
                    break;
                case 0x27A5:
                    g_Ability[0] = AT_ARMOR_PIERCE;
                    g_Ability[1] = AT_DOUBLE_SHOT;
                    break;
                case 0x27A6:
                    g_Ability[0] = AT_FRENZIED_WHIRLWIND;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x27A7:
                    g_Ability[0] = AT_DEFENSE_MASTERY;
                    g_Ability[1] = AT_FRENZIED_WHIRLWIND;
                    break;
                case 0x27A8:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_NERVE_STRIKE;
                    break;
                case 0x27A9:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_DOUBLE_STRIKE;
                    break;
                case 0x27AA:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x27AB:
                    g_Ability[0] = AT_DUAL_WIELD;
                    g_Ability[1] = AT_TALON_STRIKE;
                    break;
                case 0x27AD:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_DEFENSE_MASTERY;
                    break;
                case 0x27AE:
                    g_Ability[0] = AT_BLOCK;
                    g_Ability[1] = AT_FEINT;
                    break;
                case 0x27AF:
                    g_Ability[0] = AT_BLOCK;
                    g_Ability[1] = AT_ARMOR_PIERCE;
                    break;
                case 0x27ED:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_RIDING_SWIPE;
                    break;
                case 0x27EE:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_BLOCK;
                    break;
                case 0x27EF:
                    g_Ability[0] = AT_FRENZIED_WHIRLWIND;
                    g_Ability[1] = AT_DOUBLE_STRIKE;
                    break;
                case 0x27F0:
                    g_Ability[0] = AT_ARMOR_PIERCE;
                    g_Ability[1] = AT_DOUBLE_SHOT;
                    break;
                case 0x27F1:
                    g_Ability[0] = AT_FRENZIED_WHIRLWIND;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x27F2:
                    g_Ability[0] = AT_DEFENSE_MASTERY;
                    g_Ability[1] = AT_FRENZIED_WHIRLWIND;
                    break;
                case 0x27F3:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_NERVE_STRIKE;
                    break;
                case 0x27F4:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_DOUBLE_STRIKE;
                    break;
                case 0x27F5:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x27F6:
                    g_Ability[0] = AT_DUAL_WIELD;
                    g_Ability[1] = AT_TALON_STRIKE;
                    break;
                case 0x27F8:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_DEFENSE_MASTERY;
                    break;
                case 0x27F9:
                    g_Ability[0] = AT_BLOCK;
                    g_Ability[1] = AT_FEINT;
                    break;
                case 0x27FA:
                    g_Ability[0] = AT_BLOCK;
                    g_Ability[1] = AT_ARMOR_PIERCE;
                    break;
                case 0x2D1E:
                    g_Ability[0] = AT_FORCE_ARROW;
                    g_Ability[1] = AT_SERPENT_ARROW;
                    break;
                case 0x2D1F:
                    g_Ability[0] = AT_LIGHTNING_ARROW;
                    g_Ability[1] = AT_PSYCHIC_ATTACK;
                    break;
                case 0x2D20:
                    g_Ability[0] = AT_PSYCHIC_ATTACK;
                    g_Ability[1] = AT_BLEED_ATTACK;
                    break;
                case 0x2D21:
                    g_Ability[0] = AT_INFECTING;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x2D22:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_ARMOR_IGNORE;
                    break;
                case 0x2D23:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D24:
                    g_Ability[0] = AT_CONCUSSION_BLOW;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x2D25:
                    g_Ability[0] = AT_BLOCK;
                    g_Ability[1] = AT_FORCE_OF_NATURE;
                    break;
                case 0x2D26:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D27:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D28:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x2D29:
                    g_Ability[0] = AT_DEFENSE_MASTERY;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D2A:
                    g_Ability[0] = AT_FORCE_ARROW;
                    g_Ability[1] = AT_SERPENT_ARROW;
                    break;
                case 0x2D2B:
                    g_Ability[0] = AT_LIGHTNING_ARROW;
                    g_Ability[1] = AT_PSYCHIC_ATTACK;
                    break;
                case 0x2D2C:
                    g_Ability[0] = AT_PSYCHIC_ATTACK;
                    g_Ability[1] = AT_BLEED_ATTACK;
                    break;
                case 0x2D2D:
                    g_Ability[0] = AT_INFECTING;
                    g_Ability[1] = AT_SHADOW_STRIKE;
                    break;
                case 0x2D2E:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_ARMOR_IGNORE;
                    break;
                case 0x2D2F:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D30:
                    g_Ability[0] = AT_CONCUSSION_BLOW;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x2D31:
                    g_Ability[0] = AT_BLOCK;
                    g_Ability[1] = AT_FORCE_OF_NATURE;
                    break;
                case 0x2D32:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D33:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x2D34:
                    g_Ability[0] = AT_DISARM;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x2D35:
                    g_Ability[0] = AT_DEFENSE_MASTERY;
                    g_Ability[1] = AT_BLADEWEAVE;
                    break;
                case 0x4067:
                    g_Ability[0] = AT_MYSTIC_ARC;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x4068:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x406B:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x406C:
                    g_Ability[0] = AT_MOVING_SHOT;
                    g_Ability[1] = AT_INFUSED_THROW;
                    break;
                case 0x406D:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x406E:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x4072:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x4074:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x4075:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x4076:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x48AE:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x48B0:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x48B2:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x48B4:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_DISMOUNT;
                    break;
                case 0x48B6:
                    g_Ability[0] = AT_INFECTING;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x48B8:
                    g_Ability[0] = AT_CONCUSSION_BLOW;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x48BA:
                    g_Ability[0] = AT_DOUBLE_STRIKE;
                    g_Ability[1] = AT_ARMOR_IGNORE;
                    break;
                case 0x48BC:
                    g_Ability[0] = AT_ARMOR_IGNORE;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x48BE:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_DISARM;
                    break;
                case 0x48CA:
                    g_Ability[0] = AT_DISMOUNT;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x48C0:
                    g_Ability[0] = AT_WHIRLWIND_ATTACK;
                    g_Ability[1] = AT_CRUSHING_BLOW;
                    break;
                case 0x48C2:
                    g_Ability[0] = AT_CRUSHING_BLOW;
                    g_Ability[1] = AT_CONCUSSION_BLOW;
                    break;
                case 0x48C4:
                    g_Ability[0] = AT_BLEED_ATTACK;
                    g_Ability[1] = AT_PARALYZING_BLOW;
                    break;
                case 0x48C6:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_MORTAL_STRIKE;
                    break;
                case 0x48C8:
                    g_Ability[0] = AT_PARALYZING_BLOW;
                    g_Ability[1] = AT_INFECTING;
                    break;
                case 0x48CD:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_BLOCK;
                    break;
                case 0x48CE:
                    g_Ability[0] = AT_DUAL_WIELD;
                    g_Ability[1] = AT_TALON_STRIKE;
                    break;
                case 0x48D0:
                    g_Ability[0] = AT_FEINT;
                    g_Ability[1] = AT_DOUBLE_STRIKE;
                    break;
                default:
                    break;
            }

            if (g_Ability[0] != 0xFF)
                break;
        }
    }

    if (g_Ability[0] == 0xFF)
    {
        g_Ability[0] = AT_DISARM;
        g_Ability[1] = AT_PARALYZING_BLOW;
    }

    g_GumpManager.UpdateContent(0, 0, GT_ABILITY);
    g_GumpManager.UpdateContent(1, 0, GT_ABILITY);

    g_GumpManager.UpdateContent(0, 0, GT_COMBAT_BOOK);
}
