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

//import it.durip_app.MACSource.MyObservable;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
//import java.util.Observable;

//import android.app.ActivityManager;
//import android.app.ActivityManager.RunningServiceInfo;
import android.app.Service;
//import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
//import android.view.LayoutInflater;
//import android.view.View;
//import android.widget.EditText;
//import android.widget.Toast;

public class Iperf extends Service {
	/*
    // encapsulates management of the observers watching this datasource for update events:
    class MyObservable extends Observable {
	    @Override
	    public void notifyObservers() {
	        setChanged();
	        super.notifyObservers();
	        
	    }
    }*/
  public Iperf() {}

  public static final String LOOP="LOOP";
  public static final String DESTINATION="DESTINATION";
  public static final String TIME="TIME";
  public static final String INTERVAL="INTERVAL";
  private boolean isPlaying=false;
  private String IperfCmd = "";
  private static Process p=null;
  private static BufferedReader lines=null;
  private static Runtime r = Runtime.getRuntime();
  private String inputLine="";
  private static String ip = "";
  private static String t = "";
  private static String i = "";
  static StringBuilder infos = null;
  /*
  private MyObservable notifier; 
  {
	  notifier = new MyObservable();
  }*/
  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {

    ip=intent.getStringExtra(DESTINATION);
    t=intent.getStringExtra(TIME);
    i=intent.getStringExtra(INTERVAL);
    boolean loop=intent.getBooleanExtra(LOOP, false);
    play(ip, t, i, loop);
    
    try {        
    	inputLine = lines.readLine();
		//pingresults.setText(inputLine + "\n");	
    	/*
		while (inputLine != null){
			if (infos == null) infos = new StringBuilder();
			infos.append(inputLine+ "\n");
			//notifier.notifyObservers();
			inputLine = lines.readLine();
			
		}
		*/

		//pingresults.setText(infos);
	} catch (IOException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
    
    return(START_NOT_STICKY);
  }
  
  @Override
  public void onDestroy() {
    stop();
  }
  
  @Override
  public IBinder onBind(Intent intent) {
    return(null);
  }
  
  public static String buffer(){
	  String tmp = infos.toString();
	  infos = null;
	  return tmp;
	  
  }
  
  private void play(String paramUrl, String paramT, String paramI, boolean loop) {
	  IperfCmd = "iperf -c "+ paramUrl +" -t "+paramT+" -i "+paramI;
	  if (lines != null){
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
    	
        try {
	        p = r.exec(IperfCmd);	
	        lines = new BufferedReader(new InputStreamReader(p.getInputStream()));
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
    	isPlaying=true;
    }
  }
  
  private void stop() {
    if (isPlaying) {
    	try {
			lines.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	p.destroy();
    	isPlaying=false;
    }
  }
}
