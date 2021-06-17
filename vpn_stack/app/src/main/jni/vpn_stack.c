/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#include <jni.h>

#include "sm_files_tools.h"
#include "sm_log.h"

#include "vpn.h"

extern sm_log_config gsm_log_config;           //  global instance of main log support structure
#define SM_LOG_CONFIG &gsm_log_config   //  just synonym: SM_LOG_CONFIG == &gsm_log_config - for usage in log api calls

extern JavaVM *jvm;
extern int pipefds[2];
extern pthread_t thread_id_handle_events;
extern pthread_mutex_t lock;
extern int stop_threads;
extern int loglevel;

extern int max_tun_msg;
extern struct vpn_session *ng_session;

// JNI

/**
 *
 * @param vm
 * @param reserved
 * @return
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    log_android(ANDROID_LOG_INFO, __FUNCTION__, "JNI load");

    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        log_android(ANDROID_LOG_INFO, __FUNCTION__, "JNI load GetEnv failed");
        return -1;
    }

    // Raise file number limit to maximum
    struct rlimit rlim;
    if (getrlimit(RLIMIT_NOFILE, &rlim))
        log_android(ANDROID_LOG_WARN, __FUNCTION__, "getrlimit error %d: %s", errno, strerror(errno));
    else {
        rlim_t soft = rlim.rlim_cur;
        rlim.rlim_cur = rlim.rlim_max;
        if (setrlimit(RLIMIT_NOFILE, &rlim))
            log_android(ANDROID_LOG_WARN, __FUNCTION__, "setrlimit error %d: %s", errno, strerror(errno));
        else
            log_android(ANDROID_LOG_WARN, __FUNCTION__, "raised file limit from %d to %d", soft, rlim.rlim_cur);
    }
    return JNI_VERSION_1_6;
}

/**
 *
 * @param vm
 * @param reserved
 */
void JNI_OnUnload(JavaVM *vm, void *reserved) {
    log_android(ANDROID_LOG_INFO, __FUNCTION__, "JNI unload");
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK)
        log_android(ANDROID_LOG_INFO, __FUNCTION__, "JNI load GetEnv failed");
}

/**
 *
 * @param env
 * @param thiz
 */
JNIEXPORT void JNICALL
Java_com_smansoft_vpn_1stack_VpnStackAppService_jni_1init(JNIEnv *env, jobject thiz) {
//    loglevel = ANDROID_LOG_WARN;
    loglevel = ANDROID_LOG_VERBOSE;

    log_android(ANDROID_LOG_INFO, __FUNCTION__, "jni_1init");

    struct arguments args;
    args.env = env;
    args.instance = thiz;
    init(&args);

    if (pthread_mutex_init(&lock, NULL))
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_init failed");

    // Create signal pipe
    if (pipe(pipefds))
        log_android(ANDROID_LOG_ERROR, __FUNCTION__,"Create pipe error %d: %s", errno, strerror(errno));
    else {
        for (int i = 0; i < 2; i++) {
            int flags = fcntl(pipefds[i], F_GETFL, 0);
            if (flags < 0 || fcntl(pipefds[i], F_SETFL, flags | O_NONBLOCK) < 0)
                log_android(ANDROID_LOG_ERROR, __FUNCTION__,
                            "fcntl pipefds[%d] O_NONBLOCK error %d: %s",
                            i, errno, strerror(errno));
        }
    }
}

/**
 *
 * @param env
 * @param thiz
 * @param tun
 * @param fwd53
 * @param rcode
 * @param tcp_rst_timeout
 */
JNIEXPORT void JNICALL
Java_com_smansoft_vpn_1stack_VpnStackAppService_jni_1start(JNIEnv *env, jobject thiz, jint tun,
                                                           jboolean fwd53, jint rcode) {
    log_android(ANDROID_LOG_INFO, __FUNCTION__, "jni_1start");

    max_tun_msg = 0;

    log_android(ANDROID_LOG_INFO, __FUNCTION__, "start_1proxy (info)");
    log_android(ANDROID_LOG_VERBOSE, __FUNCTION__, "start_1proxy (verbose)");

    // Set blocking
    int flags = fcntl(tun, F_GETFL, 0);
    if (flags < 0 || fcntl(tun, F_SETFL, flags & ~O_NONBLOCK) < 0)
        log_android(ANDROID_LOG_ERROR, __FUNCTION__,"fcntl tun ~O_NONBLOCK error %d: %s",
                    errno, strerror(errno));

    if (thread_id_handle_events && pthread_kill(thread_id_handle_events, 0) == 0)
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "Already running thread %x", thread_id_handle_events);
    else {
        jint rs = (*env)->GetJavaVM(env, &jvm);
        if (rs != JNI_OK)
            log_android(ANDROID_LOG_ERROR, __FUNCTION__, "GetJavaVM failed");

        // Get arguments
        struct arguments *args = malloc(sizeof(struct arguments));
        // args->env = will be set in thread
        args->instance = (*env)->NewGlobalRef(env, thiz);
        args->tun = tun;
        args->fwd53 = fwd53;
        args->rcode = rcode;

        stop_threads = 0;

        // Start native thread
        int err = pthread_create(&thread_id_handle_events, NULL, handle_events, (void *) args);
        if (err == 0)
            log_android(ANDROID_LOG_WARN, __FUNCTION__, "Started thread %x", thread_id_handle_events);
        else
            log_android(ANDROID_LOG_ERROR, __FUNCTION__,"pthread_create error %d: %s", err, strerror(err));
    }
}

/**
 *
 * @param env
 * @param thiz
 * @param tun
 */
JNIEXPORT void JNICALL
Java_com_smansoft_vpn_1stack_VpnStackAppService_jni_1stop(JNIEnv *env, jobject thiz, jint tun) {
    pthread_t t = thread_id_handle_events;
    log_android(ANDROID_LOG_WARN, __FUNCTION__,"Stop tun %d thread %x", tun, t);
    if (t && pthread_kill(t, 0) == 0) {
        log_android(ANDROID_LOG_WARN, __FUNCTION__,"Write pipe thread %x", t);
        if (write(pipefds[1], "x", 1) < 0)
            log_android(ANDROID_LOG_WARN, __FUNCTION__,"Write pipe error %d: %s", errno, strerror(errno));
        else {
            log_android(ANDROID_LOG_WARN, __FUNCTION__,"Join thread %x", t);
            int err = pthread_join(t, NULL);
            if (err != 0)
                log_android(ANDROID_LOG_WARN, __FUNCTION__,"pthread_join error %d: %s", err, strerror(err));
        }

        clear();

        thread_id_handle_events = 0;

        log_android(ANDROID_LOG_WARN, __FUNCTION__,"Stopped thread %x", t);
    } else
        log_android(ANDROID_LOG_WARN, __FUNCTION__,"Not running thread %x", t);
}

/**
 *
 * @param env
 * @param thiz
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_smansoft_vpn_1stack_VpnStackAppService_jni_1get_1mtu(JNIEnv *env, jobject thiz) {
    return get_mtu();
}

/**
 *
 * @param env
 * @param thiz
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_smansoft_vpn_1stack_VpnStackAppService_jni_1done(JNIEnv *env, jobject thiz) {
    log_android(ANDROID_LOG_INFO, __FUNCTION__, "Done");
    clear();
    if (pthread_mutex_destroy(&lock))
        log_android(ANDROID_LOG_ERROR, __FUNCTION__, "pthread_mutex_destroy failed");
    for (int i = 0; i < 2; i++) {
        if (close(pipefds[i]))
            log_android(ANDROID_LOG_ERROR, __FUNCTION__, "Close pipe error %d: %s", errno, strerror(errno));
    }
}

