

#include "stdafx.h"

CLandObject::CLandObject(int serial, ushort graphic, ushort color, short x, short y, char z)
    : CMapObject(ROT_LAND_OBJECT, serial, 0, color, x, y, z)
    , MinZ(z)
    , AverageZ(z)
{
    WISPFUN_DEBUG("c23_f1");
    OriginalGraphic = graphic;
    UpdateGraphicBySeason();

    m_DrawTextureColor[3] = 0xFF;

    LAND_TILES &tile = g_Orion.m_LandData[graphic];

    IsStretched = (!tile.TexID && ::IsWet(tile.Flags));

    memset(&m_Rect, 0, sizeof(RECT));
    memset(&m_Normals[0], 0, sizeof(m_Normals));

#if UO_DEBUG_INFO != 0
    g_LandObjectsCount++;
#endif
}

CLandObject::~CLandObject()
{
    WISPFUN_DEBUG("c23_f2");
}

void CLandObject::UpdateGraphicBySeason()
{
    WISPFUN_DEBUG("c23_f3");
    Graphic = g_Orion.GetLandSeasonGraphic(OriginalGraphic);
    NoDrawTile = (Graphic == 2);
}

int CLandObject::GetDirectionZ(int direction)
{
    WISPFUN_DEBUG("c23_f4");
    switch (direction)
    {
        case 1:
            return (m_Rect.bottom / 4);
        case 2:
            return (m_Rect.right / 4);
        case 3:
            return (m_Rect.top / 4);
        default:
            break;
    }

    return m_Z;
}

int CLandObject::CalculateCurrentAverageZ(int direction)
{
    WISPFUN_DEBUG("c23_f5");
    int result = GetDirectionZ(((uchar)(direction >> 1) + 1) & 3);

    if (direction & 1)
        return result;

    return (result + GetDirectionZ(direction >> 1)) >> 1;
}

void CLandObject::UpdateZ(int zTop, int zRight, int zBottom)
{
    WISPFUN_DEBUG("c23_f6");
    if (IsStretched)
    {
        Serial = ((m_Z + zTop + zRight + zBottom) / 4);

        m_Rect.left = m_Z * 4 + 1;
        m_Rect.top = zTop * 4;
        m_Rect.right = zRight * 4;
        m_Rect.bottom = zBottom * 4 + 1;

        if (abs(m_Z - zRight) <= abs(zBottom - zTop))
            AverageZ = (m_Z + zRight) >> 1;
        else
            AverageZ = (zBottom + zTop) >> 1;

        MinZ = m_Z;

        if (zTop < MinZ)
            MinZ = zTop;

        if (zRight < MinZ)
            MinZ = zRight;

        if (zBottom < MinZ)
            MinZ = zBottom;
    }
}

void CLandObject::Draw(int x, int y)
{
    WISPFUN_DEBUG("c23_f7");
    if (m_Z <= g_MaxGroundZ)
    {
        ushort objColor = 0;

        if (g_DeveloperMode == DM_DEBUGGING && g_SelectedObject.Object == this)
            objColor = SELECT_LAND_COLOR;

#if UO_DEBUG_INFO != 0
        g_RenderedObjectsCountInGameWindow++;
#endif

        if (!IsStretched)
            g_Orion.DrawLandArt(Graphic, objColor, x, y);
        else
            g_Orion.DrawLandTexture(this, objColor, x, y);
    }
}

void CLandObject::Select(int x, int y)
{
    WISPFUN_DEBUG("c23_f8");
    if (m_Z <= g_MaxGroundZ)
    {
        if (!IsStretched)
        {
            if (g_Orion.LandPixelsInXY(Graphic, x, y))
                g_SelectedObject.Init(this);
        }
        else
        {
            if (g_Orion.LandTexturePixelsInXY(x, y + (m_Z * 4), m_Rect))
                g_SelectedObject.Init(this);
        }
    }
}
