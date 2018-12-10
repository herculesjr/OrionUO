

#ifndef GAMESCREEN_H
#define GAMESCREEN_H

struct RENDER_VARIABLES_FOR_GAME_WINDOW
{
    int GameWindowPosX;
    int GameWindowPosY;

    int GameWindowWidth;
    int GameWindowHeight;

    int GameWindowScaledWidth;
    int GameWindowScaledHeight;

    int GameWindowScaledOffsetX;
    int GameWindowScaledOffsetY;

    int GameWindowCenterX;
    int GameWindowCenterY;

    int WindowDrawOffsetX;
    int WindowDrawOffsetY;

    int RealMinRangeX;
    int RealMaxRangeX;
    int RealMinRangeY;
    int RealMaxRangeY;

    int MinBlockX;
    int MinBlockY;
    int MaxBlockX;
    int MaxBlockY;

    int MinPixelsX;
    int MaxPixelsX;
    int MinPixelsY;
    int MaxPixelsY;

    int PlayerX;
    int PlayerY;
    int PlayerZ;
};

struct LIGHT_DATA
{
    uchar ID;

    ushort Color;

    int DrawX;
    int DrawY;
};

struct RENDER_OBJECT_DATA
{
    class CRenderWorldObject *Object;

    ushort GrayColor;
};

struct OBJECT_HITS_INFO
{
    int X;
    int Y;

    ushort Color;

    int Width;

    ushort HealthColor;

    CGLTextTexture *HitsTexture;
};

class CGameScreen : public CBaseScreen
{
public:
    bool RenderListInitalized = false;

    bool UpdateDrawPos = false;
    int RenderIndex = 1;

private:
    CGameObject *m_ObjectHandlesList[MAX_OBJECT_HANDLES];

    int m_ObjectHandlesCount{ 0 };

    vector<RENDER_OBJECT_DATA> m_RenderList;

    int m_RenderListCount{ 0 };

    LIGHT_DATA m_Light[MAX_LIGHT_SOURCES];

    int m_LightCount{ 0 };

    int m_MaxDrawZ{ 0 };

    uint m_ProcessAlphaTimer{ 0 };

    bool m_CanProcessAlpha{ NULL };

    void DrawGameWindow(bool mode);

    void DrawGameWindowLight();

    void DrawGameWindowText(bool mode);

    void ApplyTransparentFoliageToUnion(ushort graphic, int x, int y, int z);

    void CheckFoliageUnion(ushort graphic, int x, int y, int z);

    void AddTileToRenderList(
        class CRenderWorldObject *obj,
        int worldX,
        int worldY,
        bool useObjectHandles,
        int maxZ = 150);

    void AddOffsetCharacterTileToRenderList(class CGameObject *obj, bool useObjectHandles);

    class CGumpScreenGame m_GameScreenGump;

    vector<OBJECT_HITS_INFO> m_HitsStack;

public:
    CGameScreen();
    virtual ~CGameScreen();

    static const uchar ID_SMOOTH_GS_LOGOUT = 1;

    short OldX = 0;
    short OldY = 0;
    char OldZ = 0;

    void UpdateMaxDrawZ();

    virtual void ProcessSmoothAction(uchar action = 0xFF);

    void CalculateGameWindowBounds();

    void CalculateLightLevel();

    void CalculateRenderList();

    virtual void Init();

    virtual void InitToolTip();

    void
    AddLight(class CRenderWorldObject *rwo, class CRenderWorldObject *lightObject, int x, int y);

    virtual void PrepareContent();

    virtual void Render(bool mode);

    virtual void OnLeftMouseButtonDown();
    virtual void OnLeftMouseButtonUp();
    virtual bool OnLeftMouseButtonDoubleClick();
    virtual void OnRightMouseButtonDown();
    virtual void OnRightMouseButtonUp();
    virtual bool OnRightMouseButtonDoubleClick();
    virtual void OnMidMouseButtonDown() {}
    virtual void OnMidMouseButtonUp() {}
    virtual bool OnMidMouseButtonDoubleClick() { return false; }
    virtual void OnMidMouseButtonScroll(bool up);
    virtual void OnDragging();
    virtual void OnCharPress(const WPARAM &wParam, const LPARAM &lParam);
    virtual void OnKeyDown(const WPARAM &wParam, const LPARAM &lParam);
    virtual void OnKeyUp(const WPARAM &wParam, const LPARAM &lParam);
};

extern CGameScreen g_GameScreen;

extern RENDER_VARIABLES_FOR_GAME_WINDOW g_RenderBounds;

#endif
