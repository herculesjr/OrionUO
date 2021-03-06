

#include "stdafx.h"

CGLTextTexture::CGLTextTexture()
    : CGLTexture()
{
}

CGLTextTexture::~CGLTextTexture()
{
}

void CGLTextTexture::Clear()
{
    WISPFUN_DEBUG("c35_f1");
    CGLTexture::Clear();
    LinesCount = 0;
}

void CGLTextTexture::Draw(int x, int y, bool checktrans)
{
    WISPFUN_DEBUG("c35_f2");
    if (Texture != 0)
    {
        if (checktrans)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            g_GL.Draw(*this, x, y);

            glDisable(GL_BLEND);

            glEnable(GL_STENCIL_TEST);

            g_GL.Draw(*this, x, y);

            glDisable(GL_STENCIL_TEST);
        }
        else
            g_GL.Draw(*this, x, y);
    }
}
