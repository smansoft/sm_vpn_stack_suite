/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
package com.smansoft.socket_client;

import android.app.Application;
import android.util.Log;

public class SocketClientApp extends Application {

    /**
     *
     */
    private static final String TAG = SocketClientApp.class.getSimpleName();

    /**
     *
     */
    public SocketClientApp() {
    }

    /**
     *
     */
    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate() ########################################################################################################### >>");
    }

    /**
     *
     */
    @Override
    public void onTerminate () {
        super.onTerminate();
    }

}
