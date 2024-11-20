#pragma once
// 미리 컴파일된 헤더
// 모든 cpp파일은 이 헤더를 포함하고 있어야함. 안그럼 에러.
#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기


// 네트워크 관련 
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#include <tchar.h> // _T(), ...
#include <string.h> // strncpy(), ...

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <wchar.h>
#include <cassert>
#include<vector>
#include <cmath>
#include "protocol.h"


using std::vector;

#include <string>
using std::string;
using std::wstring;


#include <map>
using std::map;
using std::make_pair;

using namespace std;

#define SERVERPORT			9000
extern const char* SERVERIP = "127.0.0.1";
extern CRITICAL_SECTION playerCS;






#define SQUARE(x) ((x)*(x))

void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);

