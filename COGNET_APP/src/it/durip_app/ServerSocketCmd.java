/*
Cognitive Network APP 
Copyright (C) 2014  Matteo Danieletto matteo.danieletto@dei.unipd.it
University of Padova, Italy +34 049 827 7778
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
package it.durip_app;

//import java.io.BufferedReader;
//import java.io.DataOutputStream;
//import java.io.IOException;
//import java.io.InputStreamReader;
//import java.io.UnsupportedEncodingException;
//import java.net.InetSocketAddress;
//import java.net.ServerSocket;
//import java.net.Socket;
//import java.net.SocketAddress;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.IBinder;
import android.util.Log;

public class ServerSocketCmd extends Service implements Runnable{
	
	private class JniCall extends AsyncTask<String, Void, String> {

	    @Override
	    protected String doInBackground(String... params) {
	    	
	        mainJNI(4,params);
	        return "Executed";
	    }      

	    /* (non-Javadoc)
	     * @see android.os.AsyncTask#onPostExecute(java.lang.Object)
	     */
	    @Override
	    protected void onPostExecute(String result) {               
//	        TextView txt = (TextView) findViewById(R.id.output);
//	        txt.setText(result);
	    }
	}
	static {
		System.loadLibrary("nl-3"); // "libmyjni.so" in Unixes
		System.loadLibrary("nl-genl-3"); // "libmyjni.so" in Unixes
		System.loadLibrary("MACReadServer"); // "libmyjni.so" in Unixes
		
    }
	public native int mainJNI(int n , String []s);
	//public native void manageThreadsAPP(int n , String []s);
	public static final String LOOP="LOOP";
	public static final String DESTINATION="DESTINATION";
	public static final String PORT="PORT";
	public static final String DEVICE="DEVICE";
	public static final String PHY="PHY";
	public static final String PRINT="PRINT";
	public static final String INTERVAL="INTERVAL";
	public static final String ESSID="ESSID";
	public static final String PATH="PATH";
	public static final String SUBNET="SUBNET";
	public static final String NETMASK="NETMASK";
	public static final int params=10;
	private boolean isPlaying=false;
	public static String s[];
	private JniCall JniHook;
	  
	public ServerSocketCmd() {
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.d("MAC SERVERSOCKET", "Launched");
		s = new String[params];
//	    boolean loop=intent.getBooleanExtra(LOOP, false);
	    JniHook = new JniCall();
	    s[0] = intent.getStringExtra(DESTINATION);
	    s[1] = intent.getStringExtra(PORT);
	    s[2] = intent.getStringExtra(DEVICE);
	    s[3] = intent.getStringExtra(PHY);
	    s[4] = intent.getStringExtra(INTERVAL);
	    s[5] = intent.getStringExtra(PRINT);
	    s[6] = intent.getStringExtra(ESSID);
	    s[7] = intent.getStringExtra(PATH);
	    s[8] = intent.getStringExtra(SUBNET);
	    s[9] = intent.getStringExtra(NETMASK);

	    this.run();
	    return(START_NOT_STICKY);
	}
	  
	@Override
	public void onDestroy() {
		stop();
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO: Return the communication channel to the service.
	    return(null);
		//throw new UnsupportedOperationException("Not yet implemented");
	}

	@Override
	public void run() {
		// TODO Auto-generated method stub
	    if (!isPlaying) {
			JniHook.execute(s);
	    	isPlaying=true;
	    }
	}
    private void stop() {
	    if (isPlaying) {
			//JniHook.cancel(true);
	    	isPlaying=false;
	    }
	}
}
