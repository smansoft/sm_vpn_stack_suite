/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
package com.smansoft.vpn_stack;

import android.app.Application;
import android.util.Log;

import java.io.File;

/**
 *
 */
public class VpnStackApp extends Application {

    static {
        try {
            System.loadLibrary("vpn_stack-lib");
        } catch (UnsatisfiedLinkError ignored) {
            System.exit(1);
        }
    }

    /**
     *
     */
    private static final String TAG = VpnStackApp.class.getSimpleName();

    /**
     *
     */
    private static final String LOG_FNAME = "vpn_stack.log";

    /**
     *
     */
    public VpnStackApp() {
    }

    /**
     *
     */
    @Override
    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "onCreate() ########################################################################################################### >>");

        try {

            final File path = new File(getFilesDir(), "/logs");
            if (!path.exists()) {
                path.mkdir();
            }
            LogService.logInit(path.getAbsolutePath(), LOG_FNAME);

        }
        catch (Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }

        LogService.logPrint(LogService.SM_LOG_LEVEL_INFO, TAG,"onCreate() ########################################################################################################### >>");
    }

    /**
     *
     */
    @Override
    public void onTerminate () {
        super.onTerminate();
    }

}
