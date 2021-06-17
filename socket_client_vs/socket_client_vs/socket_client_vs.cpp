/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

// socket_client_vs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "socket_client_vs.h"

sm_log_config gsm_log_config;           //  global instance of main log support structure
#define SM_LOG_CONFIG &gsm_log_config   //  just synonym: SM_LOG_CONFIG == &gsm_log_config - for usage in log api calls

//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	int err = 0;

	while (TRUE) {

		if (argc < 4) {
			printf("usage: %s <type> <address> <port>\n"
				"\n"
				"options:\n"
				"  type: tcp or udp\n"
				"  address: address of the server\n"
				"  port: port number of the server\n"
				"\n", argv[0]);
			err = 1;
			break;
		}

		sm_init_log();

		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "%s %s", __FUNCTION__, "---------------------------");
		sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "socket_client is started");
		sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "------------------");

		int type = -1;
		char address[DEF_BUFF_SIZE];
		int port = -1;

		memset((void*)address, 0, sizeof(address));

		int type_cmp = strncmp(argv[1], "udp", 3);
		if (!type_cmp)
			type = DEF_UDP_TYPE;
		else {
			type_cmp = strncmp(argv[1], "tcp", 3);
			if (!type_cmp)
				type = DEF_TCP_TYPE;
		}

		if (type < 0) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "wrong type of server socket: %s", argv[1]);
			err = 1;
			break;
		}

		strncpy_s(address, sizeof(address), argv[2], _TRUNCATE);

		port = atoi(argv[3]);
		if (port < 0 && port > 65535) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "wrong port number: %s", argv[3]);
			err = 1;
			break;
		}

		sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "===========================");

		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "type : %d", type);
		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "server address : %s", address);
		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "server port : %d", port);

		WSADATA      wsa_data;

		err = WSAStartup(0x202, &wsa_data);
		if (err) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "error initializing the socket library...");
			err = 1;
			break;
		}

		char message[DEF_BUFF_SIZE];

		printf("\nenter message: ");
		gets_s(message, (sizeof(message) - 1));

		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "message : %s", message);

		switch (type) {
		case DEF_TCP_TYPE:
			err = send_data_tcp(address, port, message);
			break;
		case DEF_UDP_TYPE:
			err = send_data_udp(address, port, message);
			break;
		default:
			err = -1;
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "wrong type of server socket: %d", type);
			break;
		}
		if(!err)
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "result sending data: Ok");
		else
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "result sending data: Error");

		WSACleanup();

		sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "===========================");

		sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "------------------");
		sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "socket_client is finished");
		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "%s %s", __FUNCTION__, "---------------------------");

		sm_close_log();

		break;
	}

	return err;
}

//-----------------------------------------------------------------------------

int send_data_tcp(char * const address, const int port, char* const send_message)
{
	struct addrinfo srv_addr_hints;
	struct addrinfo *srv_addr_result, *srv_addr_result_p;

	char srv_addr[256];
	char srv_port[16];

	int err = 0;
	int res;

	int res_recv;
	int res_send;
	int res_send_src;

	SOCKET       socket_cln;

	char data_in[DEF_BUFF_SIZE];
	char data_out[DEF_BUFF_SIZE];

	sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "------------");

	while (TRUE) {
		err = 0;
		memset(&srv_addr_hints, 0, sizeof(srv_addr_hints));

		srv_addr_hints.ai_family = AF_UNSPEC;
		srv_addr_hints.ai_socktype = SOCK_STREAM;
		srv_addr_hints.ai_flags = AI_PASSIVE;
		srv_addr_hints.ai_protocol = 0;
		srv_addr_hints.ai_canonname = NULL;
		srv_addr_hints.ai_addr = NULL;
		srv_addr_hints.ai_next = NULL;

		snprintf(srv_port, sizeof(srv_port), "%d", port);

		res = getaddrinfo(address, srv_port, &srv_addr_hints, &srv_addr_result);
		if (res) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "getaddrinfo: %s", gai_strerror(res));
			err = 1;
			break;
		}

		for (srv_addr_result_p = srv_addr_result; srv_addr_result_p != NULL; srv_addr_result_p = srv_addr_result_p->ai_next) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "addrinfo -------");
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_family		= %d", srv_addr_result_p->ai_family);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_socktype	= %d", srv_addr_result_p->ai_socktype);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_protocol	= %d", srv_addr_result_p->ai_protocol);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_canonname	= %d", srv_addr_result_p->ai_canonname);
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_addr->sa_data = ");
			for (int i = 0; i < sizeof(srv_addr_result_p->ai_addr->sa_data); i++)
				sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "%u(0x%x)", (unsigned char)srv_addr_result_p->ai_addr->sa_data[i], (unsigned char)srv_addr_result_p->ai_addr->sa_data[i]);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->sa_family	= %d", srv_addr_result_p->ai_addr->sa_family);
			inet_ntop(srv_addr_result_p->ai_family, get_in_addr((struct sockaddr*)srv_addr_result_p->ai_addr), srv_addr, sizeof(srv_addr));
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "client: connecting to %s", srv_addr);
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "addrinfo -------");
		}

		socket_cln = socket(PF_INET, SOCK_STREAM, 0);

		if (socket_cln == INVALID_SOCKET) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "socket wasn't created...");
			err = 1;
			break;
		}

		res = connect(socket_cln, (struct sockaddr*)srv_addr_result->ai_addr, srv_addr_result->ai_addrlen);

		if (res) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "connection creating error...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), "socket_client: Hello", _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sending data: %s", data_out);
		res_send = send(socket_cln, data_out, res_send_src, 0);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recv(socket_cln, data_in, (sizeof(data_in) - 1), 0);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), send_message, _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		res_send = send(socket_cln, data_out, res_send_src, 0);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recv(socket_cln, data_in, (sizeof(data_in) - 1), 0);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), data_in, _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		res_send = send(socket_cln, data_out, res_send_src, 0);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recv(socket_cln, data_in, (sizeof(data_in) - 1), 0);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		freeaddrinfo(srv_addr_result);
		closesocket(socket_cln);

		err = 0;

		break;
	}

	sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "------------");

	return err;
}

//-----------------------------------------------------------------------------

int send_data_udp(char* const address, const int port, char* const send_message)
{
	struct addrinfo srv_addr_hints;
	struct addrinfo* srv_addr_result, * srv_addr_result_p;

	char srv_addr[256];
	char srv_port[16];

	int err = 0;
	int res;

	int res_recv;
	int res_send;
	int res_send_src;

	int connect_port;

	SOCKET       socket_cln;

	char data_in[DEF_BUFF_SIZE];
	char data_out[DEF_BUFF_SIZE];

	sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "------------");

	while (TRUE) {
		err = 0;
		memset(&srv_addr_hints, 0, sizeof(srv_addr_hints));

		srv_addr_hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		srv_addr_hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
		srv_addr_hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
		srv_addr_hints.ai_protocol = 0;          /* Any protocol */
		srv_addr_hints.ai_canonname = NULL;
		srv_addr_hints.ai_addr = NULL;
		srv_addr_hints.ai_next = NULL;

		snprintf(srv_port, sizeof(srv_port), "%d", port);

		res = getaddrinfo(address, srv_port, &srv_addr_hints, &srv_addr_result);
		if (res) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "getaddrinfo: %s", gai_strerror(res));
			err = 1;
			break;
		}

		for (srv_addr_result_p = srv_addr_result; srv_addr_result_p != NULL; srv_addr_result_p = srv_addr_result_p->ai_next) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "addrinfo -------");
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_family		= %d", srv_addr_result_p->ai_family);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_socktype	= %d", srv_addr_result_p->ai_socktype);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_protocol	= %d", srv_addr_result_p->ai_protocol);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_canonname	= %d", srv_addr_result_p->ai_canonname);
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_addr->sa_data = ");
			for (int i = 0; i < sizeof(srv_addr_result_p->ai_addr->sa_data); i++)
				sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "%u(0x%x)", (unsigned char)srv_addr_result_p->ai_addr->sa_data[i], (unsigned char)srv_addr_result_p->ai_addr->sa_data[i]);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->sa_family	= %d", srv_addr_result_p->ai_addr->sa_family);
			inet_ntop(srv_addr_result_p->ai_family, get_in_addr((struct sockaddr*)srv_addr_result_p->ai_addr), srv_addr, sizeof(srv_addr));
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "client: connecting to %s", srv_addr);
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "addrinfo -------");
		}

		socket_cln = socket(PF_INET, SOCK_DGRAM, 0);

		if (socket_cln == INVALID_SOCKET) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "socket wasn't created...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), "socket_client: Hello", _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sending data: %s", data_out);
		res_send = sendto(socket_cln, data_out, res_send_src, 0, (struct sockaddr*)srv_addr_result->ai_addr, srv_addr_result->ai_addrlen);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recvfrom(socket_cln, data_in, (sizeof(data_in) - 1), 0, (struct sockaddr*)srv_addr_result->ai_addr, (int*)&srv_addr_result->ai_addrlen);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		freeaddrinfo(srv_addr_result);
		closesocket(socket_cln);

		connect_port = atoi(data_in);
		if (connect_port < 0 || connect_port > 65535) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "wrong port number");
			err = 1;
			break;
		}

		memset(&srv_addr_hints, 0, sizeof(srv_addr_hints));

		srv_addr_hints.ai_family = AF_UNSPEC;
		srv_addr_hints.ai_socktype = SOCK_DGRAM;
		srv_addr_hints.ai_flags = AI_PASSIVE;
		srv_addr_hints.ai_protocol = 0;
		srv_addr_hints.ai_canonname = NULL;
		srv_addr_hints.ai_addr = NULL;
		srv_addr_hints.ai_next = NULL;

		snprintf(srv_port, sizeof(srv_port), "%d", connect_port);

		res = getaddrinfo(address, srv_port, &srv_addr_hints, &srv_addr_result);
		if (res) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "getaddrinfo: %s", gai_strerror(res));
			err = 1;
			break;
		}

		for (srv_addr_result_p = srv_addr_result; srv_addr_result_p != NULL; srv_addr_result_p = srv_addr_result_p->ai_next) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "addrinfo -------");
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_family		= %d", srv_addr_result_p->ai_family);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_socktype	= %d", srv_addr_result_p->ai_socktype);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_protocol	= %d", srv_addr_result_p->ai_protocol);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_canonname	= %d", srv_addr_result_p->ai_canonname);
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->ai_addr->sa_data = ");
			for (int i = 0; i < sizeof(srv_addr_result_p->ai_addr->sa_data); i++)
				sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "%u(0x%x)", (unsigned char)srv_addr_result_p->ai_addr->sa_data[i], (unsigned char)srv_addr_result_p->ai_addr->sa_data[i]);
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "srv_addr_result_p->sa_family	= %d", srv_addr_result_p->ai_addr->sa_family);
			inet_ntop(srv_addr_result_p->ai_family, get_in_addr((struct sockaddr*)srv_addr_result_p->ai_addr), srv_addr, sizeof(srv_addr));
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "client: connecting to %s", srv_addr);
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "addrinfo -------");
		}

		socket_cln = socket(PF_INET, SOCK_DGRAM, 0);

		if (socket_cln == INVALID_SOCKET) {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "socket wasn't created...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), "socket_client: Hello", _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sending data: %s", data_out);
		res_send = sendto(socket_cln, data_out, res_send_src, 0, (struct sockaddr*)srv_addr_result->ai_addr, srv_addr_result->ai_addrlen);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recvfrom(socket_cln, data_in, (sizeof(data_in) - 1), 0, (struct sockaddr*)srv_addr_result->ai_addr, (int*)&srv_addr_result->ai_addrlen);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), send_message, _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		res_send = sendto(socket_cln, data_out, res_send_src, 0, (struct sockaddr*)srv_addr_result->ai_addr, srv_addr_result->ai_addrlen);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recvfrom(socket_cln, data_in, (sizeof(data_in) - 1), 0, (struct sockaddr*)srv_addr_result->ai_addr, (int*)&srv_addr_result->ai_addrlen);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		strncpy_s(data_out, sizeof(data_out), data_in, _TRUNCATE);

		res_send_src = strnlen_s(data_out, sizeof(data_out));
		res_send = sendto(socket_cln, data_out, res_send_src, 0, (struct sockaddr*)srv_addr_result->ai_addr, srv_addr_result->ai_addrlen);
		if (res_send == res_send_src) {
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "sent data: %s", data_out);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "sending data - error...");
			err = 1;
			break;
		}

		res_recv = recvfrom(socket_cln, data_in, (sizeof(data_in) - 1), 0, (struct sockaddr*)srv_addr_result->ai_addr, (int*)&srv_addr_result->ai_addrlen);
		if (res_recv > 0) {
			data_in[res_recv] = '\0';
			sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __FUNCTION__, "received data: %s", data_in);
		}
		else {
			sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __FUNCTION__, "receiving data - error...");
			err = 1;
			break;
		}

		freeaddrinfo(srv_addr_result);
		closesocket(socket_cln);

		err = 0;

		break;
	}

	printf("------------\n");

	return err;
}

//-----------------------------------------------------------------------------

void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	else if (sa->sa_family == AF_INET6)
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	else
		return NULL;
}

//-----------------------------------------------------------------------------

/**
 *  initializing and starting the log output
 */
errno_t sm_init_log()
{
	errno_t err = SM_RES_OK;
	err = sm_log_init_dpath_fname(SM_LOG_CONFIG,
		SM_LOG_OUT_FILE | SM_LOG_OUT_CONSOLE,
		SM_LOG_LEVEL_OFF,
		SM_LOG_LEVEL_ALL,
		DEF_LOG_DPATH,
		DEF_LOG_FNAME);
#if 0
	err = sm_log_init_dpath_fname(SM_LOG_CONFIG,
		SM_LOG_OUT_FILE | SM_LOG_OUT_CONSOLE,
		SM_LOG_LEVEL_OFF,
		SM_LOG_LEVEL_INFO,
		DEF_LOG_DPATH,
		DEF_LOG_FNAME);
#endif
	if (err == SM_RES_OK)
		err = sm_log_start(SM_LOG_CONFIG);  // starting created log
	return err;
}

//-----------------------------------------------------------------------------

/**
 *  stopping and closing the log output
 */
errno_t sm_close_log()
{
	sm_log_stop(SM_LOG_CONFIG);
	return sm_log_close(SM_LOG_CONFIG);
}

//-----------------------------------------------------------------------------
