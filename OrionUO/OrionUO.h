

#ifndef ORIONUO_H
#define ORIONUO_H

struct ClientConfiguration
{
    std::string HostName = "";
    int Port = 2593;
    std::string AccountName = "";
    std::string Password = "";
    bool RememberPassword = false;
    bool AutoLogin = false;
};

class COrion
{
public:
    string ClientVersionText = "7.0.15.1";
    int TexturesDataCount = 0;
    uint OrionVersionNumeric = 0;

    ClientConfiguration Config;

private:
    uint m_CRC_Table[256];

    std::list<std::string> m_Plugins;

    uchar m_StaticTilesFilterFlags[0x10000];

    USHORT_LIST m_StumpTiles;
    USHORT_LIST m_CaveTiles;

    deque<CIndexObjectStatic *> m_StaticAnimList;

    deque<CIndexObject *> m_UsedLandList;
    deque<CIndexObject *> m_UsedStaticList;
    deque<CIndexObject *> m_UsedGumpList;
    deque<CIndexObject *> m_UsedTextureList;
    deque<CIndexSound *> m_UsedSoundList;
    deque<CIndexObject *> m_UsedLightList;

    UCHAR_LIST m_AnimData;

    string m_GameServerIP = "";

    bool ParseMapSize(std::string &mapName, std::string &dimensions);

    void LoadClientConfig();
    void SaveClientConfig();

    void LoadAutoLoginNames();

    void LoadTiledata(int landSize, int staticsSize);

    void LoadIndexFiles();

    void UnloadIndexFiles();

    void InitStaticAnimList();

    ushort CalculateLightColor(ushort id);

    void ProcessStaticAnimList();

    void PatchFiles();

    void IndexReplaces();

    void LoadShaders();

    void CreateAuraTexture();

    void CreateObjectHandlesBackground();

    void ClearUnusedTextures();

    void ReadMulIndexFile(
        size_t indexMaxCount,
        std::function<CIndexObject *(int index)> getIdxObj,
        size_t address,
        PBASE_IDX_BLOCK ptr,
        std::function<PBASE_IDX_BLOCK()> getNewPtrValue);
    void ReadUOPIndexFile(
        size_t indexMaxCount,
        std::function<CIndexObject *(int index)> getIdxObj,
        const char *uopFileName,
        int padding,
        const char *extesion,
        CUopMappedFile &uopFile,
        int startIndex = 0);

    void GetCurrentLocale();

    ushort TextToGraphic(const char *text);

    void CheckStaticTileFilterFiles();

    string DecodeArgumentString(const char *text, int length);

    void LoadContaierOffsets();

public:
    COrion();
    ~COrion();

    static uint64_t CreateHash(const char *s);

    vector<LAND_TILES> m_LandData;

    vector<STATIC_TILES> m_StaticData;

    static UINT_LIST FindPattern(puchar ptr, int size, const UCHAR_LIST &pattern);

    bool Install();
    void Uninstall();

    void InitScreen(GAME_STATE state);

    void ProcessDelayedClicks();

    void Process(bool rendering = false);

    void LoadPlayerConfig(int serial);
    void SavePlayerConfig(int serial);

    void LoadPluginConfig();

    CIndexObjectLand m_LandDataIndex[MAX_LAND_DATA_INDEX_COUNT];
    CIndexObjectStatic m_StaticDataIndex[MAX_STATIC_DATA_INDEX_COUNT];
    CIndexGump m_GumpDataIndex[MAX_GUMP_DATA_INDEX_COUNT];
    CIndexObject m_TextureDataIndex[MAX_LAND_TEXTURES_DATA_INDEX_COUNT];
    CIndexSound m_SoundDataIndex[MAX_SOUND_DATA_INDEX_COUNT];
    CIndexMusic m_MP3Data[MAX_MUSIC_DATA_INDEX_COUNT];
    CIndexMulti m_MultiDataIndex[MAX_MULTI_DATA_INDEX_COUNT];
    CIndexLight m_LightDataIndex[MAX_LIGHTS_DATA_INDEX_COUNT];

    ushort m_WinterTile[MAX_LAND_DATA_INDEX_COUNT];

    vector<std::pair<ushort, ushort>> m_IgnoreInFilterTiles;

    bool InTileFilter(ushort graphic);

    static string FixServerName(string name);

    void Connect();

    void Disconnect();

    int Send(puchar buf, int size);

    int Send(const UCHAR_LIST &buf) { return Send((puchar)&buf[0], (int)buf.size()); }

    void ServerSelection(int pos);

    void RelayServer(const char *ip, int port, puchar gameSeed);

    void CharacterSelection(int pos);

    void LoginComplete(bool reload);

    void ChangeSeason(const SEASON_TYPE &season, int music);

    ushort GetLandSeasonGraphic(ushort graphic);
    ushort GetSeasonGraphic(ushort graphic);
    ushort GetSpringGraphic(ushort graphic);
    ushort GetSummerGraphic(ushort graphic);
    ushort GetFallGraphic(ushort graphic);
    ushort GetWinterGraphic(ushort graphic);
    ushort GetDesolationGraphic(ushort graphic);

    void ClearRemovedStaticsTextures();

    void ClearTreesTextures();

    bool IsTreeTile(ushort graphic, int &index);

    void ClearCaveTextures();

    bool IsCaveTile(ushort graphic);

    bool IsVegetation(ushort graphic);

    uint64 GetLandFlags(ushort id);

    uint64 GetStaticFlags(ushort id);

    ushort GetLightColor(ushort id) { return m_StaticDataIndex[id].LightColor; }

    WISP_GEOMETRY::CSize GetStaticArtDimension(ushort id);

    WISP_GEOMETRY::CSize GetGumpDimension(ushort id);

    CGLTexture *ExecuteGump(ushort id);

    CGLTexture *ExecuteLandArt(ushort id);

    CGLTexture *ExecuteStaticArt(ushort id);

    CGLTexture *ExecuteStaticArtAnimated(ushort id);

    CGLTexture *ExecuteTexture(ushort id);

    CGLTexture *ExecuteLight(uchar &id);

    bool ExecuteGumpPart(ushort id, int count);

    bool ExecuteResizepic(ushort id) { return ExecuteGumpPart(id, 9); }

    bool ExecuteButton(ushort id) { return ExecuteGumpPart(id, 3); }

    void DrawGump(ushort id, ushort color, int x, int y, bool partialHue = false);

    void
    DrawGump(ushort id, ushort color, int x, int y, int width, int height, bool partialHue = false);

    void DrawResizepicGump(ushort id, int x, int y, int width, int height);

    void DrawLandTexture(class CLandObject *land, ushort color, int x, int y);

    void DrawLandArt(ushort id, ushort color, int x, int y);

    void DrawStaticArt(ushort id, ushort color, int x, int y, bool selection = false);

    void DrawStaticArtAnimated(ushort id, ushort color, int x, int y, bool selection = false);

    void DrawStaticArtRotated(ushort id, ushort color, int x, int y, float angle);

    void DrawStaticArtAnimatedRotated(ushort id, ushort color, int x, int y, float angle);

    void DrawStaticArtTransparent(ushort id, ushort color, int x, int y, bool selection = false);

    void
    DrawStaticArtAnimatedTransparent(ushort id, ushort color, int x, int y, bool selection = false);

    void DrawStaticArtInContainer(
        ushort id, ushort color, int x, int y, bool selection = false, bool onMouse = false);

    void DrawLight(struct LIGHT_DATA &light);

    bool PolygonPixelsInXY(int x, int y, int width, int height);

    bool GumpPixelsInXY(ushort id, int x, int y);

    bool GumpPixelsInXY(ushort id, int x, int y, int width, int height);

    bool ResizepicPixelsInXY(ushort id, int x, int y, int width, int height);

    bool StaticPixelsInXY(ushort id, int x, int y);

    bool StaticPixelsInXYAnimated(ushort id, int x, int y);

    bool StaticPixelsInXYInContainer(ushort id, int x, int y);

    bool LandPixelsInXY(ushort id, int x, int y);

    bool LandTexturePixelsInXY(int x, int y, RECT &r);

    void CreateTextMessageF(uchar font, ushort color, const char *format, ...);

    void CreateUnicodeTextMessageF(uchar font, ushort color, const char *format, ...);

    void CreateTextMessage(
        const TEXT_TYPE &type,
        int serial,
        uchar font,
        ushort color,
        const string &text,
        class CRenderWorldObject *clientObj = nullptr);

    void CreateUnicodeTextMessage(
        const TEXT_TYPE &type,
        int serial,
        uchar font,
        ushort color,
        const wstring &text,
        class CRenderWorldObject *clientObj = nullptr);

    void AddSystemMessage(class CTextData *msg);

    void AddJournalMessage(class CTextData *msg, const string &name);

    void ChangeMap(uchar newmap);

    void PickupItem(class CGameItem *obj, int count = 0, bool isGameFigure = false);

    void DropItem(int container, ushort x, ushort y, char z);

    void EquipItem(uint container = 0);

    void Click(uint serial);

    void DoubleClick(uint serial);

    void PaperdollReq(uint serial);

    void Attack(uint serial);

    void AttackReq(uint serial);

    void SendASCIIText(const char *str, SPEECH_TYPE type);

    void CastSpell(int index);

    void CastSpellFromBook(int index, uint serial);

    void UseSkill(int index);

    void OpenDoor();

    void EmoteAction(const char *action);

    void AllNames();

    uint GetFileHashCode(puchar ptr, size_t size);

    void LoadLogin(string &login, int &port);

    void GoToWebLink(const string &url);

    void ResumeSound();

    void PauseSound();

    void PlayMusic(int index, bool warmode = false);

    void PlaySoundEffect(ushort index, float volume = -1);

    void AdjustSoundEffects(int ticks, float volume = -1);

    void RemoveRangedObjects();

    void ClearWorld();

    void LogOut();

    void ConsolePromptSend();

    void ConsolePromptCancel();

    void OpenStatus(uint serial);

    void DisplayStatusbarGump(int serial, int x, int y);

    void OpenMinimap();

    void OpenWorldMap();

    void OpenJournal();

    void OpenSkills();

    void OpenBackpack();

    void OpenLogOut();

    void OpenChat();

    void OpenConfiguration();

    void OpenMail();

    void OpenPartyManifest();

    void OpenProfile(uint serial = 0);

    void DisconnectGump();

    void OpenCombatBookGump();

    void StartReconnect();
};

extern COrion g_Orion;

#endif
