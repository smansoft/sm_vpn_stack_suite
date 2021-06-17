
#include "vpn.h"

extern sm_log_config gsm_log_config;    //  global instance of main log support structure
#define SM_LOG_CONFIG &gsm_log_config   //  just synonym: SM_LOG_CONFIG == &gsm_log_config - for usage in log api calls

extern JavaVM *jvm;
extern int pipefds[2];
extern pthread_t thread_id_handle_events;
extern pthread_mutex_t lock;
extern int stop_threads;

struct vpn_session *g_vpn_session = NULL;

void init(const struct arguments *args) {
	g_vpn_session = NULL;
}

void clear() {
    struct vpn_session *s = g_vpn_session;
    while (s != NULL) {
        if (s->socket >= 0 && close(s->socket))
            log_android(ANDROID_LOG_ERROR, __FUNCTION__,"close %d error %d: %s",
                        s->socket, errno, strerror(errno));
        if (s->protocol == IPPROTO_TCP)
            clear_tcp_data(&s->tcp);
        struct vpn_session *p = s;
        s = s->next;
        free(p);
    }
    g_vpn_session = NULL;
}

void *handle_events(void *a) {
    struct arguments *args = (struct arguments *) a;
    log_android(ANDROID_LOG_WARN, __FUNCTION__, "Start events tun=%d thread %x", args->tun, thread_id_handle_events);

    // Attach to Java
    JNIEnv *env;
    jint rs = (*jvm)->AttachCurrentThread(jvm, &env, NULL);
    if (rs != JNI_OK) {
        log_android(ANDROID_LOG_ERROR, __FUNCTION__,"AttachCurrentThread failed");
        return NULL;
    }
    args->env = env;

    // Get max sessions
    int maxsessions = 1024;
    struct rlimit rlim;
    if (getrlimit(RLIMIT_NOFILE, &rlim))
        log_android(ANDROID_LOG_WARN, __FUNCTION__,"getrlimit error %d: %s", errno, strerror(errno));
    else
        log_android(ANDROID_LOG_WARN, __FUNCTION__, "getrlimit soft %d hard %d max sessions %d",
                    rlim.rlim_cur, rlim.rlim_max, maxsessions);
    maxsessions = (int) (rlim.rlim_cur * SESSION_LIMIT / 100);
    if (maxsessions > 1000)
        maxsessions = 1000;

    // Terminate existing sessions not allowed anymore
    check_allowed(args);

    // Open epoll file
    int epoll_fd = epoll_create(1);
    if (epoll_fd < 0) {
        log_android(ANDROID_LOG_ERROR, __FUNCTION__,"epoll create error %d: %s", errno, strerror(errno));
        stop_threads = 1;
    }

    // Monitor stop events
    struct epoll_event ev_pipe;
    memset(&ev_pipe, 0, sizeof(struct epoll_event));
    ev_pipe.events = EPOLLIN | EPOLLERR;
    ev_pipe.data.ptr = &ev_pipe;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipefds[0], &ev_pipe)) {
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "epoll add pipe error %d: %s", errno, strerror(errno));
        stop_threads = 1;
    }

    // Monitor tun events
    struct epoll_event ev_tun;
    memset(&ev_tun, 0, sizeof(struct epoll_event));
    ev_tun.events = EPOLLIN | EPOLLERR;
    ev_tun.data.ptr = NULL;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, args->tun, &ev_tun)) {
        log_android(ANDROID_LOG_ERROR, __FUNCTION__,"epoll add tun error %d: %s", errno, strerror(errno));
        stop_threads = 1;
    }

    // Loop
    long long last_check = 0;
    while (!stop_threads) {
        log_android(ANDROID_LOG_DEBUG, __FUNCTION__, "Loop thread %x", thread_id_handle_events);

        int recheck = 0;
        int timeout = EPOLL_TIMEOUT;

        // Count sessions
        int isessions = 0;
        int usessions = 0;
        int tsessions = 0;
        if (pthread_mutex_lock(&lock))
            log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_lock failed");
        struct vpn_session *s = g_vpn_session;
        while (s != NULL) {
            if (s->protocol == IPPROTO_ICMP || s->protocol == IPPROTO_ICMPV6) {
                if (!s->icmp.stop)
                    isessions++;
            } else if (s->protocol == IPPROTO_UDP) {
                if (s->udp.state == UDP_ACTIVE)
                    usessions++;
            } else if (s->protocol == IPPROTO_TCP) {
                if (s->tcp.state != TCP_CLOSING && s->tcp.state != TCP_CLOSE)
                    tsessions++;
                if (s->socket >= 0)
                    recheck = recheck | monitor_tcp_session(args, s, epoll_fd);
            }
            s = s->next;
        }
        if (pthread_mutex_unlock(&lock))
            log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_unlock failed");
        int sessions = isessions + usessions + tsessions;

        // Check sessions
        long long ms = get_ms();
        if (ms - last_check > EPOLL_MIN_CHECK) {
            last_check = ms;

            if (pthread_mutex_lock(&lock))
                log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_lock failed");

            time_t now = time(NULL);
            struct vpn_session *sl = NULL;
            s = g_vpn_session;
            while (s != NULL) {
                int del = 0;
                if (s->protocol == IPPROTO_ICMP || s->protocol == IPPROTO_ICMPV6) {
                    del = check_icmp_session(args, s, sessions, maxsessions);
                    if (!s->icmp.stop && !del) {
                        int stimeout = s->icmp.time +
                                       get_icmp_timeout(&s->icmp, sessions, maxsessions) - now + 1;
                        if (stimeout > 0 && stimeout < timeout)
                            timeout = stimeout;
                    }
                } else if (s->protocol == IPPROTO_UDP) {
                    del = check_udp_session(args, s, sessions, maxsessions);
                    if (s->udp.state == UDP_ACTIVE && !del) {
                        int stimeout = s->udp.time +
                                       get_udp_timeout(&s->udp, sessions, maxsessions) - now + 1;
                        if (stimeout > 0 && stimeout < timeout)
                            timeout = stimeout;
                    }
                } else if (s->protocol == IPPROTO_TCP) {
                    del = check_tcp_session(args, s, sessions, maxsessions);
                    if (s->tcp.state != TCP_CLOSING && s->tcp.state != TCP_CLOSE && !del) {
                        int stimeout = s->tcp.time +
                                       get_tcp_timeout(&s->tcp, sessions, maxsessions) - now + 1;
                        if (stimeout > 0 && stimeout < timeout)
                            timeout = stimeout;
                    }
                }

                if (del) {
                    if (sl == NULL)
                        g_vpn_session = s->next;
                    else
                        sl->next = s->next;

                    struct vpn_session *c = s;
                    s = s->next;
                    if (c->protocol == IPPROTO_TCP)
                        clear_tcp_data(&c->tcp);
                    free(c);
                } else {
                    sl = s;
                    s = s->next;
                }
            }
            if (pthread_mutex_unlock(&lock))
                log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_unlock failed");
        } else {
            recheck = 1;
            log_android(ANDROID_LOG_DEBUG, __FUNCTION__, "Skipped session checks");
        }

        log_android(ANDROID_LOG_DEBUG,
                    __FUNCTION__,
                    "sessions ICMP %d UDP %d TCP %d max %d/%d timeout %d recheck %d",
                    isessions, usessions, tsessions, sessions, maxsessions, timeout, recheck);

        // Poll
        struct epoll_event ev[EPOLL_EVENTS];
        int ready = epoll_wait(epoll_fd, ev, EPOLL_EVENTS,
                               recheck ? EPOLL_MIN_CHECK : timeout * 1000);

        if (ready < 0) {
            if (errno == EINTR) {
                log_android(ANDROID_LOG_DEBUG,
                            __FUNCTION__,
                            "epoll interrupted tun %d thread %x", args->tun, thread_id_handle_events);
                continue;
            } else {
                log_android(ANDROID_LOG_ERROR,
                            __FUNCTION__,
                            "epoll tun %d thread %x error %d: %s",
                            args->tun, thread_id_handle_events, errno, strerror(errno));
                break;
            }
        }

        if (ready == 0)
            log_android(ANDROID_LOG_DEBUG, __FUNCTION__, "epoll timeout");
        else {

            if (pthread_mutex_lock(&lock))
                log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_lock failed");

            int error = 0;

            for (int i = 0; i < ready; i++) {
                if (ev[i].data.ptr == &ev_pipe) {
                    // Check pipe
                    stop_threads = 1;
                    uint8_t buffer[1];
                    if (read(pipefds[0], buffer, 1) < 0)
                        log_android(ANDROID_LOG_WARN, __FUNCTION__, "Read pipe error %d: %s",
                                    errno, strerror(errno));
                    else
                        log_android(ANDROID_LOG_WARN, __FUNCTION__, "Read pipe");
                    break;

                } else if (ev[i].data.ptr == NULL) {
                    // Check upstream
                    log_android(ANDROID_LOG_DEBUG, __FUNCTION__,"epoll ready %d/%d in %d out %d err %d hup %d",
                                i, ready,
                                (ev[i].events & EPOLLIN) != 0,
                                (ev[i].events & EPOLLOUT) != 0,
                                (ev[i].events & EPOLLERR) != 0,
                                (ev[i].events & EPOLLHUP) != 0);

                    while (!error && is_readable(args->tun))
                        if (check_tun(args, &ev[i], epoll_fd, sessions, maxsessions) < 0)
                            error = 1;

                } else {
                    // Check downstream
                    log_android(ANDROID_LOG_DEBUG,
                                __FUNCTION__,
                                "epoll ready %d/%d in %d out %d err %d hup %d prot %d sock %d",
                                i, ready,
                                (ev[i].events & EPOLLIN) != 0,
                                (ev[i].events & EPOLLOUT) != 0,
                                (ev[i].events & EPOLLERR) != 0,
                                (ev[i].events & EPOLLHUP) != 0,
                                ((struct vpn_session *) ev[i].data.ptr)->protocol,
                                ((struct vpn_session *) ev[i].data.ptr)->socket);

                    struct vpn_session *session = (struct vpn_session *) ev[i].data.ptr;
                    if (session->protocol == IPPROTO_ICMP ||
                        session->protocol == IPPROTO_ICMPV6)
                        check_icmp_socket(args, &ev[i]);
                    else if (session->protocol == IPPROTO_UDP) {
                        while (!(ev[i].events & EPOLLERR) && (ev[i].events & EPOLLIN) &&
                               is_readable(session->socket))
                            check_udp_socket(args, &ev[i]);
                    } else if (session->protocol == IPPROTO_TCP)
                        check_tcp_socket(args, &ev[i], epoll_fd);

                }

                if (error)
                    break;
            }

            if (pthread_mutex_unlock(&lock))
                log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_unlock failed");

            if (error)
                break;
        }
    }

    // Close epoll file
    if (epoll_fd >= 0 && close(epoll_fd))
        log_android(ANDROID_LOG_ERROR,
                    __FUNCTION__,
                    "epoll close error %d: %s", errno, strerror(errno));

    (*env)->DeleteGlobalRef(env, args->instance);

    // Detach from Java
    rs = (*jvm)->DetachCurrentThread(jvm);
    if (rs != JNI_OK)
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "DetachCurrentThread failed");

    // Cleanup
    free(args);

    log_android(ANDROID_LOG_WARN, __FUNCTION__, "Stopped events tun=%d thread %x", args->tun, thread_id_handle_events);
    thread_id_handle_events = 0;
    return NULL;
}

void check_allowed(const struct arguments *args) {
    char source[INET6_ADDRSTRLEN + 1];
    char dest[INET6_ADDRSTRLEN + 1];

    struct vpn_session *l = NULL;
    struct vpn_session *s = g_vpn_session;
    while (s != NULL) {
        if (s->protocol == IPPROTO_ICMP || s->protocol == IPPROTO_ICMPV6) {
            if (!s->icmp.stop) {
                if (s->icmp.version == 4) {
                    inet_ntop(AF_INET, &s->icmp.saddr.ip4, source, sizeof(source));
                    inet_ntop(AF_INET, &s->icmp.daddr.ip4, dest, sizeof(dest));
                } else {
                    inet_ntop(AF_INET6, &s->icmp.saddr.ip6, source, sizeof(source));
                    inet_ntop(AF_INET6, &s->icmp.daddr.ip6, dest, sizeof(dest));
                }

                s->icmp.stop = 1;
                log_android(ANDROID_LOG_WARN, __FUNCTION__, "ICMP terminate %d uid %d",
                            s->socket, s->icmp.uid);
            }

        } else if (s->protocol == IPPROTO_UDP) {
            if (s->udp.state == UDP_ACTIVE) {
                if (s->udp.version == 4) {
                    inet_ntop(AF_INET, &s->udp.saddr.ip4, source, sizeof(source));
                    inet_ntop(AF_INET, &s->udp.daddr.ip4, dest, sizeof(dest));
                } else {
                    inet_ntop(AF_INET6, &s->udp.saddr.ip6, source, sizeof(source));
                    inet_ntop(AF_INET6, &s->udp.daddr.ip6, dest, sizeof(dest));
                }

                s->udp.state = UDP_FINISHING;
                log_android(ANDROID_LOG_WARN, __FUNCTION__, "UDP terminate session socket %d uid %d",
                            s->socket, s->udp.uid);
            } else if (s->udp.state == UDP_BLOCKED) {
                log_android(ANDROID_LOG_WARN, __FUNCTION__,"UDP remove blocked session uid %d", s->udp.uid);

                if (l == NULL)
                    g_vpn_session = s->next;
                else
                    l->next = s->next;

                struct vpn_session *c = s;
                s = s->next;
                free(c);
                continue;
            }

        } else if (s->protocol == IPPROTO_TCP) {
            if (s->tcp.state != TCP_CLOSING && s->tcp.state != TCP_CLOSE) {
                if (s->tcp.version == 4) {
                    inet_ntop(AF_INET, &s->tcp.saddr.ip4, source, sizeof(source));
                    inet_ntop(AF_INET, &s->tcp.daddr.ip4, dest, sizeof(dest));
                } else {
                    inet_ntop(AF_INET6, &s->tcp.saddr.ip6, source, sizeof(source));
                    inet_ntop(AF_INET6, &s->tcp.daddr.ip6, dest, sizeof(dest));
                }

                write_rst(args, &s->tcp);
                log_android(ANDROID_LOG_WARN, __FUNCTION__,"TCP terminate socket %d uid %d",
                            s->socket, s->tcp.uid);
            }
        }

        l = s;
        s = s->next;
    }
}
