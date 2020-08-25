#pragma once

#define SERVER_PORT 3501
#define MAX_BUFFER	16384

#define CS_MOVE_UP		1
#define CS_MOVE_DOWN	2
#define CS_MOVE_LEFT	3
#define CS_MOVE_RIGHT	4

#define SC_POS			1

struct cs_test_packet
{
	int id;
};

struct CSPACKET_MOVE
{
	unsigned short size;
	unsigned short type;
};

struct SCPACKET_POS
{
	unsigned short size;
	unsigned short type;
	int id;
	short x, y;
};