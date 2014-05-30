package it.durip_app;

import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
import java.util.Arrays;

import com.androidplot.xy.BarFormatter;
import com.androidplot.xy.BarRenderer;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.PointLabelFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;
import com.androidplot.xy.XYStepMode;

import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;
//import android.view.LayoutInflater;
import android.view.Menu;
//import android.view.View;
//import android.view.ViewGroup;
//import android.hardware.Sensor;
//import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
//import android.hardware.SensorManager;


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
		
		            
        switch(nr){
            case 1:            
	            labels = new String[] {"X","Y","Z","Accelerometer" , "m/s^2"};
	            break;

            case 2:       
	            labels = new String[] {"X-axis","Y-axis","Z-axis","Gyroscope" , "rad/sec" };
	            break;
            case 3:
	            labels = new String[] {"Azimuth","Pitch","Roll","Orientation" , "Deg"};
	            break;
	            //A three dimensional vector indicating acceleration along each device axis, 
//	            not including gravity. All values have units of m/s^2
            case 4:
	            labels = new String[] {"X","Y","Z","Linear Acceleration" ,"m/s^2"};
	            break;
            case 5:
	            labels = new String[] {"Z+X+Y/rad+Z/rad","Y+Z/rad+X/rad","Null->orientation","Gravity" , "m/s^2"};
	            break;
            case 6:
	            labels = new String[] {"Y.Z tgGround East","tgGround North","PerpGround","Rotation Vector" , "TBD"};
	            break;
            case 7:
	            labels = new String[] {"Light","None","None","Light" , "Si"};
	            break;
	            
        }

//        View rootView;
//
//        rootView = inflater.inflate(R.layout.sensors, container, false);

        //                ((TextView) rootView.findViewById(R.id.texxt)).setText(
        //                        getString(R.string.plotNr, nr));

        //                hwAcceleratedCb = (CheckBox) rootView.findViewById(R.id.hwAccelerationCb);
        //                showFpsCb = (CheckBox) rootView.findViewById(R.id.showFpsCb);


        	// setup the APR History plot:
        	aprHistoryPlot = (XYPlot)findViewById(R.id.timeserieChart);

        	
        	
        	
        	azimuthHistorySeries = new SimpleXYSeries(labels[0]);
        	azimuthHistorySeries.useImplicitXVals();
        	pitchHistorySeries = new SimpleXYSeries(labels[1]);
        	pitchHistorySeries.useImplicitXVals();
        	rollHistorySeries = new SimpleXYSeries(labels[2]);
        	rollHistorySeries.useImplicitXVals();

        	//	                aprHistoryPlot.setRangeBoundaries(-180, 359, BoundaryMode.AUTO);
        	//	                aprHistoryPlot.setDomainBoundaries(0, 300, BoundaryMode.AUTO);
        	PointLabelFormatter point1 = new  PointLabelFormatter(Color.rgb(255, 255, 255)); 
        	PointLabelFormatter point2 = new  PointLabelFormatter(Color.rgb(0, 0, 0)); 
        	PointLabelFormatter point3 = new  PointLabelFormatter(Color.rgb(200, 100, 100)); 
//
        	aprHistoryPlot.addSeries(azimuthHistorySeries, new LineAndPointFormatter(Color.rgb(100, 100, 200), Color.BLUE, null , point1));
        	aprHistoryPlot.addSeries(pitchHistorySeries, new LineAndPointFormatter(Color.rgb(100, 200, 100), Color.BLACK, null , point2));
        	aprHistoryPlot.addSeries(rollHistorySeries, new LineAndPointFormatter(Color.rgb(200, 100, 100), Color.RED, null , point3));

        	aprHistoryPlot.setDomainStep(XYStepMode.INCREMENT_BY_VAL ,5);

        	aprHistoryPlot.setTicksPerRangeLabel(3);
        	//	                
        	aprHistoryPlot.getDomainLabelWidget().pack();
        	//	                aprHistoryPlot.getBackgroundPaint().setAlpha(0);
        	//	                aprHistoryPlot.getGraphWidget().getBackgroundPaint().setAlpha(0);
        	//	                aprHistoryPlot.getGraphWidget().getGridBackgroundPaint().setAlpha(0);
        	aprHistoryPlot.setRangeLabel(labels[4]);
        	aprHistoryPlot.setTitle(labels[3]);
        	aprHistoryPlot.getRangeLabelWidget().pack();	               


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
            case 1:
            
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ACCELEROMETER)) {
	                if (sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 2:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_GYROSCOPE)) {
	                if (sensor.getType() == Sensor.TYPE_GYROSCOPE) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 3:
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
            case 6:
	            for (Sensor sensor : managerSensor.getSensorList(Sensor.TYPE_ROTATION_VECTOR)) {
	                if (sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
	                    orSensor = sensor;
	                }
	            }
	            break;
            case 7:
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
//        	Number[] seriesNumbers = {sensorEvent.values[0], 0, 0};
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
	
	
	
	
	
	

