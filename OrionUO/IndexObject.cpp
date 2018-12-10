

#include "stdafx.h"

CIndexObject::CIndexObject()
{
}

CIndexObject::~CIndexObject()
{
    WISPFUN_DEBUG("c189_f1");
    if (Texture != NULL)
    {
        delete Texture;
        Texture = NULL;
    }
}

CIndexObjectLand::CIndexObjectLand()
    : CIndexObject()
{
}

CIndexObjectLand::~CIndexObjectLand()
{
}

CIndexObjectStatic::CIndexObjectStatic()
    : CIndexObject()
{
}

CIndexObjectStatic::~CIndexObjectStatic()
{
}

CIndexSound::CIndexSound()
    : CIndexObject()
{
}

CIndexSound::~CIndexSound()
{
}

CIndexMulti::CIndexMulti()
    : CIndexObject()
{
}

CIndexMulti::~CIndexMulti()
{
}

CIndexLight::CIndexLight()
    : CIndexObject()
{
}

CIndexLight::~CIndexLight()
{
}

CIndexGump::CIndexGump()
    : CIndexObject()
{
}

CIndexGump::~CIndexGump()
{
}

CIndexAnimation::CIndexAnimation()
{
}

CIndexAnimation::~CIndexAnimation()
{
}

CIndexMusic::CIndexMusic()
{
}

CIndexMusic::~CIndexMusic()
{
}

void CIndexObject::ReadIndexFile(size_t address, PBASE_IDX_BLOCK ptr, const ushort id)
{
    Address = ptr->Position;
    DataSize = ptr->Size;

    if (Address == 0xFFFFFFFF || !DataSize || DataSize == 0xFFFFFFFF)
    {
        Address = 0;
        DataSize = 0;
    }
    else
        Address = Address + address;

    ID = id;
};

void CIndexMulti::ReadIndexFile(size_t address, PBASE_IDX_BLOCK ptr, const ushort id)
{
    CIndexObject::ReadIndexFile(address, ptr, id);
    Count = (ushort)(DataSize / sizeof(MULTI_BLOCK));
};

void CIndexLight::ReadIndexFile(size_t address, PBASE_IDX_BLOCK ptr, const ushort id)
{
    CIndexObject::ReadIndexFile(address, ptr, id);
    PLIGHT_IDX_BLOCK realPtr = (PLIGHT_IDX_BLOCK)ptr;
    Width = realPtr->Width;
    Height = realPtr->Height;
};

void CIndexGump::ReadIndexFile(size_t address, PBASE_IDX_BLOCK ptr, const ushort id)
{
    CIndexObject::ReadIndexFile(address, ptr, id);
    PGUMP_IDX_BLOCK realPtr = (PGUMP_IDX_BLOCK)ptr;
    Width = realPtr->Width;
    Height = realPtr->Height;
};
