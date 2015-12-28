#include "precomp.h"
#include "../constants.h"
#include "SocketServer.h"

SocketServer::SocketServer(void) {
    isConnected = FALSE;
    fromlen = sizeof(from);

    int wsaret = WSAStartup(0x101,&wsaData);

    // Populate the sockaddr_in structure
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons((u_short)TCP_LISTEN_PORT);

    // Creates our socket, exiting on failure
    server = socket(AF_INET,SOCK_STREAM,0);

    // Bind socket to port
    bind(server,(sockaddr*)&local,sizeof(local));

    // Listen for incoming connections from clients
    listen(server,10);
}

SocketServer::~SocketServer(void) {
    // Close the socket, release the socket descriptor, and clean up
    closesocket(server);
    WSACleanup();
}

void SocketServer::WaitForConnection() {

    // Block until a client has connected
    client=accept(server, (struct sockaddr*)&from, &fromlen);
    isConnected = TRUE;
//    printf("Connection from %s\n", inet_ntoa(from.sin_addr));
}

void SocketServer::CloseConnection() {
    // Close the client socket
    closesocket(client);
    isConnected = FALSE;
}

void SocketServer::SendData(char* data) {
    if (!isConnected) return;
    // Send to the client and check if the client is still there
    int bytesSent = send(client,data,strlen(data)+1,0);
    if (bytesSent == SOCKET_ERROR) {
//        printf("Client disconnected\n");
        CloseConnection();
    }
}

BOOL SocketServer::IsConnected() {
    return isConnected;
}
