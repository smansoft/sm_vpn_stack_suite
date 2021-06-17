/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
package com.smansoft.vpn_stack;

/**
 *
 */
public class LogService {

    private static final String TAG = LogService.class.getSimpleName();

    public final int SM_LOG_OUT_FILE        = 1;
    public final int SM_LOG_OUT_CONSOLE     = 2;

    public final static int SM_LOG_LEVEL_OFF       = 0;
    public final static int SM_LOG_LEVEL_FATAL     = 10;
    public final static int SM_LOG_LEVEL_ERROR     = 20;
    public final static int SM_LOG_LEVEL_WARN      = 30;
    public final static int SM_LOG_LEVEL_INFO      = 40;
    public final static int SM_LOG_LEVEL_DEBUG     = 50;
    public final static int SM_LOG_LEVEL_TRACE     = 60;
    public final static int SM_LOG_LEVEL_ALL       = SM_LOG_LEVEL_TRACE;

    /**
     *
     */
    static {
        System.loadLibrary("vpn_stack-lib");
    }

    /**
     *
     * @param logDirPath
     * @param logFileName
     * @return
     */
    public native static int logInit(String logDirPath, String logFileName);

    /**
     *
     * @return
     */
    public native static int logClose();

    /**
     *
     * @param logLevel
     * @param logCategory
     * @param logText
     * @return
     */
    public native static int logPrint(int logLevel, String logCategory, String logText);
}
