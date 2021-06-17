/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
package com.smansoft.vpn_stack;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.VpnService;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import java.io.IOException;

/**
 *
 */
public class VpnStackAppService extends VpnService {

    private static final String TAG = "VpnRstAppService.Service";

    public static final String ACTION_START = "com.example.vpn_rst.start";
    public static final String ACTION_STOP = "com.example.vpn_rst.stop";
    public static final String PREF_PROXY_HOST = "pref_proxy_host";
    public static final String PREF_PROXY_PORT = "pref_proxy_port";
    public static final String PREF_RUNNING = "pref_running";

    private ParcelFileDescriptor vpn;
    private boolean vpnStarted = false;

    static {
        try {
            System.loadLibrary("vpn_stack-lib");
        } catch (UnsatisfiedLinkError ignored) {
            System.exit(1);
        }
    }

    private PendingIntent configureIntent;

    private Handler handler;

    private static volatile PowerManager.WakeLock wlInstance = null;

    private SharedPreferences prefs;

    private native void jni_init();

    private native void jni_start(int tun, boolean fwd53, int rcode);

    private native void jni_stop(int tun);

    private native int jni_get_mtu();

    private native int jni_done();

    /**
     *
     * @param context
     * @return
     */
    synchronized private static PowerManager.WakeLock getLock(Context context) {
        if (wlInstance == null) {
            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            wlInstance = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, context.getString(R.string.app_name) + " wakelock");
            wlInstance.setReferenceCounted(true);
        }
        return wlInstance;
    }

    /**
     *
     * @param intent
     * @return
     */
    @Override
    public IBinder onBind(Intent intent) {
        return new VpnRstServiceBinder();
    }

    /**
     *
     */
    public class VpnRstServiceBinder extends Binder {
        @Override
        public boolean onTransact(int code, Parcel data, Parcel reply, int flags) throws RemoteException {
            if (code == IBinder.LAST_CALL_TRANSACTION) {
                onRevoke();
                return true;
            }
            return super.onTransact(code, data, reply, flags);
        }

        public VpnStackAppService getService() {
            return VpnStackAppService.this;
        }
    }

    /**
     *
     */
    private void start() {
        try {
            if (!vpnStarted) {
                vpn = startVPN(getBuilder());
                startNative(vpn);
                vpnStarted = true;
                sendNotificationMessage(R.string.started_vpn);
            }
            else {
                sendNotificationMessage(R.string.already_started_vpn);
            }
            Intent intent = new Intent("vpn_started");
            LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
        } catch (Throwable ex) {
            Log.e(TAG, ex.toString() + "\n" + Log.getStackTraceString(ex));
            sendNotificationMessage(R.string.error_starting_vpn);
            Intent intent = new Intent("vpn_stopped");
            LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
        }
    }

    /**
     *
     */
    private void stop() {
        try {
            if (vpnStarted) {
                stopNative(vpn);
                stopVPN(vpn);
                vpnStarted = false;
                sendNotificationMessage(R.string.stopped_vpn);
            }
            else {
                sendNotificationMessage(R.string.not_started_vpn);
            }
            Intent intent = new Intent("vpn_stopped");
            LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
            stopForeground(true);
        }
        catch (Throwable ex) {
            Log.e(TAG, ex.toString() + "\n" + Log.getStackTraceString(ex));
            sendNotificationMessage(R.string.error_starting_vpn);
            Intent intent = new Intent("vpn_stopped");
            LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
        }
    }

    /**
     *
     */
    @Override
    public void onRevoke() {
        Log.i(TAG, "onRevoke");
        stop();
        super.onRevoke();
    }

    /**
     *
     * @param builder
     * @return
     * @throws SecurityException
     */
    private ParcelFileDescriptor startVPN(Builder builder) throws SecurityException {
        Log.i(TAG, "startVPN");
        try {
            return builder.establish();
        } catch (Throwable ex) {
            Log.e(TAG, ex.toString() + "\n" + Log.getStackTraceString(ex));
            throw ex;
        }
    }

    /**
     *
     * @param pfd
     */
    private void stopVPN(ParcelFileDescriptor pfd) {
        Log.i(TAG, "stopVPN");
        try {
            pfd.close();
        } catch (IOException ex) {
            Log.e(TAG, ex.toString() + "\n" + Log.getStackTraceString(ex));
        }
    }

    /**
     *
     * @return
     */
    private Builder getBuilder() throws PackageManager.NameNotFoundException {

        Builder builder = new Builder();

        String vpn4 = "10.1.10.1";
        builder.addAddress(vpn4, 32);
        String vpn6 = "fd00:1:fd00:1:fd00:1:fd00:1";
        builder.addAddress(vpn6, 128);

        String route4 = "0.0.0.0";
        String route6 = "0:0:0:0:0:0:0:0";

        builder.addRoute(route4, 0);
        builder.addRoute(route6, 0);

        String dns4_1 = "8.8.8.8";
        String dns4_2 = "8.8.4.4";

        String dns6_1 = "2001:4860:4860::8888";
        String dns6_2 = "2001:4860:4860::8844";

        builder.addDnsServer(dns4_1);
        builder.addDnsServer(dns4_2);

        builder.addDnsServer(dns6_1);
        builder.addDnsServer(dns6_2);

        int mtu = jni_get_mtu();

        builder.setMtu(mtu);

        String appPackage = "";
        boolean catchAppValue = true;
        try {
            catchAppValue = prefs.getBoolean(VpnStackAppActivity.Prefs.CATCH_APP, true);
        }
        catch (Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }

        try {
            if(catchAppValue) {
                appPackage = getApplicationContext().getPackageName();
            }
            else {
                appPackage = prefs.getString(VpnStackAppActivity.Prefs.APP_PACKAGE, "");
            }
        }
        catch (Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }

        try {
            if(appPackage.length() > 0) {
                builder.addAllowedApplication(appPackage);
            }
        }
        catch (Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }

        builder.setSession(getString(R.string.app_name)).setConfigureIntent(this.configureIntent);

        Log.i(TAG, "vpn4 = " + vpn4);
        Log.i(TAG, "vpn6 = " + vpn6);

        Log.i(TAG, "route4 = " + route4);
        Log.i(TAG, "route6 = " + route6);

        Log.i(TAG, "dns4_1 = " + dns4_1);
        Log.i(TAG, "dns4_2 = " + dns4_2);

        Log.i(TAG, "dns6_1 = " + dns6_1);
        Log.i(TAG, "dns6_2 = " + dns6_2);

        Log.i(TAG, "MTU = " + mtu);

        Log.i(TAG, "appPackage = " + appPackage);

        return builder;
    }

    /**
     *
     * @param vpn
     */
    private void startNative(ParcelFileDescriptor vpn) {
        jni_start(vpn.getFd(), false, 3);
    }

    /**
     *
     * @param vpn
     */
    private void stopNative(ParcelFileDescriptor vpn) {
        try {
            jni_stop(vpn.getFd());
        } catch (Throwable ex) {
            // File descriptor might be closed
            Log.e(TAG, ex.toString() + "\n" + Log.getStackTraceString(ex));
            jni_stop(-1);
        }
    }

    /**
     *
     */
    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        // Native init
        jni_init();

        super.onCreate();

        prefs = getSharedPreferences(VpnStackAppActivity.Prefs.NAME, MODE_PRIVATE);
        configureIntent = PendingIntent.getActivity(this, 0, new Intent(this, VpnStackAppActivity.class),
                PendingIntent.FLAG_UPDATE_CURRENT);
        if (handler == null) {
            handler = new Handler(Looper.getMainLooper(),
                    new Handler.Callback() {
                        @Override
                        public boolean handleMessage(Message message) {
                            Toast.makeText(VpnStackAppService.this, message.what, Toast.LENGTH_SHORT).show();
                            updateForegroundNotification(message.what);
                            return true;
                        }
                    });
        }
        sendNotificationMessage(R.string.vpn_created);
    }

    /**
     *
     * @param intent
     * @param flags
     * @param startId
     * @return
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "Received " + intent);
        // Handle service restart
        if (intent == null) {
            return START_STICKY;
        }
        if (ACTION_START.equals(intent.getAction())) {
            sendNotificationMessage(R.string.starting_vpn);
            start();
        }
        if (ACTION_STOP.equals(intent.getAction())) {
            sendNotificationMessage(R.string.stopping_vpn);
            stop();
        }
        return START_STICKY;
    }

    /**
     *
     */
    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        try {
            if (vpnStarted) {
                stopNative(vpn);
                stopVPN(vpn);
                vpnStarted = false;
            }
        } catch (Throwable ex) {
            Log.e(TAG, ex.toString() + "\n" + Log.getStackTraceString(ex));
        }
        jni_done();
        super.onDestroy();
    }

    /**
     *
     * @param context
     */
    public static void start(Context context) {
        Intent intent = new Intent(context, VpnStackAppService.class);
        intent.setAction(ACTION_START);
        context.startService(intent);
    }

    /**
     *
     * @param context
     */
    public static void stop(Context context) {
        Intent intent = new Intent(context, VpnStackAppService.class);
        intent.setAction(ACTION_STOP);
        context.startService(intent);
    }

    /**
     *
     * @param messageTextId
     */
    private void sendNotificationMessage(final int messageTextId) {
        handler.sendEmptyMessage(messageTextId);
    }

    /**
     *
     * @param message
     */
    private void updateForegroundNotification(final int message) {
        final String NOTIFICATION_CHANNEL_ID = "vpn";
        NotificationManager notificationManager = (NotificationManager) getSystemService(
                NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(new NotificationChannel(
                NOTIFICATION_CHANNEL_ID, NOTIFICATION_CHANNEL_ID,
                NotificationManager.IMPORTANCE_DEFAULT));
        startForeground(1, new Notification.Builder(this, NOTIFICATION_CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_vpn)
                .setContentText(getString(message))
                .setContentIntent(this.configureIntent)
                .build());
    }

    /**
     *
     * @return
     */
    public boolean getVpnStarted() {
        return vpnStarted;
    }
}
