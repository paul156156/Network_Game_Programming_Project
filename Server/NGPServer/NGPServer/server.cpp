#include "pch.h"
#include "protocol.h"

USHORT                              cid;
USHORT                              mid;
CRITICAL_SECTION                    clientsCS;
unordered_map<USHORT, SESSION>      clients;

DWORD WINAPI ClientThread(LPVOID arg);

bool SendPlayerMove(USHORT _id, const CS_MOVE_PLAYER& _packet);

int main()
{
    InitializeCriticalSection(&clientsCS);

    int retval{};
    cid = 0;
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // ���� ����
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET)
        err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
        err_quit("bind()");

    printf("listen()..\n");
    // listen()
    retval = listen(listen_socket, SOMAXCONN);
    if (retval == SOCKET_ERROR)
        err_quit("listen()");

    // ������ſ� ����� ����
    SOCKET                  client_sock;
    struct sockaddr_in      clientaddr;
    int                     addrlen;
    char                    buf[BUFSIZE + 1];
    HANDLE                  hThread;
    char                    addr[INET_ADDRSTRLEN];

    addrlen = sizeof(clientaddr);
    while (true) {
        cout << "accept..." << endl;

        // client accept()
        client_sock = accept(listen_socket, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        printf("\nŬ���̾�Ʈ ���� ����! : IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", addr, ntohs(clientaddr.sin_port));

        // Ŭ���̾�Ʈ ���� �����ؼ� �����̳ʿ� ����
        SESSION client_session(cid, client_sock);
        client_session.SetPosition(0.f,0.f,0.f);        /// ���� ������ �����ϱ�
        client_session.SetDir(0);
        clients.emplace(cid, client_session);

        //// �ش� Ŭ���̾�Ʈ ���̵� ����
        //SendID(client_session);


        hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)clients[cid].GetID(), 0, NULL);
        if (hThread == NULL) {
            cout << cid << "�� Ŭ���̾�Ʈ NULL" << endl;
            closesocket(clients[cid].GetSocket());
        }
        else { CloseHandle(hThread); }

        if (clients.size() == 1) {

            printf("====== ��� Ŭ���̾�Ʈ ���� �Ϸ�! ======\n");
            break;
        }
        else
            ++cid;
    }

    // �Ӱ� ����
    DeleteCriticalSection(&clientsCS);
}

DWORD __stdcall ClientThread(LPVOID arg)
{
    return 0;
}

bool SendPlayerMove(USHORT _id, const CS_MOVE_PLAYER& _packet)
{
    SC_MOVE_PLAYER packet;
    clients[_id].SetPosition(packet.x,packet.y,packet.z);

    packet.client_id = _id;
    packet.x = _packet.x;
    packet.y = _packet.y;
    packet.z = _packet.z;

    for (auto& [id, session] : clients) {
        if (id == _id)
            continue;

        int retval = send(session.GetSocket(), (char*)&packet, sizeof(packet), 0);
        if (retval == SOCKET_ERROR) {
            err_display("Send Player Move()");
            return false;
        }
    }

    return true;
}
