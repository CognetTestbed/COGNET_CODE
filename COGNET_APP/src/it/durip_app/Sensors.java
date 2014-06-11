/*
Cognetive Network APP 

Copyright (C) 2014  Matteo Danieletto matteo.danieletto@dei.unipd.it
University of Padova, Italy +39 049 827 7778
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

//TYPE_ACCELEROMETER uses the accelerometer and only the accelerometer. It returns raw accelerometer events, with minimal or no processing at all.
//
//TYPE_GYROSCOPE (if present) uses the gyroscope and only the gyroscope. Like above, it returns raw events (angular speed un rad/s) with no processing at all (no offset / scale compensation).
//
//TYPE_ORIENTATION is deprecated. It returns the orientation as yaw/ pitch/roll in degres. It's not very well defined and can only be relied upon when the device has no "roll". This sensor uses a combination of the accelerometer and the magnetometer. Marginally better results can be obtained using SensorManager's helpers. This sensor is heavily "processed".
//
//TYPE_LINEAR_ACCELERATION, TYPE_GRAVITY, TYPE_ROTATION_VECTOR are "fused" 
//sensors which return respectively the linear acceleration, gravity and rotation vector 
//(a quaternion). It is not defined how these are implemented. 
//On some devices they are implemented in h/w, on some devices they use the accelerometer + 
//the magnetometer, on some other devices they use the gyro.

package it.durip_app;

//import java.io.BufferedReader;


import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
//import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.text.SimpleDateFormat;
//import java.text.DateFormat;
//import java.text.SimpleDateFormat;
import java.util.Calendar;
//import java.util.Date;


//import java.util.Date;

import java.util.Locale;





import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.BatteryManager;
import android.os.Environment;
import android.os.IBinder;
//import android.text.format.DateFormat;
import android.util.Log;
//import android.util.Log;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.view.LayoutInflater;
import android.view.View;

public class Sensors extends Service implements SensorEventListener {
	public Sensors() {
	}

	public static final String LOOP="LOOP";
	public static final String DESTINATION="DESTINATION";
	public static final String TIME="TIME";
	public static final String INTERVAL="INTERVAL";
	/*
	 * FILE NAME*/
	private static final String PATH_SENSOR_FOLDER = "/local/SensorLog/";
	
	
	private static final int TS_SENSOR = 1000; 
	private boolean isPlaying=false;
	private static SensorManager managerSensor = null;
	private static Sensor logSensor [] = new Sensor[7];
//	private static Sensor orSensor2 = null;
//	private static Sensor orSensor3 = null;
    
    
    
    private int  level;
	private static OutputStream outAcceleration = null;
    private static OutputStream outGyroscope = null;    
    private static OutputStream outOrientation = null; 
    private static OutputStream outGravity = null;
    private static OutputStream outLinearAcceleration = null;
    private static OutputStream outRotation = null;
    private static OutputStream outLigth = null;
    private static OutputStream outBattery= null;
    private static OutputStream outMove= null;
    
    
    private static Writer writeAcceleration;
    private static Writer writeGyroscope;
    private static Writer writeOrientation;
    private static Writer writeGravity;
    private static Writer writeLinearAcceleration;   
    private static Writer writeRotation;    
    private static Writer writeLigth;
    private static Writer writeBattery;
    private static Writer writeMove;
    
    private runnableChart r;
    private SimpleDateFormat formatTime = new SimpleDateFormat("HH:mm:ss.S" , Locale.ITALY);
   
    private ToggleButton tb;
	private class runnableChart implements Runnable {
		
		private boolean doRun = true;		
		@Override
		public void run(){
			
			while(doRun){		            	
				try {
					try {

						writeBattery.append(formatTime.format(System.currentTimeMillis()) + 
								" " +  getValueFromFile() + " " + getValueVoltageFromFile() + " " + level + 
								" "+getValueMHzCPUFile(0) +" "+ getValueMHzCPUFile(1) +" " +
								getValueMHzCPUFile(2) +" "+ getValueMHzCPUFile(3) +" " 
								+"\n");
						
						writeMove.append(formatTime.format(System.currentTimeMillis()) + " "+ tb.isChecked() +"\n");
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					//DA PORTARE FUORI
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
//				}finally{
//					try {
					
//					} catch (IOException e) {
//						// TODO Auto-generated catch block
//						e.printStackTrace();
//					}
				}
		

		}
		public void stopThread(){
			System.out.println("close");
			doRun = false;	
			try {
				writeBattery.close();
				writeMove.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		       
	
	}
    
    
    @Override
	public int onStartCommand(Intent intent, int flags, int startId) {
	    boolean loop=intent.getBooleanExtra(LOOP, false);
	    Calendar c = Calendar.getInstance();
//	    LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
	   
	    		
//	    		(TextView)findViewById(R.id.textvalueConsumption);
	    
	    int hh     = c.get(Calendar.HOUR);
	    int mm     = c.get(Calendar.MINUTE);
	    int second = c.get(Calendar.HOUR);
	    int month  = c.get(Calendar.DAY_OF_MONTH);
	    int day    = c.get(Calendar.MONTH)+1;
	    LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
	    
	    
	    View layout =inflater.inflate(R.layout.activity_main , null);
	    tb = (ToggleButton) layout.findViewById(R.id.toggleButtonMove);
	    File file = new File(Environment.getExternalStorageDirectory(), PATH_SENSOR_FOLDER);
	    if (!file.exists()) {
	        if (!file.mkdirs()) {
	            Log.e("TravellerLog :: ", "Problem creating Image folder");
	        }
	    }
	    
	    String fileAcceleration =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logAcceleration_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    String fileGyroscope=  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logGyroscope_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    String fileOrientation=  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logOrientation_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    String fileLinearAcceleration =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logLinearAcceleration_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;

	    String fileGravity =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logGravity_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    String fileRotation =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logRotation_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    String fileLigth =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logLigth_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    String fileBattery =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logBattery_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;

	    String fileLogMove =  Environment.getExternalStorageDirectory().getPath()+PATH_SENSOR_FOLDER +
	    		"logMove_"+hh+"_"+mm+"_"+second+"_"+month+"_"+day;
	    
	    managerSensor = (SensorManager) getApplicationContext().getSystemService(Context.SENSOR_SERVICE);
		
        
        try {
        	outAcceleration = new FileOutputStream(fileAcceleration, true);
        	outGyroscope = new FileOutputStream(fileGyroscope, true);
        	outOrientation = new FileOutputStream(fileOrientation, true);
        	outLinearAcceleration = new FileOutputStream(fileLinearAcceleration, true);        	
			outGravity = new FileOutputStream(fileGravity, true);			
			outRotation = new FileOutputStream(fileRotation, true);
			outLigth = new FileOutputStream(fileLigth, true);
			outBattery = new FileOutputStream(fileBattery, true);
			outMove = new FileOutputStream(fileLogMove, true);
			
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block			
			e.printStackTrace();
		}
        
        
        
        try {
        	writeAcceleration= new OutputStreamWriter(outAcceleration, "UTF-8");
        	writeGyroscope= new OutputStreamWriter(outGyroscope, "UTF-8");
        	writeOrientation= new OutputStreamWriter(outOrientation, "UTF-8");
			writeLinearAcceleration = new OutputStreamWriter(outLinearAcceleration, "UTF-8");
			writeGravity = new OutputStreamWriter(outGravity, "UTF-8");
			writeRotation = new OutputStreamWriter(outRotation, "UTF-8");
			writeLigth = new OutputStreamWriter(outLigth, "UTF-8");
			writeBattery= new OutputStreamWriter(outBattery, "UTF-8");
			writeMove= new OutputStreamWriter(outMove, "UTF-8");
			
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	    play(loop);
	    	   	    
	    this.registerReceiver(this.mBatInfoReceiver,  new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
			
			
               
			

	    r = new runnableChart();
	    new Thread(r).start();
	    
	    
	    
	    return(START_NOT_STICKY);
	}
	  
	@Override
	public void onDestroy() {
	    stop();
	    r.stopThread();
		this.unregisterReceiver(this.mBatInfoReceiver);
	}
	  
	@Override
	public IBinder onBind(Intent intent) {
	    return(null);
	}
	  
	private void play(boolean loop) {

	    if (!isPlaying) {
	    	
//	    	System.out.println("TEST");
	    	//THIS SENSOR REPORT ONLY THE FUSION VALUE
	    	
	        for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ALL)) {
	        	
	        	switch (sensor.getType()){
//	        		case Sensor.TYPE_ACCELEROMETER:
//	        			logSensor[0] = sensor;
//				        managerSensor.registerListener(this, logSensor[0], TS_SENSOR);
//	        			break;
	        		case Sensor.TYPE_GYROSCOPE:
	        			logSensor[1] = sensor;
				        managerSensor.registerListener(this, logSensor[1], TS_SENSOR);
	        			break;
	        		case Sensor.TYPE_ORIENTATION:
	        			logSensor[2] = sensor;
				        managerSensor.registerListener(this, logSensor[2], TS_SENSOR);
	        			break;
	        		case Sensor.TYPE_LINEAR_ACCELERATION:
	        			logSensor[3] = sensor;
				        managerSensor.registerListener(this, logSensor[3], TS_SENSOR);
	        			break;
//	        		case Sensor.TYPE_GRAVITY:
//	        			logSensor[4] = sensor;
//				        managerSensor.registerListener(this, logSensor[4], TS_SENSOR);
//	        			break;
//	        		case Sensor.TYPE_ROTATION_VECTOR:
//	        			logSensor[5] = sensor;
//				        managerSensor.registerListener(this, logSensor[5], TS_SENSOR);
//	        			break;
	        		case Sensor.TYPE_LIGHT:
	        			logSensor[6] = sensor;
				        managerSensor.registerListener(this, logSensor[6], TS_SENSOR);
	        			break;
	        	}
			}

			
	    	isPlaying=true;
	    }
	}
	  
	private void stop() {
	    if (isPlaying) {
	    	try {
	    		writeAcceleration.close();
				writeGravity.close();
				writeRotation.close();
				writeLinearAcceleration.close();
		        writeGyroscope.close();
		        writeOrientation.close();		          		            
		        writeLigth.close();				
				managerSensor.unregisterListener(this);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	    	
	    	isPlaying=false;
	    }
	}
	
    // Called whenever a new orSensor reading is taken.
    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent) {
    	
    	//    	formatTime.format(new Date());
    	// update instantaneous data:
    	//System.out.println(sensorEvent.sensor.getType() + " VALS: "+ sensorEvent.values[0] + " - " + sensorEvent.values[1] + " - "+sensorEvent.values[2]);
    	if(isPlaying){


    
    		switch (sensorEvent.sensor.getType()){
    			case Sensor.TYPE_ACCELEROMETER:
    				try {
    					//    				writeAcceleration.append(formatTime.format(System.currentTimeMillis()) + " " 
    					//    						+ sensorEvent.values[0] + " "+ sensorEvent.values[1] + " " + sensorEvent.values[2] +
    					//    						sensorEvent.values[3] + " " + sensorEvent.values[4] + " " + sensorEvent.values[5] +"\n");
    					writeLinearAcceleration.append(formatTime.format(System.currentTimeMillis()) + " " 
    							+ sensorEvent.values[0] + " "+ sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n"); //+
    					//    						sensorEvent.values[3] + " " + sensorEvent.values[4] + " " + sensorEvent.values[5] +"\n")
    				} catch (IOException e) {
    					// TODO Auto-generated catch block
    					e.printStackTrace();
    				}
    			break;
    			case Sensor.TYPE_GYROSCOPE:
    				try {
    					writeGyroscope.append(formatTime.format(System.currentTimeMillis()) + 
    							" " + sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
    				}catch (IOException e) {
    					// TODO Auto-generated catch block
    					e.printStackTrace();
    				}
    				break;
    		case Sensor.TYPE_ORIENTATION:
    			try {
    				writeOrientation.append(formatTime.format(System.currentTimeMillis()) + 
    						" " + sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
    			}catch (IOException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
    			break;
    		case Sensor.TYPE_LINEAR_ACCELERATION:
    			try {
    				writeLinearAcceleration.append(formatTime.format(System.currentTimeMillis()) + " " 
    						+ sensorEvent.values[0] + " "+ sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n"); //+
    				//    						sensorEvent.values[3] + " " + sensorEvent.values[4] + " " + sensorEvent.values[5] +"\n");
    			} catch (IOException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
    			break;
    		case Sensor.TYPE_GRAVITY:
    			try {
    				writeGravity.append(formatTime.format(System.currentTimeMillis()) + 
    						" " + sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
    			} catch (IOException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
    			break;
    		case Sensor.TYPE_ROTATION_VECTOR:
    			try {
    				writeRotation.append(formatTime.format(System.currentTimeMillis()) + " " 
    						+ sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
    			} catch (IOException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
    			break;
    		case Sensor.TYPE_LIGHT:

    			try {
    				writeLigth.append(formatTime.format(System.currentTimeMillis()) + 
    						" " + sensorEvent.values[0] +"\n");
    			}catch (IOException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}

    			break;
    		}
    	}
    }

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
		
	}
	
	
	
	
	
	private BroadcastReceiver mBatInfoReceiver = new BroadcastReceiver(){
	      @Override
	      public void onReceive(Context arg0, Intent intent) {
	    	  
	

	    	  level= intent.getIntExtra(BatteryManager.EXTRA_LEVEL,0);
	    	  boolean  present= intent.getExtras().getBoolean(BatteryManager.EXTRA_PRESENT); 
	    	  int  temperature= intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE,0);

	      }
	    };
	
	
	private static Long getValueFromFile() {
	    
		String text = null;
		File f = null; 
		f = new File("/sys/class/power_supply/battery/current_now");
		try {

			FileInputStream fs = new FileInputStream(f);	      
			DataInputStream ds = new DataInputStream(fs);

			text = ds.readLine();

			ds.close();    
			fs.close();  

		}
		catch (Exception ex) {
			ex.printStackTrace();
		}

		Long value = null;

		if (text != null)
		{
			try
			{
				value = Long.parseLong(text);
			}
			catch (NumberFormatException nfe)
			{
				value = null;
			}	    	      
			value = value/1000; // convert to milliampere
		}
		return value;
	}
	
	
	
	
	
	private static Long getValueVoltageFromFile() {
	    
		String text = null;
		File f = null; 
		f = new File("/sys/class/power_supply/battery/voltage_now");
		try {

			FileInputStream fs = new FileInputStream(f);	      
			DataInputStream ds = new DataInputStream(fs);

			text = ds.readLine();

			ds.close();    
			fs.close();  

		}
		catch (Exception ex) {
			ex.printStackTrace();
		}

		Long value = null;

		if (text != null)
		{
			try
			{
				value = Long.parseLong(text);
			}
			catch (NumberFormatException nfe)
			{
				value = null;
			}	    	      	
		}
		return value/1000;
	}
	
	private static Long getValueMHzCPUFile(int cpuNumber) {
	    
		String text = null;
		File f = null; 
		f = new File("/sys/devices/system/cpu/cpu"+cpuNumber+"/cpufreq/cpuinfo_cur_freq");
		try {

//			System.out.println("/sys/devices/system/cpu/cpu"+cpuNumber+"/cpufreq/cpuinfo_cur_freq");
			FileInputStream fs = new FileInputStream(f);	      
			DataInputStream ds = new DataInputStream(fs);

			text = ds.readLine();

			ds.close();    
			fs.close();  

			Long value = null;

//			System.out.println(text);
			if(Long.parseLong(text) > 1000)
				return Long.parseLong(text)/1000;
			else
				return Long.parseLong(text);
		}
		catch (Exception ex) {
			return (long)0;
		}
	}
	

}
