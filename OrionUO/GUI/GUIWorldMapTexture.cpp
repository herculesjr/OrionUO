

#include "stdafx.h"

CGUIWorldMapTexture::CGUIWorldMapTexture(int x, int y)
    : CBaseGUI(GOT_EXTERNALTEXTURE, 0, 0, 0, x, y)
{
}

CGUIWorldMapTexture::~CGUIWorldMapTexture()
{
}

void CGUIWorldMapTexture::Draw(bool checktrans)
{
    WISPFUN_DEBUG("c83_f1");
    if (g_MapTexture[Index].Texture != 0)
    {
        CGLTexture tex;
        tex.Texture = g_MapTexture[Index].Texture;
        tex.Width = Width;
        tex.Height = Height;

        g_GL.Draw(tex, m_X + OffsetX, m_Y + OffsetY);

        tex.Texture = 0;
    }
}

bool CGUIWorldMapTexture::Select()
{
    WISPFUN_DEBUG("c83_f2");
    bool select = false;

    int x = g_MouseManager.Position.X - m_X;
    int y = g_MouseManager.Position.Y - m_Y;

    if (x >= 0 && y >= 0 && x < Width && y < Height)
        select = true;

    return select;
}
