/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <Winsock2.h>
#include <ws2tcpip.h>

#include "sm_files_tools.h"
#include "sm_log.h"

// default directory, where log file will be saved
#define DEF_LOG_DPATH               SM_BIN_PATH_MARKER SM_FILES_DIR_SLASH "."		//  default directory path, where log file will be created (current directory)

// file name of log file
#define DEF_LOG_FNAME               "socket_client_vs.log"							//  log file name

#define DEF_UDP_TYPE	0 
#define DEF_TCP_TYPE	1 

#define DEF_BUFF_SIZE   1024

errno_t sm_init_log();
errno_t sm_close_log();

int send_data_tcp(char* const address, const int port, char* const send_message);
int send_data_udp(char* const address, const int port, char* const send_message);
void* get_in_addr(struct sockaddr* sa);
