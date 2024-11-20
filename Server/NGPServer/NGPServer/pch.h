#pragma once
#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <iostream>
#include <vector>
#include <unordered_map>
#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���
#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <iomanip>
#include <random>


#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ
using namespace std;
struct Vec2;
class SESSION;

extern unordered_map<USHORT, SESSION>      clients;

#define SERVERPORT 9000
#define BUFSIZE    16000
// ���� �Լ� ���� ���
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
	/// Vec2	pos;			// �̵��� �÷��̾��� ��ǥ��
	float x, y, z;
	char	dir;			// ĳ���Ͱ� �ٶ� ����
	USHORT	oth_cid;		// Ÿ Ŭ���̾�Ʈ id
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

// Singleton ��ũ��
#define SINGLE(type) public: \
			static type* GetInst() \
			{	\
				static type mgr;\
				return &mgr; \
			}\
			private: \
				type(); \
				~type();


