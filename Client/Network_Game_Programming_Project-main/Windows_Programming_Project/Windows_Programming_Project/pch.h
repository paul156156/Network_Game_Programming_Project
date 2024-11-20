#pragma once
// �̸� �����ϵ� ���
// ��� cpp������ �� ����� �����ϰ� �־����. �ȱ׷� ����.
#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����


// ��Ʈ��ũ ���� 
#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���
#include <tchar.h> // _T(), ...
#include <string.h> // strncpy(), ...

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

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

