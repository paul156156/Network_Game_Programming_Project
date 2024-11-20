#pragma once
#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <iostream>
#include <vector>
#include <unordered_map>
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <iomanip>
#include <random>


#pragma comment(lib, "ws2_32") // ws2_32.lib 링크
using namespace std;
struct Vec2;
class SESSION;

extern unordered_map<USHORT, SESSION>      clients;

#define SERVERPORT 9000
#define BUFSIZE    16000
// 소켓 함수 오류 출력
void err_display(int errcode);
void err_display(const char* msg);
void err_quit(const char* msg);



struct Vec2
{
	float x;
	float y;

	Vec2() : x(0.f), y(0.f) {}
	Vec2(float _x, float _y) : x(_x), y(_y) {}
	Vec2(int _x, int _y) : x(_x), y(_y) {}
	Vec2(const POINT& pt) :x((float)pt.x), y((float)pt.y) {}

	Vec2& operator=(const Vec2& vec)
	{
		x = (float)vec.x;
		y = (float)vec.y;

		return *this;
	}
};

class SESSION {
private:
	USHORT	id;
	SOCKET	socket;
	/// Vec2	pos;			// 이동할 플레이어의 좌표값
	float x, y, z;
	char	dir;			// 캐릭터가 바라볼 방향
	USHORT	oth_cid;		// 타 클라이언트 id
	bool	mis_flag;
public:
	// Special()
	SESSION() = default;
	SESSION(USHORT _id, SOCKET& _socket);
	virtual ~SESSION();

	// Get&Set()
	/// const Vec2& GetPostion() const { return pos; }
	void SetPosition(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	const char& GetDir() const { return dir; }
	void SetDir(const char& _dir) { dir = _dir; }
	const USHORT& GetOth_ID() const { return oth_cid; }
	void SetOth_ID(const char& oth_id) { oth_cid = oth_id; }
	const bool& GetMisFlag() const { return mis_flag; }
	void SetMisFlag(bool _flag) { mis_flag = _flag; }
	// Socket Get()
	USHORT GetID() const { return id; };
	const SOCKET& GetSocket() const;

};


#include "protocol.h"

// Singleton 매크로
#define SINGLE(type) public: \
			static type* GetInst() \
			{	\
				static type mgr;\
				return &mgr; \
			}\
			private: \
				type(); \
				~type();


