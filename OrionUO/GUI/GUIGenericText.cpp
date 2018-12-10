

#include "stdafx.h"

CGUIGenericText::CGUIGenericText(int index, ushort color, int x, int y, int maxWidth)
    : CGUIText(color, x, y)
    , TextID(index)
    , MaxWidth(maxWidth)
{
}

CGUIGenericText::~CGUIGenericText()
{
}

void CGUIGenericText::CreateTexture(const wstring &str)
{
    WISPFUN_DEBUG("c54_f1");
    ushort flags = UOFONT_BLACK_BORDER;

    if (MaxWidth)
        flags |= UOFONT_CROPPED;

    CreateTextureW(1, str, 30, MaxWidth, TS_LEFT, flags);
}
