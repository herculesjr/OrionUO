

#ifndef GUMPSKILL_H
#define GUMPSKILL_H

class CGumpSkill : public CGump
{
private:
    static const int ID_GS_LOCK_MOVING = 1;

public:
    CGumpSkill(int serial, int x, int y);
    virtual ~CGumpSkill();

    GUMP_BUTTON_EVENT_H;

    virtual void OnLeftMouseButtonUp();
};

#endif
