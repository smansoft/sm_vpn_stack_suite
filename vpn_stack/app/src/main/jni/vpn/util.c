
#include "vpn.h"

extern sm_log_config gsm_log_config;    //  global instance of main log support structure
#define SM_LOG_CONFIG &gsm_log_config   //  just synonym: SM_LOG_CONFIG == &gsm_log_config - for usage in log api calls

extern int loglevel;

uint16_t calc_checksum(uint16_t start, const uint8_t *buffer, size_t length) {
    register uint32_t sum = start;
    register uint16_t *buf = (uint16_t *) buffer;
    register size_t len = length;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len > 0)
        sum += *((uint8_t *) buf);

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t) sum;
}

int compare_u32(uint32_t s1, uint32_t s2) {
    // https://tools.ietf.org/html/rfc1982
    if (s1 == s2)
        return 0;

    int i1 = s1;
    int i2 = s2;
    if ((i1 < i2 && i2 - i1 < 0x7FFFFFFF) ||
        (i1 > i2 && i1 - i2 > 0x7FFFFFFF))
        return -1;
    else
        return 1;
}

void log_android(int prio, const char* const log_category, const char *fmt, ...) {
    char line[64*1024];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(line, fmt, argptr);
    if (prio >= loglevel)
        __android_log_print(prio, TAG, "%s", line);
    sm_log_print(SM_LOG_CONFIG, convert_log_prio(prio), log_category, line);
    va_end(argptr);
}

uint8_t char2nible(const char c) {
    if (c >= '0' && c <= '9') return (uint8_t) (c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t) ((c - 'a') + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t) ((c - 'A') + 10);
    return 255;
}

void hex2bytes(const char *hex, uint8_t *buffer) {
    size_t len = strlen(hex);
    for (int i = 0; i < len; i += 2)
        buffer[i / 2] = (char2nible(hex[i]) << 4) | char2nible(hex[i + 1]);
}

char *trim(char *str) {
    while (isspace(*str))
        str++;
    if (*str == 0)
        return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;
    *(end + 1) = 0;
    return str;
}

const char *strstate(const int state) {
    switch (state) {
        case TCP_ESTABLISHED:
            return "ESTABLISHED";
        case TCP_SYN_SENT:
            return "SYN_SENT";
        case TCP_SYN_RECV:
            return "SYN_RECV";
        case TCP_FIN_WAIT1:
            return "FIN_WAIT1";
        case TCP_FIN_WAIT2:
            return "FIN_WAIT2";
        case TCP_TIME_WAIT:
            return "TIME_WAIT";
        case TCP_CLOSE:
            return "CLOSE";
        case TCP_CLOSE_WAIT:
            return "CLOSE_WAIT";
        case TCP_LAST_ACK:
            return "LAST_ACK";
        case TCP_LISTEN:
            return "LISTEN";
        case TCP_CLOSING:
            return "CLOSING";
        default:
            return "UNKNOWN";
    }
}

char *to_str(const u_int8_t *data, const size_t len) {
    char *strout;
    strout = (char *) malloc(len + 2); // TODO free

    strncpy(strout, data, len);
    strout[len] = '\0';

    return strout;
}

char *to_hex(const u_int8_t *data, const size_t len) {
    char hex_str[] = "0123456789ABCDEF";

    char *hexout;
    hexout = (char *) malloc(len * 3 + 1); // TODO free

    for (size_t i = 0; i < len; i++) {
        hexout[i * 3 + 0] = hex_str[(data[i] >> 4) & 0x0F];
        hexout[i * 3 + 1] = hex_str[(data[i]) & 0x0F];
        hexout[i * 3 + 2] = ' ';
    }
    hexout[len * 3] = 0;

    return hexout;
}

int32_t get_local_port(const int sock) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sock, (struct sockaddr *) &sin, &len) < 0) {
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "getsockname error %d: %s", errno, strerror(errno));
        return -1;
    }
    else
        return ntohs(sin.sin_port);
}

int is_event(int fd, short event) {
    struct pollfd p;
    p.fd = fd;
    p.events = event;
    p.revents = 0;
    int r = poll(&p, 1, 0);
    if (r < 0) {
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "poll readable error %d: %s", errno, strerror(errno));
        return 0;
    }
    else if (r == 0)
        return 0;
    else
        return (p.revents & event);
}

int is_readable(int fd) {
    return is_event(fd, POLLIN);
}

int is_writable(int fd) {
    return is_event(fd, POLLOUT);
}

long long get_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1e6;
}

int convert_log_prio(int android_log_prio)
{
    int res_log_prio = SM_LOG_LEVEL_OFF;
    switch(android_log_prio) {
        case ANDROID_LOG_UNKNOWN:
            res_log_prio = SM_LOG_LEVEL_TRACE;
            break;
        case ANDROID_LOG_DEFAULT:
            res_log_prio = SM_LOG_LEVEL_INFO;
            break;
        case ANDROID_LOG_VERBOSE:
            res_log_prio = SM_LOG_LEVEL_TRACE;
            break;
        case ANDROID_LOG_DEBUG:
            res_log_prio = SM_LOG_LEVEL_DEBUG;
            break;
        case ANDROID_LOG_INFO:
            res_log_prio = SM_LOG_LEVEL_INFO;
            break;
        case ANDROID_LOG_WARN:
            res_log_prio = SM_LOG_LEVEL_WARN;
            break;
        case ANDROID_LOG_ERROR:
            res_log_prio = SM_LOG_LEVEL_ERROR;
            break;
        case ANDROID_LOG_FATAL:
            res_log_prio = SM_LOG_LEVEL_FATAL;
            break;
        case ANDROID_LOG_SILENT:
            res_log_prio = SM_LOG_LEVEL_OFF;
            break;
        default:
            res_log_prio = SM_LOG_LEVEL_TRACE;
            break;
    }
    return res_log_prio;
}
