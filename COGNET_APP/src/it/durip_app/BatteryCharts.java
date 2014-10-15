package it.durip_app;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
//import java.io.IOException;

import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.PointLabelFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;
import com.androidplot.xy.XYStepMode;

import android.graphics.Color;
//import android.hardware.Sensor;
//import android.hardware.SensorManager;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.Handler;
//import android.os.SystemClock;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
//import android.view.Menu;
import android.widget.TextView;



public class BatteryCharts extends Activity {


    private static final int HISTORY_SIZE = 50;            // number of points to plot in history
    private int ts ;
    private XYPlot aprHistoryPlot = null;
    private TextView textCharge;
    private TextView textVoltage;
    private TextView textTemp;
    
    private TextView textCPU0;
    private TextView textCPU1;
    private TextView textCPU2;
    private TextView textCPU3;
    private SimpleXYSeries xBattery = null;
    
//    private Thread batteryChart;
    final String[] barLabel = new String[] {
            "X","Y","Z"
        };
	
    
    private runnableChart r;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_battery_charts);
		textCharge = (TextView) findViewById(R.id.textvalueCharge);
   	 	textTemp = (TextView) findViewById(R.id.textvalueTemp);
		this.registerReceiver(this.mBatInfoReceiver,  new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
		
		
   	 	

   	 	Intent myIntent = getIntent();
		aprHistoryPlot = (XYPlot)findViewById(R.id.timeserieBatteryChart);

		xBattery = new SimpleXYSeries("Battery");
		xBattery.useImplicitXVals();
		

		aprHistoryPlot.setRangeBoundaries(-1000, 200, BoundaryMode.FIXED);
		aprHistoryPlot.setDomainBoundaries(0, 300, BoundaryMode.AUTO);
		PointLabelFormatter point1 = new  PointLabelFormatter(Color.rgb(255, 255, 255)); 
		aprHistoryPlot.addSeries(xBattery, new LineAndPointFormatter(Color.rgb(100, 100, 200), Color.BLUE, null , point1));
//
//
		aprHistoryPlot.setDomainStep(XYStepMode.INCREMENT_BY_VAL ,5);
		aprHistoryPlot.setTicksPerRangeLabel(3);        
		aprHistoryPlot.getDomainLabelWidget().pack();
		aprHistoryPlot.getRangeLabelWidget().pack();	               
		
		aprHistoryPlot.setTitle("Current Consumption");
		aprHistoryPlot.setDomainLabel("[S]");
		aprHistoryPlot.setRangeLabel("mA");
		
		ts = myIntent.getIntExtra( "timesample" ,1 );
		r = new runnableChart();
		new Thread(r).start();

	}
	
	private class runnableChart implements Runnable {
		
		private boolean doRun = true;
		 Handler handler = new Handler();
		@Override
		public void run(){
			textVoltage = (TextView) findViewById(R.id.textvalueVoltage);
			textCPU0 = (TextView) findViewById(R.id.textvalueCPU0);
			textCPU1 = (TextView) findViewById(R.id.textvalueCPU1);
			textCPU2 = (TextView) findViewById(R.id.textvalueCPU2);
			textCPU3 = (TextView) findViewById(R.id.textvalueCPU3);
			while(doRun){		            	
				try {
//					System.out.println("Value " + getValueFromFile());

	        		if (xBattery.size() > HISTORY_SIZE) {
	        			
	        			xBattery.removeFirst();
	        		}
	        		// add the latest history sample:
	        		xBattery.addLast(null, getValueFromFile());	        		
	        		aprHistoryPlot.redraw();
	        		
					
					handler.post(new Runnable(){
						 public void run() {
							 textVoltage.setText(getValueVoltageFromFile() + "mV");
							 textCPU0.setText(getValueMHzCPUFile(0) + "MHz");
							 textCPU1.setText(getValueMHzCPUFile(1) + "MHz");
							 textCPU2.setText(getValueMHzCPUFile(2) + "MHz");
							 textCPU3.setText(getValueMHzCPUFile(3) + "MHz");
						 }
					});
					Thread.sleep(ts * 1000);
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

	
	private BroadcastReceiver mBatInfoReceiver = new BroadcastReceiver(){
	      @Override
	      public void onReceive(Context arg0, Intent intent) {
	    	  
	          //this will give you battery current status
//	        int level = intent.getIntExtra("level", 0);
//
////	        contentTxt.setText(String.valueOf(level) + "%");
//
//	        int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
////	        textView2.setText("status:"+status);
//	        boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING ||
//	                            status == BatteryManager.BATTERY_STATUS_FULL;
////	        textView3.setText("is Charging:"+isCharging);
//	        int chargePlug = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
////	        textView4.setText("is Charge plug:"+chargePlug);
//	        boolean usbCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_USB;
//
//	        boolean acCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_AC;
////	        textView5.setText("USB Charging:"+usbCharge+" AC charging:"+acCharge);
//	    	  int  health= intent.getIntExtra(BatteryManager.EXTRA_HEALTH,0);

	    	  int  level= intent.getIntExtra(BatteryManager.EXTRA_LEVEL,0);
//	    	  int  plugged= intent.getIntExtra(BatteryManager.EXTRA_PLUGGED,0);
	    	  boolean  present= intent.getExtras().getBoolean(BatteryManager.EXTRA_PRESENT); 
//	    	  int  scale= intent.getIntExtra(BatteryManager.EXTRA_SCALE,0);
//	    	  int  status= intent.getIntExtra(BatteryManager.EXTRA_STATUS,0);
//	    	  String  technology= intent.getExtras().getString(BatteryManager.EXTRA_TECHNOLOGY);
	    	  int  temperature= intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE,0);
//	    	  int  voltage= intent.getIntExtra(BatteryManager.EXTRA_VOLTAGE,0);
	    	  
//	    	  textCharge.setText(level);
//	    	  textVoltage.setText(voltage + "V");
	    	  textCharge.setText(level+"%");
	    	  textTemp.setText(temperature/10+"C");
//	    	  System.out.println(voltage +" " + temperature + " " + level);  
	    	  
	      }
	    };
	
	@Override
	protected void onDestroy() {
		super.onDestroy();		
		r.stopThread();
		this.unregisterReceiver(this.mBatInfoReceiver);
	}
	
	
	
//	@Override
//	public boolean onCreateOptionsMenu(Menu menu) {
//		// Inflate the menu; this adds items to the action bar if it is present.
////		getMenuInflater().inflate(R.menu.battery_charts, menu);
//		return true;
//	}
	

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
		//	ex.printStackTrace();
			return (long)0;
		}
	}
}




	
