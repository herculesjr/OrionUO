

#include "stdafx.h"

CGUIPolygonal::CGUIPolygonal(
    GUMP_OBJECT_TYPE type, int x, int y, int width, int height, bool callOnMouseUp)
    : CBaseGUI(type, 0, 0, 0, x, y)
    , Width(width)
    , Height(height)
    , CallOnMouseUp(callOnMouseUp)
{
}

CGUIPolygonal::~CGUIPolygonal()
{
}

bool CGUIPolygonal::Select()
{
    WISPFUN_DEBUG("c68_f1");
    int x = g_MouseManager.Position.X - m_X;
    int y = g_MouseManager.Position.Y - m_Y;

    return (x >= 0 && y >= 0 && x < Width && y < Height);
}
