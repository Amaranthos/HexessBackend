#ifndef PACKET_HPP
#define PACKET_HPP

#pragma pack(push, 1)

//
class Packet {
	enum {
		E_ID = 1,
		E_START,
		E_MOVE,
		E_END,
		E_MSG
	};

	int type;
	int id;
};

#pragma pack(pop)

#endif // PACKET_HPP