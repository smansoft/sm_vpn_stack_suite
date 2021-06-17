/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
package com.smansoft.vpn_stack;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.IBinder;
import android.text.InputType;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.google.android.material.switchmaterial.SwitchMaterial;

/**
 *
 */
public class VpnStackAppActivity extends AppCompatActivity {

    private static final String TAG = VpnStackAppActivity.class.getSimpleName();

    public interface Prefs {
        String NAME = "vpn_stack";
        String CATCH_APP = "catch.app";
        String APP_PACKAGE = "app.package";
    }

    static {
        //AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_UNSPECIFIED);
        //AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO);
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES);
    }

    private SwitchMaterial catchApp;
    private EditText editTextPackage;

    private TextView status;

    private Button buttonStartVpn;
    private Button buttonStopVpn;

    private SharedPreferences prefs;

    private ActivityResultLauncher<Intent> activityResultLauncher;

    private int editTextInputType;

    /**
     *
     */
    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
        if ("vpn_started".equals(intent.getAction())) {
            final TextView status = findViewById(R.id.textViewStatus);
            status.setText(getResources().getString(R.string.status_started));
        }
        else if ("vpn_stopped".equals(intent.getAction())) {
            final TextView status = findViewById(R.id.textViewStatus);
            status.setText(getResources().getString(R.string.status_nstarted));
        }
        }
    };

    /**
     *
     * @param savedInstanceState
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vpn_stack_app);
        getDelegate().applyDayNight();

        catchApp = findViewById(R.id.switchCatchApp);
        status = findViewById(R.id.textViewStatus);

        buttonStartVpn = findViewById(R.id.buttonStartVpn);
        buttonStopVpn = findViewById(R.id.buttonStopVpn);

        editTextPackage = findViewById(R.id.editTextPackage);
        editTextInputType = editTextPackage.getInputType();

        prefs = getSharedPreferences(Prefs.NAME, MODE_PRIVATE);

        LocalBroadcastManager.getInstance(this).registerReceiver(broadcastReceiver, new IntentFilter("vpn_started"));
        LocalBroadcastManager.getInstance(this).registerReceiver(broadcastReceiver, new IntentFilter("vpn_stopped"));

        Intent intent = new Intent("vpn_is_started");
        LocalBroadcastManager.getInstance(this).sendBroadcastSync (intent);

        boolean vpnStarted = intent.getBooleanExtra("vpn_is_started", false);
        if(vpnStarted) {
            status.setText(getResources().getString(R.string.status_started));
        }
        else {
            status.setText(getResources().getString(R.string.status_nstarted));
        }

        activityResultLauncher = registerForActivityResult(
            new ActivityResultContracts.StartActivityForResult(),
            result -> {
                if (result.getResultCode() == Activity.RESULT_OK) {
                    startService(getServiceIntent().setAction(VpnStackAppService.ACTION_START));
/*
                    -- just for direct binding
                    Intent serviceIntent = new Intent(this, VpnRstAppService.class);
                    bindService(serviceIntent, vpnServiceConnection, 0);
 */
                }
            }
        );

        buttonStartVpn.setOnClickListener(v -> {
            prefs.edit()
                    .putBoolean(Prefs.CATCH_APP, catchApp.isChecked())
                    .putString(Prefs.APP_PACKAGE, editTextPackage.getText().toString())
                    .commit();
            Intent vpnIntent = VpnStackAppService.prepare(VpnStackAppActivity.this);
            if (vpnIntent != null) {
                activityResultLauncher.launch(vpnIntent);
            } else {
                startService(getServiceIntent().setAction(VpnStackAppService.ACTION_START));
/*
                -- just for direct binding
                Intent serviceIntent = new Intent(this, VpnRstAppService.class);
                bindService(serviceIntent, vpnServiceConnection, 0);
 */
            }
        });

        buttonStopVpn.setOnClickListener(v -> {
            startService(getServiceIntent().setAction(VpnStackAppService.ACTION_STOP));
        });

        catchApp.setOnCheckedChangeListener( (buttonView, isChecked) -> {
            if(isChecked) {
                editTextPackage.setText("");
                editTextPackage.setEnabled(false);
                editTextPackage.setInputType(InputType.TYPE_NULL);
            }
            else {
                editTextPackage.setEnabled(true);
                editTextPackage.setInputType(editTextInputType);
            }
        });

        try {
            catchApp.setChecked(prefs.getBoolean(Prefs.CATCH_APP, true));
        }
        catch (Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }

        try {
            editTextPackage.setText(prefs.getString(Prefs.APP_PACKAGE, ""));
        }
        catch (Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onRestart() {
        super.onRestart();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    /**
     *
     */
    @Override
    public void onStop() {
        super.onStop();
        try {
            prefs.edit()
                    .putBoolean(Prefs.CATCH_APP, catchApp.isChecked())
                    .putString(Prefs.APP_PACKAGE, editTextPackage.getText().toString())
                    .commit();
        }
        catch(Exception e) {
            Log.e(TAG, e.toString() + "\n" + Log.getStackTraceString(e));
        }
    }

    /**
     *
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    /**
     *
     * @return
     */
    private Intent getServiceIntent() {
        return new Intent(this, VpnStackAppService.class);
    }

    /*  for direct binding */
    private ServiceConnection vpnServiceConnection = new ServiceConnection() {

        /**
         *
         * @param className
         * @param service
         */
        @Override
        public void onServiceConnected(ComponentName className,
                                       IBinder service) {

            VpnStackAppService.VpnRstServiceBinder binder = (VpnStackAppService.VpnRstServiceBinder)service;
            VpnStackAppService vpnService = binder.getService();

            boolean vpnStarted = vpnService.getVpnStarted();

            Log.i(TAG,"vpnStarted = " + vpnStarted);
        }

        /**
         *
         * @param arg0
         */
        @Override
        public void onServiceDisconnected(ComponentName arg0) {
        }
    };

}
