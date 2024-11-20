#pragma once
#pragma pack(push, 1)

/// Client
// 플레이어의 좌표 갱신이 일어날 때 상대 클라이언트에게 전송해주는 패킷
struct CS_MOVE_PLAYER {
	char   type = 0;
	/// Vec2   pos;
	float x, y, z;
};


/// Server
// 이동 패킷
struct SC_MOVE_PLAYER {						// type = 0
	char	type = 0;
	USHORT	client_id;
	/// Vec2   pos;
	float x, y, z;
};
