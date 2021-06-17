/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */
package com.smansoft.socket_client;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Toast;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SocketChannel;

/**
 *
 */
public class SocketClientActivity extends AppCompatActivity {

    private static final String TAG = SocketClientActivity.class.getSimpleName();

    private static final int BUFF_SIZE = 1024;

    private static final String CLIENT_HELLO = "socket_client: Hello";

    static {
        //AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_UNSPECIFIED);
        //AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO);
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES);
    }

    public interface Prefs {
        String NAME = "socket_client";
        String SERVER_ADDRESS = "server.address";
        String SERVER_PORT = "server.port";
        String SERVER_PROTOCOL = "server.protocol";
    }

    /**
     *
     */
    public static enum CommunicationProtocol {

        UDP(0),
        TCP(1);

        private int value;

        CommunicationProtocol(int value) {
            this.value = value;
        }

        int getValue() {
            return this.value;
        }
    };

    private Handler handler;

    private EditText editTextSrvAddress;
    private EditText editTextSrvPort;

    private EditText editTextClnMessage;
    private EditText editTextSrvMessage1;
    private EditText editTextSrvMessage2;

    private Button buttonSendMessage;

    private RadioButton radioButtonTcp;
    private RadioButton radioButtonUdp;

    private CommunicationProtocol commProtocol;

    private SharedPreferences prefs;

    /**
     *
     */
    private class CommunicationRunnable implements Runnable {

        /**
         *
         */
        @Override
        public void run() {
            Log.i(TAG, "CommunicationRunnable started");

            Log.i(TAG, "Curr Server Address: "  + editTextSrvAddress.getText().toString());
            Log.i(TAG, "Curr Server Port: "     + editTextSrvPort.getText().toString());
            Log.i(TAG, "Curr Client Message: "  + editTextClnMessage.getText().toString());
            Log.i(TAG, "Curr Protocol: "        + commProtocol.toString());

            editTextSrvMessage1.setText("");
            editTextSrvMessage2.setText("");

            try {
                final String srvAddress = editTextSrvAddress.getText().toString();
                final int srvPort = Integer.parseInt(editTextSrvPort.getText().toString());
                final String clnMessage = editTextClnMessage.getText().toString();
                if(clnMessage.length() >= 3 && srvAddress.length() > 0 && srvPort > 0 && srvPort <= 65535) {
                    switch(commProtocol) {
                        case TCP:
                            communicationTcp(srvAddress, srvPort, clnMessage);
                            break;
                        case UDP:
                            communicationUdp(srvAddress, srvPort, clnMessage);
                            break;
                    }
                    prefs.edit()
                            .putString(Prefs.SERVER_ADDRESS, srvAddress)
                            .putInt(Prefs.SERVER_PORT, srvPort)
                            .putInt(Prefs.SERVER_PROTOCOL, commProtocol.getValue()).commit();
                }
                else {
                    String forLog = "wrong parameters...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                }
            }
            catch (NumberFormatException e) {
                Log.e(TAG, "NumberFormatException: " + e.toString() + "\n" + Log.getStackTraceString(e));
            }
            catch (UnknownHostException e) {
                Log.e(TAG, "UnknownHostException: " + e.toString() + "\n" + Log.getStackTraceString(e));
            }
            catch (IOException e) {
                Log.e(TAG, "IOException: " + e.toString() + "\n" + Log.getStackTraceString(e));
            }
            catch (Exception e) {
                Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
            }
            Log.i(TAG, "CommunicationRunnable finished");
        }

        /**
         *
         * @param srvAddress
         * @param srvPort
         * @param clnMessage
         * @throws IOException
         */
        private void communicationTcp(String srvAddress, int srvPort, String clnMessage) throws IOException {
            SocketChannel connection = null;
            try {
                final InetAddress inetAddress = InetAddress.getByName(srvAddress);

                ByteBuffer packet = ByteBuffer.allocate(BUFF_SIZE);

                Log.i(TAG, "inetAddress.getHostName()       = "  + inetAddress.getHostName());
                Log.i(TAG, "inetAddress.getHostAddress()    = "  + inetAddress.getHostAddress());

                final SocketAddress serverAddress = new InetSocketAddress(inetAddress, srvPort);

                connection = SocketChannel.open();

                connection.connect(serverAddress);

                String forLog;
                String message = new String(CLIENT_HELLO);
                String answer = null;

                forLog = "sending message: " + message;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                packet.put(message.getBytes()).flip();
                packet.position(0);
                int resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                forLog = "sending message (1): " + clnMessage;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                packet.put(clnMessage.getBytes()).flip();
                resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                Log.i(TAG, "resConnectionforLog = " + resConnection);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                    editTextSrvMessage1.append(answer);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                }

                forLog = "sending message (2): " + answer;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                packet.put(answer.getBytes()).flip();
                resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                Log.i(TAG, "resConnectionforLog = " + resConnection);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                    editTextSrvMessage2.setText(answer);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                }
            }
            finally {
                if(connection != null) {
                    try {
                        connection.close();
                    }
                    catch(IOException e) {
                        Log.e(TAG, "IOException: " + e.toString() + "\n" + Log.getStackTraceString(e));
                    }
                }
            }
        }

        /**
         *
         * @param srvAddress
         * @param srvPort
         * @param clnMessage
         * @throws IOException
         */
        private void communicationUdp(String srvAddress, int srvPort, String clnMessage) throws IOException {
            DatagramChannel connection = null;
            try {
                final InetAddress inetAddress = InetAddress.getByName(srvAddress);

                ByteBuffer packet = ByteBuffer.allocate(BUFF_SIZE);

                Log.i(TAG, "inetAddress.getHostName()       = "  + inetAddress.getHostName());
                Log.i(TAG, "inetAddress.getHostAddress()    = "  + inetAddress.getHostAddress());

                SocketAddress serverAddress = new InetSocketAddress(inetAddress, srvPort);

                connection = DatagramChannel.open();

                connection.connect(serverAddress);

                String forLog;
                String message = new String(CLIENT_HELLO);
                String answer = null;

                forLog = "sending message: " + message;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                byte[] bytes = message.getBytes();
                packet.put(message.getBytes()).flip();
                packet.position(0);
                int resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                    srvPort = Integer.parseInt(answer);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                connection.close();
                connection = null;

                serverAddress = new InetSocketAddress(inetAddress, srvPort);

                connection = DatagramChannel.open();
                connection.connect(serverAddress);

                message = new String(CLIENT_HELLO);

                forLog = "sending message: " + message;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                bytes = message.getBytes();
                packet.put(message.getBytes()).flip();
                packet.position(0);
                resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                forLog = "sending message (1): " + clnMessage;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                packet.put(clnMessage.getBytes()).flip();
                resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                    editTextSrvMessage1.setText(answer);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                }

                forLog = "sending message (2): " + answer;

                Log.i(TAG, forLog);
                sendToastMessage(forLog);

                packet.clear();
                packet.put(answer.getBytes()).flip();
                resConnection = connection.write(packet);
                if(resConnection > 0) {
                    forLog = "sent: " + new String(packet.array(),0, resConnection);
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                }
                else {
                    forLog = "sending data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                    throw new IOException(forLog);
                }

                packet.clear();
                resConnection = connection.read(packet);
                if(resConnection > 0) {
                    answer = new String(packet.array(), 0, resConnection);
                    forLog = "received: " + answer;
                    Log.i(TAG, forLog);
                    sendToastMessage(forLog);
                    editTextSrvMessage2.setText(answer);
                }
                else {
                    forLog = "receiving data - error...";
                    Log.e(TAG, forLog);
                    sendToastMessage(forLog);
                }

            }
            finally {
                if(connection != null) {
                    try {
                        connection.close();
                    }
                    catch(IOException e) {
                        Log.e(TAG, "IOException: " + e.toString() + "\n" + Log.getStackTraceString(e));
                    }
                }
            }
        }
    }

    /**
     *
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.socket_client_activity);
        getDelegate().applyDayNight();

        if (this.handler == null) {
            this.handler = new Handler(Looper.getMainLooper(),
                    new Handler.Callback() {
                        @Override
                        public boolean handleMessage(Message message) {
                            Toast.makeText(SocketClientActivity.this, new String(message.getData().getByteArray("message")), Toast.LENGTH_SHORT).show();
                            return true;
                        }
                    });
        }

        prefs = getSharedPreferences(Prefs.NAME, MODE_PRIVATE);

        radioButtonTcp = findViewById(R.id.radioButtonTcp);
        radioButtonUdp = findViewById(R.id.radioButtonUdp);

        editTextSrvAddress = findViewById(R.id.editTextSrvAddress);
        editTextSrvPort = findViewById(R.id.editTextSrvPort);

        editTextClnMessage = findViewById(R.id.editTextClnMessage);
        editTextSrvMessage1 = findViewById(R.id.editTextSrvMessage1);
        editTextSrvMessage2 = findViewById(R.id.editTextSrvMessage2);

        buttonSendMessage = findViewById(R.id.buttonSendMessage);

        buttonSendMessage.setOnClickListener(v -> {
            Log.i(TAG, "starting send message");
            try {
                final CommunicationRunnable communicationRunnable = new CommunicationRunnable();
                final Thread communicationThread = new Thread(communicationRunnable, "CommunicationRunnable");

                communicationThread.start();
            }
            catch (IllegalThreadStateException e) {
                Log.e(TAG, "IllegalThreadStateException: " + e.toString() + "\n" + Log.getStackTraceString(e));
            }
            catch (Exception e) {
                Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
            }
            Log.i(TAG, "thread for sending message has been launched...");
        } );

        radioButtonTcp.setOnClickListener( v -> {
            setCurrentCommunicationProtocol(CommunicationProtocol.TCP);
        });

        radioButtonUdp.setOnClickListener( v -> {
            setCurrentCommunicationProtocol(CommunicationProtocol.UDP);
        } );

        try {
            editTextSrvAddress.setText(prefs.getString(Prefs.SERVER_ADDRESS, ""));
        }
        catch(Exception e) {
            Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
        }

        try {
            int serverPortPrefValue = prefs.getInt(Prefs.SERVER_PORT, 0);
            editTextSrvPort.setText(String.valueOf(serverPortPrefValue == 0 ? "" : serverPortPrefValue));
        }
        catch(Exception e) {
            Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
        }

        try {
            int serverPortPrefValue = prefs.getInt(Prefs.SERVER_PORT, 0);
            editTextSrvPort.setText(String.valueOf(serverPortPrefValue == 0 ? "" : serverPortPrefValue));
        }
        catch(Exception e) {
            Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
        }

        try {
            int serverProtocolPrefValue = prefs.getInt(Prefs.SERVER_PROTOCOL, 1);
            if(CommunicationProtocol.UDP.getValue() == serverProtocolPrefValue) {
                setCurrentCommunicationProtocol(CommunicationProtocol.UDP);
            }
            else if(CommunicationProtocol.TCP.getValue() == serverProtocolPrefValue) {
                setCurrentCommunicationProtocol(CommunicationProtocol.TCP);
            }
            else {
                setCurrentCommunicationProtocol(CommunicationProtocol.TCP);
            }
        }
        catch(Exception e) {
            Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
        }

    }

    /**
     *
     * @param commProtocol
     */
    private void setCurrentCommunicationProtocol(CommunicationProtocol commProtocol) {
        this.commProtocol = commProtocol;
        switch(commProtocol) {
            case TCP:
                radioButtonTcp.setChecked(true);
                break;
            case UDP:
                radioButtonUdp.setChecked(true);
                break;
        }
    }

    /**
     *
     * @param messageText
     */
    private void sendToastMessage(String messageText) {
        Message message = new Message();
        Bundle msgBundle = new Bundle();
        msgBundle.putByteArray("message", messageText.getBytes());
        message.setData(msgBundle);
        SocketClientActivity.this.handler.sendMessage(message);
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

    @Override
    public void onStop() {
        super.onStop();
        int serverPortNum;
        try {
            serverPortNum = Integer.parseInt(editTextSrvPort.getText().toString());
        } catch (NumberFormatException e) {
            serverPortNum = 0;
        }
        try {
            prefs.edit()
                    .putString(Prefs.SERVER_ADDRESS, editTextSrvAddress.getText().toString())
                    .putInt(Prefs.SERVER_PORT, serverPortNum)
                    .putInt(Prefs.SERVER_PROTOCOL, commProtocol.getValue()).commit();
        }
        catch(Exception e) {
            Log.e(TAG, "Exception: " + e.toString() + "\n" + Log.getStackTraceString(e));
        }
    }

    /**
     *
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

}
