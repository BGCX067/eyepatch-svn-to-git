#pragma once

class SocketServer
{
public:
    SocketServer(void);
    ~SocketServer(void);
    void SendData(char* data);
    void WaitForConnection();
    void CloseConnection();
    BOOL IsConnected();

private:
    SOCKET server, client;
    WSADATA wsaData;
    sockaddr_in local, from;
    int fromlen;
    BOOL isConnected;
};
