#pragma once
#pragma pack(push, 1)

/// Client
// �÷��̾��� ��ǥ ������ �Ͼ �� ��� Ŭ���̾�Ʈ���� �������ִ� ��Ŷ
struct CS_MOVE_PLAYER {
	char   type = 0;
	/// Vec2   pos;
	float x, y, z;
};


/// Server
// �̵� ��Ŷ
struct SC_MOVE_PLAYER {						// type = 0
	char	type = 0;
	USHORT	client_id;
	/// Vec2   pos;
	float x, y, z;
};
