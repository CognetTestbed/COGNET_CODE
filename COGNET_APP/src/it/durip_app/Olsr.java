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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.app.IntentService;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class Olsr extends IntentService {
	
	public Olsr() {
		super("Olsrd");
		// TODO Auto-generated constructor stub
	}
	static {
		System.loadLibrary("olsrdLib"); // "libmyjni.so" in Unixes
    }
	public native int olsrdOff(int n);
	public native int olsrdOn(int n);


  public static final String LOOP="LOOP";
  private boolean isPlaying=false;
  private static Process p=null,p1=null;
  private static BufferedReader lines=null;
  private static Runtime r = Runtime.getRuntime();
  private String inputLine="";
  
  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    boolean loop=intent.getBooleanExtra(LOOP, false);
    System.out.println("OLSRD");
    play(loop);
    
    return(START_NOT_STICKY);
  }
  
  @Override
  public void onDestroy() {
    try {
		stop();
	} catch (IOException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
  }
  
  @Override
  public IBinder onBind(Intent intent) {
    return(null);
  }
  
  private void play(boolean loop) {
	  if (lines != null){
          try {
			p1 = r.exec("pgrep olsrd");
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
          lines = new BufferedReader(new InputStreamReader(p1.getInputStream()));
		  try {
		  	  inputLine = lines.readLine();
			  if (inputLine != null){
				  isPlaying = true;
			  }else{
				  isPlaying = false;
				  lines.close();
				  p.destroy();
			  }
		  } catch (IOException e) {
			// TODO Auto-generated catch block
			  e.printStackTrace();
		  }

	  }else{
		  
	  }
	  
	  
    if (!isPlaying) {
    	/*
        try {
	        //p = r.exec("su -c \"olsrd -f /data/local/etc/olsrd.conf\"");	
	        //lines = new BufferedReader(new InputStreamReader(p.getInputStream()));
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
        */
        olsrdOn(1);
    	isPlaying=true;
    }
  }
  
  private void stop() throws IOException {
    if (isPlaying) {
    	
        p1 = r.exec("pgrep olsrd");
        lines = new BufferedReader(new InputStreamReader(p1.getInputStream()));
	  	inputLine = lines.readLine();
    	try {
			lines.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	olsrdOff(Integer.parseInt(inputLine));
    	//p.destroy();
    	isPlaying=false;
    }
  }
	@Override
	protected void onHandleIntent(Intent arg0) {
		// TODO Auto-generated method stub
		
	}
}
