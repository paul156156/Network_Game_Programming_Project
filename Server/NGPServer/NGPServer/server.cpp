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
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 소켓 생성
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

    // 소켓통신에 사용할 변수
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
        printf("\n클라이언트 연결 성공! : IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

        // 클라이언트 세션 생성해서 컨테이너에 적재
        SESSION client_session(cid, client_sock);
        client_session.SetPosition(0.f,0.f,0.f);        /// 시작 포지션 적재하기
        client_session.SetDir(0);
        clients.emplace(cid, client_session);

        //// 해당 클라이언트 아이디 전송
        //SendID(client_session);


        hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)clients[cid].GetID(), 0, NULL);
        if (hThread == NULL) {
            cout << cid << "의 클라이언트 NULL" << endl;
            closesocket(clients[cid].GetSocket());
        }
        else { CloseHandle(hThread); }

        if (clients.size() == 1) {

            printf("====== 모든 클라이언트 접속 완료! ======\n");
            break;
        }
        else
            ++cid;
    }

    // 임계 영역
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
