package it.durip_app;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.PointLabelFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;
import com.androidplot.xy.XYStepMode;

import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.SystemClock;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;



public class BatteryCharts extends Activity {

	
	

    private static final int HISTORY_SIZE = 50;            // number of points to plot in history
//    private SensorManager sensorMgr = null;
//    private Sensor orSensor3 = null;
//    private Sensor orSensor2 = null;
    

//    private XYPlot aprLevelsPlot = null;
//    private SimpleXYSeries aprLevelsSeries = null;
    
    
    private XYPlot aprHistoryPlot = null;
    private String[] labels;

    
    private SimpleXYSeries xBattery = null;
    
    private Thread batteryChart;
    final String[] barLabel = new String[] {
            "X","Y","Z"
        };
	
    
    private runnableChart r;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_battery_charts);
		
		System.out.println("MATTEO " + getValueFromFile());
		
        	// setup the APR History plot:
		aprHistoryPlot = (XYPlot)findViewById(R.id.timeserieBatteryChart);

		xBattery = new SimpleXYSeries("Battery");
		xBattery.useImplicitXVals();
		

		aprHistoryPlot.setRangeBoundaries(-180, 359, BoundaryMode.AUTO);
		aprHistoryPlot.setDomainBoundaries(0, 300, BoundaryMode.AUTO);
		PointLabelFormatter point1 = new  PointLabelFormatter(Color.rgb(255, 255, 255)); 
		aprHistoryPlot.addSeries(xBattery, new LineAndPointFormatter(Color.rgb(100, 100, 200), Color.BLUE, null , point1));
//
//
		aprHistoryPlot.setDomainStep(XYStepMode.INCREMENT_BY_VAL ,5);
		aprHistoryPlot.setTicksPerRangeLabel(3);
//		//	                
		aprHistoryPlot.getDomainLabelWidget().pack();
		aprHistoryPlot.getRangeLabelWidget().pack();	               


		r = new runnableChart();
		new Thread(r).start();
	        
	        
	        

		
	
	}
	private class runnableChart implements Runnable {
		
		private boolean doRun = true;
		@Override
		public void run(){
			while(doRun){		            	
				try {
					System.out.println("Value +" + getValueFromFile());

	        		if (xBattery.size() > HISTORY_SIZE) {
	        			
	        			xBattery.removeFirst();
	        		}
	        		// add the latest history sample:
	        		xBattery.addLast(null, getValueFromFile());	        	
	        		aprHistoryPlot.redraw();
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}

		}
		public void stopThread(){
			System.out.println("close");
			doRun = false;

		}
		       
	
	}
		

	
	@Override
	protected void onDestroy() {
		super.onDestroy();		
		r.stopThread();
	}
	
	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.battery_charts, menu);
		return true;
	}
	

	
	
	
	
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
	
	
}
