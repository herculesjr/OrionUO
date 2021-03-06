

#include "stdafx.h"

CConnectionManager g_ConnectionManager;

CConnectionManager::CConnectionManager()
{
}

CConnectionManager::~CConnectionManager()
{
    WISPFUN_DEBUG("c139_f1");

    if (m_LoginSocket.Connected)
        m_LoginSocket.Disconnect();

    if (m_GameSocket.Connected)
        m_GameSocket.Disconnect();
}

void CConnectionManager::SetUseProxy(bool val)
{
    WISPFUN_DEBUG("c139_f2");
    m_UseProxy = val;
    m_LoginSocket.UseProxy = val;
    m_GameSocket.UseProxy = val;
}

void CConnectionManager::SetProxyAddress(const string &val)
{
    WISPFUN_DEBUG("c139_f3");
    m_ProxyAddress = val;
    m_LoginSocket.ProxyAddress = val;
    m_GameSocket.ProxyAddress = val;
}

void CConnectionManager::SetProxyPort(int val)
{
    WISPFUN_DEBUG("c139_f4");
    m_ProxyPort = val;
    m_LoginSocket.ProxyPort = val;
    m_GameSocket.ProxyPort = val;
}

void CConnectionManager::SetProxySocks5(bool val)
{
    WISPFUN_DEBUG("c139_f5");
    m_ProxySocks5 = val;
    m_LoginSocket.ProxySocks5 = val;
    m_GameSocket.ProxySocks5 = val;
}

void CConnectionManager::SetProxyAccount(const string &val)
{
    WISPFUN_DEBUG("c139_f6");
    m_ProxyAccount = val;
    m_LoginSocket.ProxyAccount = val;
    m_GameSocket.ProxyAccount = val;
}

void CConnectionManager::SetProxyPassword(const string &val)
{
    WISPFUN_DEBUG("c139_f7");
    m_ProxyPassword = val;
    m_LoginSocket.ProxyPassword = val;
    m_GameSocket.ProxyPassword = val;
}

void CConnectionManager::Init()
{
    WISPFUN_DEBUG("c139_f8");

    if (m_LoginSocket.Connected)
        return;

    m_IsLoginSocket = true;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(1, 1), &wsaData);

    char hostName[1024] = { 0 };

    if (!gethostname(hostName, 1024))
    {
        if (LPHOSTENT lphost = gethostbyname(hostName))
        {
            WISP_DATASTREAM::CDataWriter stream;
            stream.WriteUInt32LE(((LPIN_ADDR)lphost->h_addr)->s_addr);
            UCHAR_LIST &data = stream.Data();

            memcpy(&m_Seed[0], &data[0], 4);
        }
    }

    WSACleanup();
}

void CConnectionManager::Init(puchar gameSeed)
{
    WISPFUN_DEBUG("c139_f9");

    if (m_GameSocket.Connected)
        return;

    m_IsLoginSocket = false;
}

void CConnectionManager::SendIP(CSocket &socket, puchar seed)

{
    WISPFUN_DEBUG("c139_f10");
    socket.Send(seed, 4);
}

bool CConnectionManager::Connect(const string &address, int port, puchar gameSeed)
{
    WISPFUN_DEBUG("c139_f11");
    if (m_IsLoginSocket)
    {
        if (m_LoginSocket.Connected)
            return true;

        bool result = m_LoginSocket.Connect(address, port);

        if (result)
        {
            g_TotalSendSize = 4;
            g_LastPacketTime = g_Ticks;
            g_LastSendTime = g_LastPacketTime;

            BYTE buf = 0xEF;
            m_LoginSocket.Send(&buf, 1);

            SendIP(m_LoginSocket, m_Seed);

            WISP_DATASTREAM::CDataWriter stream;

            string str = g_Orion.ClientVersionText;

            if (str.length())
            {
                char ver[20] = { 0 };
                char *ptr = ver;
                strncpy_s(ver, str.c_str(), str.length());
                int idx = 0;

                for (int i = 0; i < (int)str.length(); i++)
                {
                    if (ver[i] == '.')
                    {
                        ver[i] = 0;
                        stream.WriteUInt32BE(atoi(ptr));
                        ptr = ver + (i + 1);
                        idx++;

                        if (idx > 3)
                            break;
                    }
                }

                stream.WriteUInt32BE(atoi(ptr));
            }

            g_TotalSendSize = 21;
            m_LoginSocket.Send(stream.Data());
        }
        else
            m_LoginSocket.Disconnect();

        return result;
    }
    else
    {
        if (m_GameSocket.Connected)
            return true;

        g_TotalSendSize = 4;
        g_LastPacketTime = g_Ticks;
        g_LastSendTime = g_LastPacketTime;

        bool result = m_GameSocket.Connect(address, port);

        if (result)
            SendIP(m_GameSocket, gameSeed);

        m_LoginSocket.Disconnect();

        return result;
    }

    return false;
}

void CConnectionManager::Disconnect()
{
    WISPFUN_DEBUG("c139_f12");

    if (m_LoginSocket.Connected)
        m_LoginSocket.Disconnect();

    if (m_GameSocket.Connected)
        m_GameSocket.Disconnect();
}

void CConnectionManager::Recv()
{
    WISPFUN_DEBUG("c139_f13");
    if (m_IsLoginSocket)
    {
        if (!m_LoginSocket.Connected)
            return;

        if (!m_LoginSocket.ReadyRead())
        {
            if (m_LoginSocket.DataReady == SOCKET_ERROR)
            {
                LOG("Failed to Recv()...Disconnecting...\n");

                g_Orion.InitScreen(GS_MAIN_CONNECT);
                g_ConnectionScreen.SetType(CST_CONLOST);
            }
            return;
        }

        g_PacketManager.Read(&m_LoginSocket);
    }
    else
    {
        if (!m_GameSocket.Connected)
            return;

        if (!m_GameSocket.ReadyRead())
        {
            if (m_GameSocket.DataReady == SOCKET_ERROR)
            {
                LOG("Failed to Recv()...Disconnecting...\n");

                if (g_GameState == GS_GAME ||
                    (g_GameState == GS_GAME_BLOCKED && g_GameBlockedScreen.Code))
                    g_Orion.DisconnectGump();
                else
                {
                    g_Orion.InitScreen(GS_MAIN_CONNECT);
                    g_ConnectionScreen.SetType(CST_CONLOST);
                }
            }

            return;
        }

        g_PacketManager.Read(&m_GameSocket);
    }
}

int CConnectionManager::Send(puchar buf, int size)
{
    WISPFUN_DEBUG("c139_f14");
    if (g_TheAbyss)
    {
        switch (buf[0])
        {
            case 0x34:
                buf[0] = 0x71;
                break;
            case 0x72:
                buf[0] = 0x6C;
                break;
            case 0x6C:
                buf[0] = 0x72;
                break;
            case 0x3B:
                buf[0] = 0x34;
                break;
            case 0x6F:
                buf[0] = 0x56;
                break;
            case 0x56:
                buf[0] = 0x6F;
                break;
            default:
                break;
        }
    }
    else if (g_Asmut)
    {
        if (buf[0] == 0x02)
            buf[0] = 0x04;
        else if (buf[0] == 0x07)
            buf[0] = 0x0A;
    }

    if (m_IsLoginSocket)
    {
        if (!m_LoginSocket.Connected)
            return 0;

        return m_LoginSocket.Send(buf, size);
    }
    else
    {
        if (!m_GameSocket.Connected)
            return 0;

        return m_GameSocket.Send(buf, size);
    }

    return 0;
}

int CConnectionManager::Send(const UCHAR_LIST &data)
{
    WISPFUN_DEBUG("c139_f15");
    return Send((puchar)&data[0], (int)data.size());
}
