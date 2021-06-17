
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "sm_files_tools.h"
#include "sm_log.h"

#include "socket_server.h"

sm_log_config gsm_log_config;           //  global instance of main log support structure
#define SM_LOG_CONFIG &gsm_log_config   //  just synonym: SM_LOG_CONFIG == &gsm_log_config - for usage in log api calls

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int err = 0;
    int port;
    int type = -1;
    int type_cmp;
    
    while (TRUE) {
        
        if (argc < 3) {
            printf("usage: %s <type> <port>\n"
                   "\n"
                   "options:\n"
                   "  type: tcp or udp\n"
                   "  port: port number\n"
                   "\n", argv[0]);
            err = 1;
            break;
        }

        sm_init_log();

        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "%s %s", __PRETTY_FUNCTION__, "---------------------------");
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "socket_server is started");
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "------------------");

        type_cmp = strncmp (argv[1], "udp", 3);    
        if(!type_cmp)
            type = DEF_UDP_TYPE;
        else {
            type_cmp = strncmp (argv[1], "tcp", 3);    
            if(!type_cmp)
                type = DEF_TCP_TYPE;    
        }
        
        if(type < 0) {
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "wrong type of server socket: %s", argv[1]);
            err = 1;
            break;
        }

        port = atoi(argv[2]);
        if(port < 0 && port > 65535) {
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "wrong port number: %s", argv[2]);            
            err = 1;
            break;
        }
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "===========================");
        
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "type : %d", type);
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "listen port : %d", port);        
        
        switch(type) {
            case DEF_TCP_TYPE:
                start_server_tcp(port);
                err = 0;
                break;
            case DEF_UDP_TYPE:
                start_server_udp(port);
                err = 0;
                break;
            default:
                sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "wrong type of server socket: %s", argv[1]);               
                err = 1;
                break;            
        }

        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "===========================");
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "------------------");
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "result = %d", err);
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "socket_server is finished");
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "%s %s", __PRETTY_FUNCTION__, "---------------------------");
        
        sm_close_log();
        
        break;
    }

    return err;
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

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

//-----------------------------------------------------------------------------

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//-----------------------------------------------------------------------------

void* exchange_data_tcp(void *param)
{
    char client_addr_str[256];    

    int res_recv;
    int res_send;
    int res_send_src;

    sm_connect_params *connect_params = (sm_connect_params *) param;
    
    while(TRUE) {
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "------------");

        inet_ntop(connect_params->m_client_addr.ss_family,
            get_in_addr((struct sockaddr *)&connect_params->m_client_addr),
            client_addr_str, sizeof(client_addr_str));
            
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "got connection from %s", client_addr_str);            
        
        res_recv = recv(connect_params->m_socket_connect, connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1, 0);
        if(res_recv > 0) {
            connect_params->m_data_in[res_recv] = '\0';
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);  
        }
        else {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "receiving data - error...");              
            break;
        }
        
        strncpy (connect_params->m_data_out, "socket_server: Hello", sizeof(connect_params->m_data_out));  
        res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sending data: %s", connect_params->m_data_out);  
        res_send = send(connect_params->m_socket_connect, connect_params->m_data_out,  res_send_src, 0);
        if(res_send != res_send_src) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sending data - error...");
            break;
        }
        
        res_recv = recv(connect_params->m_socket_connect, connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1, 0);
        if(res_recv > 0) {
            connect_params->m_data_in[res_recv] = '\0';
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);
        }
        else {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "receiving data - error...");
            break;
        }
        
        res_send_src = strnlen(connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1);   
        for (int i = 0; i < res_send_src; i++)
            connect_params->m_data_out[i] = connect_params->m_data_in[res_send_src - i - 1];
        connect_params->m_data_out[res_send_src] = '\0';

        res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sending data: %s", connect_params->m_data_out);
        res_send = send(connect_params->m_socket_connect, connect_params->m_data_out,  res_send_src, 0);
        if(res_send != res_send_src) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sending data - error...");
            break;
        }

        res_recv = recv(connect_params->m_socket_connect, connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1, 0);
        if(res_recv > 0) {
            connect_params->m_data_in[res_recv] = '\0';
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);
        }
        else {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "receiving data - error...");
            break;
        }
        
        res_send_src = strnlen(connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1);   
        for (int i = 0; i < res_send_src; i++)
            connect_params->m_data_out[i] = connect_params->m_data_in[res_send_src - i - 1];
        connect_params->m_data_out[res_send_src] = '\0';
        
        res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sending data: %s", connect_params->m_data_out);
        res_send = send(connect_params->m_socket_connect, connect_params->m_data_out,  res_send_src, 0);
        if(res_send != res_send_src) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sending data - error...");
            break;
        }
    
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "exchange data is Ok");
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "------------");
        
        break;
    }
    close(connect_params->m_socket_connect);
    free((void*)connect_params);
    return NULL;
}

//-----------------------------------------------------------------------------

void* exchange_data_udp(void *param)
{
    char client_addr_str[256];
    
    struct addrinfo srv_addr_hints;
    struct addrinfo *srv_addr_result, *srv_addr_result_p; 
    
    int socket_connect;

    int res_recv;
    int res_send;
    int res_send_src;
    
    socklen_t sock_size;
    
    int err = 0;
    int res;
    int opt_value = 1; 

    char srv_addr[256];
    char srv_port[16];    

    sm_connect_params *connect_params = (sm_connect_params *) param;
    
    while(TRUE) {
        
        err = 0;
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "------------");        

        inet_ntop(connect_params->m_client_addr.ss_family,
            get_in_addr((struct sockaddr *)&connect_params->m_client_addr),
            client_addr_str, sizeof(client_addr_str));
            
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "got connection from %s", client_addr_str);
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sent data: %s", connect_params->m_data_out);
        
        snprintf(srv_port, sizeof(srv_port), "%d", connect_params->m_port); 
                
        memset(&srv_addr_hints, 0, sizeof(srv_addr_hints));
        
        srv_addr_hints.ai_family    = AF_UNSPEC;
        srv_addr_hints.ai_socktype  = SOCK_DGRAM;
        srv_addr_hints.ai_flags     = AI_PASSIVE;
        
        if ((res = getaddrinfo(NULL, srv_port, &srv_addr_hints, &srv_addr_result)) != 0) {
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "getaddrinfo: %s", gai_strerror(res));
            err = 1;
            break;
        }
        
        for(srv_addr_result_p = srv_addr_result; srv_addr_result_p != NULL; srv_addr_result_p = srv_addr_result_p->ai_next) {
            if ((socket_connect = socket(srv_addr_result_p->ai_family, srv_addr_result_p->ai_socktype, srv_addr_result_p->ai_protocol)) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "socket");
                continue;
            }
            if (setsockopt(socket_connect, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value)) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "setsockopt");
                err = 1;
            }
            inet_ntop(srv_addr_result_p->ai_family, get_in_addr((struct sockaddr*)srv_addr_result_p->ai_addr), srv_addr, sizeof(srv_addr));
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "binding to %s", srv_addr);
            if (bind(socket_connect, srv_addr_result_p->ai_addr, srv_addr_result_p->ai_addrlen) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "bind");
                close(socket_connect);
                continue;
            }
            break;
        } 
        
        if(err)
            break;  
        
        sock_size = sizeof (connect_params->m_client_addr);        
        res_recv = recvfrom(socket_connect, connect_params->m_data_in, sizeof(connect_params->m_data_in)-1, 0, (struct sockaddr *)&connect_params->m_client_addr, &sock_size);
        if(res_recv > 0) {
            connect_params->m_data_in[res_recv] = '\0';
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);
        }
        else {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "receiving data - error...");
            break;
        }        
        
        strncpy (connect_params->m_data_out, "socket_server: Hello", sizeof(connect_params->m_data_out));

        res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));
        sock_size =  sizeof(connect_params->m_client_addr);
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sending data: %s", connect_params->m_data_out);
        res_send = sendto(socket_connect, connect_params->m_data_out, res_send_src, 0, (struct sockaddr *)&connect_params->m_client_addr, sock_size);
        if(res_send != res_send_src) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sending data - error...");
            err = 1;
            break;
        }
        
        sock_size =  sizeof(connect_params->m_client_addr);
        res_recv = recvfrom(socket_connect, connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1, 0, (struct sockaddr *)&connect_params->m_client_addr, &sock_size);
        if(res_recv > 0) {
            connect_params->m_data_in[res_recv] = '\0';
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);
        }
        else {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "receiving data - error...");
            err = 1;
            break;
        }
        
        res_send_src = strnlen(connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1);
        for (int i = 0; i < res_send_src; i++)
            connect_params->m_data_out[i] = connect_params->m_data_in[res_send_src - i - 1];
        connect_params->m_data_out[res_send_src] = '\0';
        
        res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));    
        sock_size =  sizeof(connect_params->m_client_addr);
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sending data: %s", connect_params->m_data_out);
        res_send = sendto(socket_connect, connect_params->m_data_out, res_send_src, 0, (struct sockaddr *)&connect_params->m_client_addr, sock_size);
        if(res_send != res_send_src) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sending data - error...");
            err = 1;
            break;
        }

        sock_size =  sizeof(connect_params->m_client_addr);
        res_recv = recvfrom(socket_connect, connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1, 0, (struct sockaddr *)&connect_params->m_client_addr, &sock_size);
        if(res_recv > 0) {
            connect_params->m_data_in[res_recv] = '\0';
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "received data: %s", connect_params->m_data_in);
        }
        else {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "receiving data - error...");
            err = 1;
            break;
        }
        
        res_send_src = strnlen(connect_params->m_data_in, sizeof(connect_params->m_data_in) - 1);   
        for (int i = 0; i < res_send_src; i++)
            connect_params->m_data_out[i] = connect_params->m_data_in[res_send_src - i - 1];
        connect_params->m_data_out[res_send_src] = '\0';
        
        res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));    
        sock_size =  sizeof(connect_params->m_client_addr);
        sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "sending data: %s", connect_params->m_data_out);
        res_send = sendto(socket_connect, connect_params->m_data_out, res_send_src, 0, (struct sockaddr *)&connect_params->m_client_addr, sock_size);
        if(res_send != res_send_src) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sending data - error...");
            err = 1;
            break;
        }
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "exchange data is Ok");
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "------------");        
        
        break;
    }
    
    sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "err = %d", err); 
    
    close(socket_connect);
    free((void*)connect_params);
    return NULL;
}

//-----------------------------------------------------------------------------

int start_server_tcp(int port)
{
    struct addrinfo srv_addr_hints;
    struct addrinfo *srv_addr_result, *srv_addr_result_p; 

    struct sockaddr_storage client_addr;

    int     socket_srv;
    int     socket_connect;
    
    socklen_t sock_size;
    struct sigaction sa;

    char srv_addr[256];
    char srv_port[16];

    int err = 0;
    int res;
    int opt_value = 1;
    
    pthread_t exchange_data_thread;
    
    while(TRUE) {
        
        err = 0;
        
        snprintf(srv_port, sizeof(srv_port), "%d", port);    
        
        memset(&srv_addr_hints, 0, sizeof(srv_addr_hints));
        
        srv_addr_hints.ai_family    = AF_UNSPEC;
        srv_addr_hints.ai_socktype  = SOCK_STREAM;
        srv_addr_hints.ai_flags     = AI_PASSIVE;
        
        if ((res = getaddrinfo(NULL, srv_port, &srv_addr_hints, &srv_addr_result)) != 0) {
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "getaddrinfo: %s", gai_strerror(res));
            err = 1;
            break;
        }
        
        for(srv_addr_result_p = srv_addr_result; srv_addr_result_p != NULL; srv_addr_result_p = srv_addr_result_p->ai_next) {
            if ((socket_srv = socket(srv_addr_result_p->ai_family, srv_addr_result_p->ai_socktype, srv_addr_result_p->ai_protocol)) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "socket");
                continue;
            }
            if (setsockopt(socket_srv, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value)) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "setsockopt");                
                err = 1;
            }
            inet_ntop(srv_addr_result_p->ai_family, get_in_addr((struct sockaddr*)srv_addr_result_p->ai_addr), srv_addr, sizeof(srv_addr));
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "binding to %s", srv_addr);
            if (bind(socket_srv, srv_addr_result_p->ai_addr, srv_addr_result_p->ai_addrlen) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "bind");
                close(socket_srv);
                continue;
            }
            break;
        }
        
        if(err)
            break;        
        
        if (!srv_addr_result_p)  {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "failed to bind");
            err = 1;
            break;
        }
        
        freeaddrinfo(srv_addr_result);
        
        if (listen(socket_srv, DEF_NUM_CONNECTIONS) == -1) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "listen");
            err = 1;
            break;
        }
        
        sa.sa_handler = sigchld_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sigaction");
            err = 1;
            break;
        }
        
        while(TRUE) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "waiting for connections...");
            sock_size = sizeof (client_addr);
            socket_connect = accept(socket_srv, (struct sockaddr *)&client_addr, &sock_size);
            if (socket_connect == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "accept");
                continue;
            }
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "incoming connection is created...");
            
            sm_connect_params *connect_params = (sm_connect_params *)malloc(sizeof(sm_connect_params));
            connect_params->m_socket_connect = socket_connect;
            connect_params->m_client_addr = client_addr;
            
            pthread_create(&exchange_data_thread, NULL, exchange_data_tcp, connect_params);        
        }
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "all connections are closed...");
        
        break;
    }

    return err;
}

//-----------------------------------------------------------------------------

int start_server_udp(int port)
{
    struct addrinfo srv_addr_hints;
    struct addrinfo *srv_addr_result, *srv_addr_result_p; 

    struct sockaddr_storage client_addr;

    int     socket_srv;
    
    socklen_t sock_size;
    struct sigaction sa;

    char srv_addr[256];
    char srv_port[16];

    int err = 0;
    int res;
    int opt_value = 1;
    int connect_port = port;
    
    int res_recv;
    int res_send;
    int res_send_src;
    
    pthread_t exchange_data_thread;
    
    while(TRUE) {
        err = 0;
        
        snprintf(srv_port, sizeof(srv_port), "%d", port);    
        
        memset(&srv_addr_hints, 0, sizeof(srv_addr_hints));
        
        srv_addr_hints.ai_family    = AF_UNSPEC;
        srv_addr_hints.ai_socktype  = SOCK_DGRAM;
        srv_addr_hints.ai_flags     = AI_PASSIVE;
        
        if ((res = getaddrinfo(NULL, srv_port, &srv_addr_hints, &srv_addr_result)) != 0) {
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "getaddrinfo: %s", gai_strerror(res));
            err = 1;
            break;
        }
        
        for(srv_addr_result_p = srv_addr_result; srv_addr_result_p != NULL; srv_addr_result_p = srv_addr_result_p->ai_next) {
            if ((socket_srv = socket(srv_addr_result_p->ai_family, srv_addr_result_p->ai_socktype, srv_addr_result_p->ai_protocol)) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "socket");
                continue;
            }
            if (setsockopt(socket_srv, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value)) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "setsockopt");
                err = 1;
            }
            inet_ntop(srv_addr_result_p->ai_family, get_in_addr((struct sockaddr*)srv_addr_result_p->ai_addr), srv_addr, sizeof(srv_addr));
            sm_log_printf(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "binding to %s", srv_addr);
            if (bind(socket_srv, srv_addr_result_p->ai_addr, srv_addr_result_p->ai_addrlen) == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "bind");
                close(socket_srv);
                continue;
            }
            break;
        }
        
        if(err)
            break;        
        
        if (!srv_addr_result_p)  {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "failed to bind");
            err = 1;
            break;
        }
        
        freeaddrinfo(srv_addr_result);        
        
        sa.sa_handler = sigchld_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sigaction");
            err = 1;
            break;
        }
        
        while(TRUE) {
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "waiting for connections...");
            sm_connect_params *connect_params = (sm_connect_params *)malloc(sizeof(sm_connect_params));
            sock_size = sizeof (client_addr);        
            res_recv = recvfrom(socket_srv, connect_params->m_data_in, sizeof(connect_params->m_data_in)-1, 0, (struct sockaddr *)&client_addr, &sock_size);
            if (res_recv == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "recvfrom");
                err = 1;
                continue;            
            }
            
            sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "incoming connection is created...");
            
            connect_params->m_data_in[res_recv] = '\0';
            connect_params->m_socket_connect = socket_srv;
            connect_params->m_client_addr = client_addr;
            connect_port++;
            if(connect_port > (port + 10))
                connect_port = port + 1;
            connect_params->m_port = connect_port;
            
            snprintf(connect_params->m_data_out, sizeof(connect_params->m_data_out)-1, "%d", connect_params->m_port);
            res_send_src = strnlen(connect_params->m_data_out, sizeof(connect_params->m_data_out));
            sock_size = sizeof (client_addr);
            res_send = sendto(socket_srv, connect_params->m_data_out, res_send_src, 0, (struct sockaddr *)&client_addr, sock_size);
            if (res_send == -1) {
                sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_ERROR, __PRETTY_FUNCTION__, "sendto");
                err = 1;
                continue;            
            }        
            
            pthread_create(&exchange_data_thread, NULL, exchange_data_udp, connect_params);
        }
        
        sm_log_print(SM_LOG_CONFIG, SM_LOG_LEVEL_INFO, __PRETTY_FUNCTION__, "all connections are closed...");
        
        err = 0;
        break;
    }

    return err;
}

//-----------------------------------------------------------------------------

