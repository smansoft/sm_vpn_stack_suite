
#pragma once

// default directory, where log file will be saved
#define DEF_LOG_DPATH               SM_BIN_PATH_MARKER SM_FILES_DIR_SLASH ".." SM_FILES_DIR_SLASH "log"     //  default directory path, where log file will be created (current directory)

// file name of log file
#define DEF_LOG_FNAME               "socket_server.log"                                              //  log file name

#define DEF_UDP_TYPE    0 
#define DEF_TCP_TYPE    1 

#define DEF_NUM_CONNECTIONS 10

#define DEF_BUFF_SIZE   1024

typedef struct _sm_connect_params
{
    int                         m_socket_connect;
    int                         m_port;
    struct  sockaddr_storage    m_client_addr;
    char                        m_data_in[DEF_BUFF_SIZE];    
    char                        m_data_out[DEF_BUFF_SIZE];
} sm_connect_params, * psm_connect_params;

int start_server_tcp(int port);
int start_server_udp(int port);

void* exchange_data_tcp(void *param);
void* exchange_data_udp(void *param);

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

errno_t sm_init_log();
errno_t sm_close_log();
