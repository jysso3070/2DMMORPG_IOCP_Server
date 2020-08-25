#pragma once

#include <chrono>
#include "Protocol.h"

enum class EVENT_TYPE
{
	EV_RECV, EV_SEND, EV_TEST
};

struct OVER_EX
{
	WSAOVERLAPPED over;
	WSABUF wsabuf[1];
	char net_buf[MAX_BUFFER];
	EVENT_TYPE eventType;
	int ex_key;
};

struct CLIENT_INFO
{
	OVER_EX over_ex;
	SOCKET socket;
	bool is_connect;
	unsigned int id;
	short x, y;
};

struct EVENT
{
	int obj_id;
	int target_obj;
	std::chrono::high_resolution_clock::time_point wakeup_time;
	EVENT_TYPE event_type;
	constexpr bool operator < (const EVENT& left) const {
		return wakeup_time > left.wakeup_time;
	}
};