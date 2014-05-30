package it.durip_app;

import java.util.Arrays;

import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;

import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;
import android.view.Menu;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;


public class SensorCharts extends Activity implements SensorEventListener{

	
	private static SensorManager managerSensor;
    private static final int HISTORY_SIZE = 50;            // number of points to plot in history
//    private SensorManager sensorMgr = null;
//    private Sensor orSensor3 = null;
//    private Sensor orSensor2 = null;
    private Sensor orSensor = null;

    private XYPlot aprLevelsPlot = null;
    private XYPlot aprHistoryPlot = null;
    private String[] labels;
//    private CheckBox hwAcceleratedCb;
//    private CheckBox showFpsCb;
    private SimpleXYSeries aprLevelsSeries = null;
    private SimpleXYSeries azimuthHistorySeries = null;
    private SimpleXYSeries pitchHistorySeries = null;
    private SimpleXYSeries rollHistorySeries = null;
    
    final String[] barLabel = new String[] {
            "X","Y","Z"
        };
//    private OutputStream outGravity = null;
//    private OutputStream outLinear = null;
//    private OutputStream outRotation = null;
//    private Writer writeGravity;
//    private Writer writeRotation;
//    private Writer writeLinear;
    //Number to identify sensor to plot
    private int nr;
	
	
	
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		managerSensor = (SensorManager) getApplicationContext().getSystemService(Context.SENSOR_SERVICE);
		setContentView(R.layout.activity_sensor_charts);
		Intent myIntent = getIntent();
		nr = myIntent.getIntExtra( "SensorName" ,1 );
		
		System.out.println("NR value" + nr);
		registerSensor();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.sensor_charts, menu);
		return true;
	}

	
	
	
	
	
	
	
	
	
	//PARTE DEL SENSORE
    private void registerSensor(){
        // register for orientation sensor events:
//        sensorMgr = managerSensor;
        System.out.println("REGISTER SENSOR");
        switch(nr){
            case 0:
            
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ACCELEROMETER)) {
	                if (sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 1:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_GYROSCOPE)) {
	                if (sensor.getType() == Sensor.TYPE_GYROSCOPE) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 2:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ORIENTATION)) {
	                if (sensor.getType() == Sensor.TYPE_ORIENTATION) {
	                    orSensor = sensor;
	                }
	            }
	            break;
	            
//FUSION VALUE		            
            case 4:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_LINEAR_ACCELERATION)) {
	                if (sensor.getType() == Sensor.TYPE_LINEAR_ACCELERATION) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 5:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_GRAVITY)) {
	                if (sensor.getType() == Sensor.TYPE_GRAVITY) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 7:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ROTATION_VECTOR)) {
	                if (sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 8:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_LIGHT)) {
	                if (sensor.getType() == Sensor.TYPE_LIGHT) {
	                    orSensor = sensor;
	                }
	            }
	            break; 
        }
        

        // if we can't access the orientation sensor then exit:
        if (orSensor == null) {
            System.out.println("Failed to attach to orSensor.");
            cleanup();
        }else{
        	managerSensor.registerListener(this, orSensor, SensorManager.SENSOR_DELAY_UI);
        }
    	
    }

    private void cleanup() {
        // unregister with the orientation sensor before exiting:
    	managerSensor.unregisterListener(this);
        //finish();
    }
    
    //TO CHECK
    
    // Called whenever a new orSensor reading is taken.
    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent) {
        // update instantaneous data:
        if(nr < 7){
//        	Number[] series1Numbers = {sensorEvent.values[0], sensorEvent.values[1], sensorEvent.values[2]};
//        	if (nr%2==0){
//        		//TO DRAW ON BAR PLOT
//        		aprLevelsSeries.setModel(Arrays.asList(series1Numbers), SimpleXYSeries.ArrayFormat.Y_VALS_ONLY);
//        		// redraw the Plots:
//        		aprLevelsPlot.redraw();
//        	}else{
        		// get rid the oldest sample in history:
        		if (rollHistorySeries.size() > HISTORY_SIZE) {
        			rollHistorySeries.removeFirst();
        			pitchHistorySeries.removeFirst();
        			azimuthHistorySeries.removeFirst();
        		}

        		// add the latest history sample:
        		azimuthHistorySeries.addLast(null, sensorEvent.values[0]);
        		pitchHistorySeries.addLast(null, sensorEvent.values[1]);
        		rollHistorySeries.addLast(null, sensorEvent.values[2]);
        		aprHistoryPlot.redraw();
//        	}
    	}else{
        	Number[] series1Numbers = {sensorEvent.values[0], 0, 0};
//        	if (nr%2==0){
//        		//TO DRAW ON BAR PLOT
//        		aprLevelsSeries.setModel(Arrays.asList(series1Numbers), SimpleXYSeries.ArrayFormat.Y_VALS_ONLY);
//        		// redraw the Plots:
//        		aprLevelsPlot.redraw();
//        	}else{
        		// get rid the oldest sample in history:
        		if (rollHistorySeries.size() > HISTORY_SIZE) {
        			rollHistorySeries.removeFirst();
        			pitchHistorySeries.removeFirst();
        			azimuthHistorySeries.removeFirst();
        		}

        		// add the latest history sample:
        		azimuthHistorySeries.addLast(null, sensorEvent.values[0]);
        		pitchHistorySeries.addLast(null, 0);
        		rollHistorySeries.addLast(null, 0);
        		aprHistoryPlot.redraw();   
//        	}
    	}
        
    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {
        // Not interested in this event
    }
}
	
	
	
	
	
	

