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
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.IBinder;
import android.util.Log;

public class Sensors extends Service implements SensorEventListener {
	public Sensors() {
	}

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
	private static SensorManager managerSensor = null;
    private static Sensor orSensor3 = null;
    private static Sensor orSensor2 = null;
    private static Sensor orSensor1 = null;
    private static OutputStream outGravity = null;
    private static OutputStream outLinear = null;
    private static OutputStream outRotation = null;
    private static Writer writeGravity;
    private static Writer writeRotation;
    private static Writer writeLinear;
	  @Override
	public int onStartCommand(Intent intent, int flags, int startId) {
	    boolean loop=intent.getBooleanExtra(LOOP, false);
        managerSensor = (SensorManager) getApplicationContext().getSystemService(Context.SENSOR_SERVICE);
		try {
			outGravity = new FileOutputStream("/sdcard/Service_GRAVITY.txt", true);
			outLinear = new FileOutputStream("/sdcard/Service_LINEAR_ACCELERATION.txt", true);
			outRotation = new FileOutputStream("/sdcard/Service_MAGNETIC_ROTATION.txt", true);
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        try {
			writeGravity = new OutputStreamWriter(outGravity, "UTF-8");
			writeLinear = new OutputStreamWriter(outLinear, "UTF-8");
			writeRotation = new OutputStreamWriter(outRotation, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	try {
			writeGravity.append("TIME Xm/s2 Ym/s2 Zm/s2\n");
			writeLinear.append("TIME Xm/s2 Ym/s2 Zm/s2\n");
			writeRotation.append("TIME X Y Z\n");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	    play(loop);
	    
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
	  
	private void play(boolean loop) {

	    if (!isPlaying) {
	    	
	        for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ALL)) {
			    if (sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
			        orSensor1 = sensor;
			    }
			    if (sensor.getType() == Sensor.TYPE_GRAVITY) {
			        orSensor2 = sensor;
			    }
			    if (sensor.getType() == Sensor.TYPE_LINEAR_ACCELERATION) {
			        orSensor3 = sensor;
			    }
			}
			//really bad programmation example! to be re-designed!
			managerSensor.registerListener(this, orSensor1, SensorManager.SENSOR_DELAY_NORMAL);
			managerSensor.registerListener(this, orSensor2, SensorManager.SENSOR_DELAY_NORMAL);
			managerSensor.registerListener(this, orSensor3, SensorManager.SENSOR_DELAY_NORMAL);
	        
	    	isPlaying=true;
	    }
	}
	  
	private void stop() {
	    if (isPlaying) {
	    	try {
				writeGravity.close();
				writeRotation.close();
				writeLinear.close();
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

        // update instantaneous data:
		//System.out.println(sensorEvent.sensor.getType() + " VALS: "+ sensorEvent.values[0] + " - " + sensorEvent.values[1] + " - "+sensorEvent.values[2]);
    	if(sensorEvent.sensor.getType() == Sensor.TYPE_GRAVITY) {
        	try {
				writeGravity.append(System.currentTimeMillis() + " " + sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
    	}
    	if(sensorEvent.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
        	try {
				writeRotation.append(System.currentTimeMillis() + " " + sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
    	}           	
    	if(sensorEvent.sensor.getType() == Sensor.TYPE_LINEAR_ACCELERATION) {
        	try {
				writeLinear.append(System.currentTimeMillis() + " " + sensorEvent.values[0] + " " + sensorEvent.values[1] + " " + sensorEvent.values[2] + "\n");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
    	}
    
    }

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
		
	}
}
