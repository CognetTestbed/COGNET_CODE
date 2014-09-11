/*
Cognitive Network APP 
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
package it.durip_app;

//import it.durip_app.BatteryCharts.runnableChart;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;



import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.PointLabelFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;
import com.androidplot.xy.XYStepMode;

import android.os.BatteryManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
//import android.util.AttributeSet;
import android.util.Log;
import android.view.Menu;
//import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
//import android.view.View.OnTouchListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
//import android.widget.CompoundButton;
//import android.widget.Button;
import android.widget.EditText;
import android.widget.NumberPicker;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
//import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;
//import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.AdapterView.OnItemSelectedListener;
public class MainActivity extends Activity implements SensorEventListener{

	
	public static final String SHELL_RESULTS = "it.durip_app.RESULTS"; 
	public static final String PARAM_URL = "it.durip_app.PARAM_URL"; 
	public static final String PARAM_T = "it.durip_app.PARAM_T"; 
	public static final String PARAM_I = "it.durip_app.PARAM_I"; 
	public static final String PARAM_PORT = "it.durip_app.PARAM_PORT"; 
	public static final String PARAM_FILENAME = "it.durip_app.PARAM_FILENAME"; 
	public static final String PARAM_MAC = "it.durip_app.PARAM_MAC"; 
	public static final String PARAM_VERBOSE = "it.durip_app.PARAM_VERBOSE"; 
	public static final String PARAM_SLEEP = "it.durip_app.PARAM_SLEEP"; 
	public static final String URL_STREAMING = "streaming";
	public static final String MEDIA_STREAMING = "media";
	public static final String USERNAME = "user"; 
	public static PowerManager.WakeLock wl = null;
	public static PowerManager pm = null;
	private boolean checkOndemandSpinner = false;
	private NumberPicker numberPickerOLSR ;
	private NumberPicker numberPickerSensor;
	
	
//	private int ts;
//	private int tsOLSR;
	//PLOT SECTION
	private static SensorManager managerSensor;
    private static final int HISTORY_SIZE = 10;            // number of points to plot in history
//    private SensorManager sensorMgr = null;
//    private Sensor orSensor3 = null;
//    private Sensor orSensor2 = null;
    private Sensor orSensor = null;

    
    private XYPlot aprHistoryPlot = null;
    private String[] labels;
//    private CheckBox hwAcceleratedCb;
//    private CheckBox showFpsCb;
    
    private SimpleXYSeries azimuthHistorySeries = null;
    private SimpleXYSeries pitchHistorySeries = null;
    private SimpleXYSeries rollHistorySeries = null;      
    private Number [] boundRange = new Number[2];
    //Number to identify sensor to plot
//    private int nr;
    private int plotNumber;
    private int ctrlPlotPage= 0 ;
    
    
    private int ctrlFirstSetup = 1;
    
    private SimpleXYSeries xBattery = null;
    
    private runnableChart r;
    private TextView textCharge;
    private TextView textVoltage;
    private TextView textTemp;
    
    private TextView textCPU0;
    private TextView textCPU1;
    private TextView textCPU2;
    private TextView textCPU3;
    
    private BroadcastReceiver mBatInfoReceiverMain = new BroadcastReceiver(){
        @Override
        public void onReceive(Context arg0, Intent intent) {
      	  
     
      	  int  level= intent.getIntExtra(BatteryManager.EXTRA_LEVEL,0);
      	  int  temperature= intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE,0);

      	  if(plotNumber == 8){
      		  textCharge.setText(level+"% ");
      		  textTemp.setText(temperature/10+"C ");
      	  }
      	  
        }
      };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Spinner macSpinner = (Spinner) findViewById(R.id.mac_select);
        managerSensor = (SensorManager) getApplicationContext().getSystemService(Context.SENSOR_SERVICE);
        textCharge = (TextView) findViewById(R.id.textvalueCharge);
   	 	textTemp = (TextView) findViewById(R.id.textvalueTemp);
        registerReceiver(mBatInfoReceiverMain,  new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
     
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        final int SUPERUSER_REQUEST = 2323; // arbitrary number of your choosing
        Intent intent = new Intent("android.intent.action.superuser"); // superuser request
        intent.putExtra("name", "durip_app"); // tell Superuser the name of the requesting app
        intent.putExtra("packagename", "it.durip_app"); // tel Superuser the name of the requesting package
        try{
            startActivityForResult(intent, SUPERUSER_REQUEST); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
        
        
        textVoltage = (TextView) findViewById(R.id.textvalueVoltage);
		textCPU0 = (TextView) findViewById(R.id.textvalueCPU0);
		textCPU1 = (TextView) findViewById(R.id.textvalueCPU1);
		textCPU2 = (TextView) findViewById(R.id.textvalueCPU2);
		textCPU3 = (TextView) findViewById(R.id.textvalueCPU3);
        
        
        pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		wl = pm.newWakeLock(
		            PowerManager.PARTIAL_WAKE_LOCK
		            | PowerManager.ON_AFTER_RELEASE,
		            "DuripApp");
		wl.acquire();
        updateMac(findViewById(R.layout.activity_main));
        
          
        Spinner spinner = (Spinner)findViewById(R.id.spinnerSensor);
        
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
        		this,
	        	android.R.layout.simple_spinner_item,
	        	new String[]{"Accelerometer","Gyroscope","Orientation","Accelerometer Fusion" , "Gravity", "Rotation",
        				"Light" ,"Battery"}
        		);
        ArrayAdapter<String> adapterMAC = new ArrayAdapter<String>(
        		this,
	        	android.R.layout.simple_spinner_item,
	        	new String[]{"MAC address"}
        		);
        macSpinner.setAdapter(adapterMAC);
        numberPickerSensor = (NumberPicker) findViewById(R.id.numberpickerSensor);
        numberPickerSensor.setMaxValue(10);       
        numberPickerSensor.setMinValue(1);      
//        numberPickerSensor.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
//            @Override
//            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
//                // do something here
////                Log.d("OPEN", "Number of Nights change value.");
//            	ts = newVal;
//            }
//
//        });
        
//        ts = numberPickerSensor.getValue();
        
        

        
        numberPickerOLSR = (NumberPicker) findViewById(R.id.numberPickerOlsr);
        numberPickerOLSR.setMaxValue(10);       
        numberPickerOLSR.setMinValue(1);         
        
//        numberPickerOLSR.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
//            @Override
//            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
//                // do something here
////                Log.d("OPEN", "Number of Nights change value.");
////            	tsOLSR = newVal;
//            }
//        });
        
//        tsOLSR = numberPickerSensor.getValue();

        spinner.setAdapter(adapter);
        
        spinner.setOnItemSelectedListener(new OnItemSelectedListener() {
        	public void onItemSelected(AdapterView<?> parent, View view,int pos, long id ) {
        		RadioGroup r1 = (RadioGroup) findViewById(R.id.radioGroup2);
        		RadioButton rb = (RadioButton) findViewById(R.id.radioPlot0);
        		int viewPlot = r1.getCheckedRadioButtonId();
        		System.out.println(viewPlot + "Value" + rb.getId() );
        		if(checkOndemandSpinner){
        			if(rb.getId() == viewPlot ){
        				if(pos < 7){
        					Intent intentSensor = new Intent(MainActivity.this ,  SensorCharts.class);
        					try{
        						intentSensor.putExtra("SensorName", pos+1);
        						startActivity(intentSensor); // make the request!
        					}catch(Exception e){
        						Log.i("sensor err", e.toString());
        					}
        				}else{
        					try{

        						Intent intentSensor = new Intent(MainActivity.this ,  BatteryCharts.class);	                		
        						intentSensor.putExtra("timesample",  numberPickerSensor.getValue());
        						startActivity(intentSensor); // make the request!
        					}catch(Exception e){
        						Log.i("sensor err", e.toString());
        					}
        				}
        			}else{        		
        				
        				if(ctrlPlotPage == 1){
        					//call stop register
        					stopManager();
        					aprHistoryPlot.clear();
        					
        				}
        				ctrlPlotPage =1;
        				if(pos < 7){        				
        					onPlot(pos+1);
        				}else{
        					plotBattery(pos+1);
        				}
//        				System.out.println("Enter");
        				
        			}
        		}else{
//        			adapter.setFocusable(true);
        				//        			spinner.setFocusable(true);
        				checkOndemandSpinner = true;
        		}
        		
        	}
        	public void onNothingSelected(AdapterView<?> arg0) {
        		
        	}
		});
        
        //NEW PART TO MANAGE MULTIPLE ROW RADIO BUTTONS
        
//        RadioGroup r1 = (RadioGroup) findViewById(R.id.radioGroup1);
        
        RadioButton r0 = (RadioButton) findViewById(R.id.radio0);
        r0.setOnClickListener(new OnClickListener(){
        

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				RadioButton rb0 = (RadioButton) findViewById(R.id.radio0);
				RadioButton rb1 = (RadioButton) findViewById(R.id.radio1);
				RadioButton rb2 = (RadioButton) findViewById(R.id.radio2);
				RadioButton rb3 = (RadioButton) findViewById(R.id.radio3);
				rb0.setChecked(true);
				rb1.setChecked(false);
				rb2.setChecked(false);
				rb3.setChecked(false);
			}
        	
        });
               
        arrangeRadioButtonGroupGrid();
    }
    
    
    private void arrangeRadioButtonGroupGrid(){
    	RadioButton r1 = (RadioButton) findViewById(R.id.radio1);
        r1.setOnClickListener(new OnClickListener(){
        

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				RadioButton rb0 = (RadioButton) findViewById(R.id.radio0);
				RadioButton rb1 = (RadioButton) findViewById(R.id.radio1);
				RadioButton rb2 = (RadioButton) findViewById(R.id.radio2);
				RadioButton rb3 = (RadioButton) findViewById(R.id.radio3);
				rb0.setChecked(false);
				rb1.setChecked(true);
				rb2.setChecked(false);
				rb3.setChecked(false);
			}
        	
        });

        
        RadioButton r2 = (RadioButton) findViewById(R.id.radio2);
        r2.setOnClickListener(new OnClickListener(){
        
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				RadioButton rb0 = (RadioButton) findViewById(R.id.radio0);
				RadioButton rb1 = (RadioButton) findViewById(R.id.radio1);
				RadioButton rb2 = (RadioButton) findViewById(R.id.radio2);
				RadioButton rb3 = (RadioButton) findViewById(R.id.radio3);
				rb0.setChecked(false);
				rb1.setChecked(false);
				rb2.setChecked(true);
				rb3.setChecked(false);
			}
        	
        });
        
        RadioButton r3 = (RadioButton) findViewById(R.id.radio3);
        r3.setOnClickListener(new OnClickListener(){
       
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				RadioButton rb0 = (RadioButton) findViewById(R.id.radio0);
				RadioButton rb1 = (RadioButton) findViewById(R.id.radio1);
				RadioButton rb2 = (RadioButton) findViewById(R.id.radio2);
				RadioButton rb3 = (RadioButton) findViewById(R.id.radio3);
				rb0.setChecked(false);
				rb1.setChecked(false);
				rb2.setChecked(false);
				rb3.setChecked(true);
			}
        	
        });
    }
    
    
    private void stopManager(){
    	managerSensor.unregisterListener(this);
    }
    
    @Override
    public void onDestroy(){
    	super.onDestroy();
    	closeIperf(this.findViewById(R.id.iperf));
    	((ToggleButton) this.findViewById(R.id.manageSensors)).setChecked(false);
    	manageSensors(((ToggleButton) this.findViewById(R.id.manageSensors)));
    	wl.release();
    	
    	unregisterReceiver(this.mBatInfoReceiverMain);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    public void updateMac(View view){
        Process p;
        Spinner macSpinner = (Spinner) findViewById(R.id.mac_select);
        String inputLine = null, phy = "phy1";
        ArrayList<String> stringArrayList = new ArrayList<String>();
        Runtime r = Runtime.getRuntime();
        BufferedReader ls;
        
        try {
			p = r.exec("ls /sys/kernel/debug/ieee80211/");
			ls = new BufferedReader(new InputStreamReader(p.getInputStream()));	
			while ((inputLine = ls.readLine()) != null) 
				phy = inputLine;
			ls.close();
			p.destroy();
			p = r.exec("ls /sys/kernel/debug/ieee80211/"+phy+"/netdev:wlan1/stations");
			ls = new BufferedReader(new InputStreamReader(p.getInputStream()));	
			while ((inputLine = ls.readLine()) != null) 
				stringArrayList.add(inputLine);
			ls.close();
			p.destroy();
			ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, stringArrayList);
			macSpinner.setAdapter(adapter);
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
    	
    }
    
    public void closeIperf(View view) {
        try{
        	stopService(new Intent(this, Iperf.class));
        }catch(Exception e){
    		Log.i("stop err", e.toString());
        }
    }
    
    public void updateStatus(View view) {
        Process p;
        EditText statsInfo = (EditText)findViewById(R.id.statusInfo);
        EditText infoWlan = (EditText)findViewById(R.id.paramMyDevice);
        String inputLine = null, wlan = infoWlan.getText().toString();
        StringBuilder infos = new StringBuilder();
        int countRow =0;
        String []token ;
        Runtime r = Runtime.getRuntime();
        BufferedReader ls;
        
        try {
			p = r.exec("iwconfig " + wlan);
			ls = new BufferedReader(new InputStreamReader(p.getInputStream()));	
			while ((inputLine = ls.readLine()) != null){
				
				switch(countRow){
					case 0:
						token = inputLine.split(" ");
						infos.append(token[8] + " ");
					break;
					case 1:
						token = inputLine.split(" ");
						infos.append(token[12] + " "+ "GHz" +"\n" + "Cell:" +token[16]+ "\n");
						break;
				}
				countRow++;

			}
			statsInfo.setText(infos);
			ls.close();
			p.destroy();
        }catch (IOException e) {
        	Log.i("stop err", e.toString());
        }
    }
    
    
    public void openIperf(View view) {
        EditText urlText = (EditText) findViewById(R.id.paramUrl);
        String url = urlText.getText().toString();
        EditText tText = (EditText) findViewById(R.id.paramT);
        int t = Integer.parseInt(tText.getText().toString());
        EditText iText = (EditText) findViewById(R.id.paramI);
        int i = Integer.parseInt(iText.getText().toString());
        Intent intentGraph = new Intent(this, Iperf.class);
        intentGraph.putExtra(Iperf.DESTINATION, url);
        intentGraph.putExtra(Iperf.LOOP, false);
        intentGraph.putExtra(Iperf.TIME, t+"");
        intentGraph.putExtra(Iperf.INTERVAL, i+"");
        try{
        	startService(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
    }
    
    public void openKiller(View view) {
        Intent intentGraph = new Intent(this, DeviceSwitch.class);
        try{
        	startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
    }
    
    public void openOlsrdSettings(View view) {
        Intent intentOlsrdSettings = new Intent(this, OlsrdSettings.class);
        try{
        	startActivity(intentOlsrdSettings); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
    }
    
	public void startReadMACParam(View v){
		
		EditText mEdit;
		String s[];
		s= new String[ServerSocketCmd.params];
		Log.d("MAC READ", "STAR READ MAC");		
		mEdit   = (EditText)findViewById(R.id.paramMyIp);
		s[0]= mEdit.getText().toString();
		Log.d("MAC READ", "IP " + s[0]);
		mEdit   = (EditText)findViewById(R.id.paramMyPort);
		s[1]= mEdit.getText().toString();
		Log.d("MAC READ", "PORT " + s[1]);
		mEdit   = (EditText)findViewById(R.id.paramMyDevice);
		s[2]= mEdit.getText().toString();
		Log.d("MAC READ", "Device " + s[2]);
		mEdit   = (EditText)findViewById(R.id.paramMyPhy);
		s[3]= mEdit.getText().toString();
		Log.d("MAC READ", "PHY " + s[3]);
		mEdit   = (EditText)findViewById(R.id.paramMySleep);
		s[4]= mEdit.getText().toString();
		Log.d("MAC READ", "PHY " + s[3]);
		mEdit   = (EditText)findViewById(R.id.paramMyEssid);
		s[6]= mEdit.getText().toString();
		Log.d("MAC READ", "ESSID " + s[6]);
		mEdit   = (EditText)findViewById(R.id.logPath);
		s[7]= mEdit.getText().toString();
		Log.d("MAC READ", "LOGFOLDER " + s[7]);
		mEdit   = (EditText)findViewById(R.id.paramSubnet);
		s[8]= mEdit.getText().toString();
		Log.d("MAC READ", "SUBNET " + s[8]);
		mEdit   = (EditText)findViewById(R.id.paramNetmask);
		s[9]= mEdit.getText().toString();
		Log.d("MAC READ", "NETMASK " + s[9]);
		if (((RadioButton) findViewById(R.id.radio0)).isChecked()){
			s[5]="0";
		}else if(((RadioButton) findViewById(R.id.radio1)).isChecked()){
			s[5]="1";
		}else if(((RadioButton) findViewById(R.id.radio2)).isChecked()){
			s[5]="2";
		}else if(((RadioButton) findViewById(R.id.radio3)).isChecked()){
			s[5]="3";
		}
	
//					
		
        Intent intentSocket = new Intent(this, ServerSocketCmd.class);
        intentSocket.putExtra(ServerSocketCmd.DESTINATION, s[0]);
        intentSocket.putExtra(ServerSocketCmd.PORT, s[1]);
        intentSocket.putExtra(ServerSocketCmd.DEVICE, s[2]);
        intentSocket.putExtra(ServerSocketCmd.PHY, s[3]);
        intentSocket.putExtra(ServerSocketCmd.INTERVAL, s[4]);
        intentSocket.putExtra(ServerSocketCmd.PRINT, s[5]);
        intentSocket.putExtra(ServerSocketCmd.ESSID, s[6]);
        intentSocket.putExtra(ServerSocketCmd.PATH, s[7]);
        intentSocket.putExtra(ServerSocketCmd.SUBNET, s[8]);
        intentSocket.putExtra(ServerSocketCmd.NETMASK, s[9]);
        
        if(ctrlFirstSetup == 1){
        	//if (IPAddressUtil.isIPv4LiteralAddress(s[0]))
            try{
            	//ServerSocketCmd.java
            	Log.d("MAC READ", "ENTRO QUA?? ");
            	startService(intentSocket); // make the request!
            	ctrlFirstSetup = 0;
            }catch(Exception e){
        		Log.i("superusering err", e.toString());
            }	
        
        }else{
        	
//            intentSocket.putExtra(ServerSocketCmd.DEVICE, s[2]);
//            intentSocket.putExtra(ServerSocketCmd.PHY, s[3]);            
//            intentSocket.putExtra(ServerSocketCmd.ESSID, s[6]);
            
        	Log.d("MAC READ", "Already run");
        }
        
        
	}
	
	
    public void manageIperf(View view) {
		boolean on = ((ToggleButton) view).isChecked();

		if (on){
	        try{
	            Intent intentIperf = new Intent(this, Iperf.class);
	            EditText urlText = (EditText) findViewById(R.id.paramUrl);
	            String url = urlText.getText().toString();
	            EditText tText = (EditText) findViewById(R.id.paramT);
	            int t = Integer.parseInt(tText.getText().toString());
	            EditText iText = (EditText) findViewById(R.id.paramI);
	            int i = Integer.parseInt(iText.getText().toString());
	            intentIperf.putExtra(Iperf.DESTINATION, url);
	            intentIperf.putExtra(Iperf.LOOP, false);
	            intentIperf.putExtra(Iperf.TIME, t+"");
	            intentIperf.putExtra(Iperf.INTERVAL, i+"");
	        	startService(intentIperf); // make the request!
	        }catch(Exception e){
	    		Log.i("IPERF err", e.toString());
	        }
		}else{
	        try{
	        	stopService(new Intent(this, Iperf.class)); // make the request!
	        }catch(Exception e){
	    		Log.i("IPERF err", e.toString());
	        }
		}
    }
	
    
    public void manageSensors(View view) {
		boolean on = ((ToggleButton) view).isChecked();

		if (on){
	        try{
	            Intent intentSensors = new Intent(this, Sensors.class);
	        	startService(intentSensors); // make the request!
	        }catch(Exception e){
	    		Log.i("SENSORS err", e.toString());
	        }
		}else{
	        try{
	        	stopService(new Intent(this, Sensors.class)); // make the request!
	        }catch(Exception e){
	    		Log.i("MATTEO sensors err", e.toString());
	        }
		}
    }
    
    public void manageIperfServer(View view) {
		boolean on = ((ToggleButton) view).isChecked();

		if (on){
	        try{
	            Intent intentIperServer = new Intent(this, IperfServer.class);
	        	startService(intentIperServer); // make the request!
	        }catch(Exception e){
	    		Log.i("iperfserver err", e.toString());
	        }
		}else{
	        try{
	        	stopService(new Intent(this, IperfServer.class)); // make the request!
	        }catch(Exception e){
	    		Log.i("iperf server err", e.toString());
	        }
		}
    }
    
    public void manageOlsrd(View view) {
		boolean on = ((ToggleButton) view).isChecked();

		if (on){
			Log.i("OLSRD launcher","Button trying to turn it on");
	        try{
	            Intent intentOlsrd = new Intent(this, Olsr.class);
	        	startService(intentOlsrd); // make the request!
	        }catch(Exception e){
	    		Log.i("OLSRD err", e.toString());
	        }
		}else{
			Log.i("OLSRD launcher","Button trying to switch off olsr");
	        try{
	        	stopService(new Intent(this, Olsr.class)); // make the request!
	        }catch(Exception e){
	    		Log.i("OLSRD err", e.toString());
	        }
		}
    }
    
    
    public void manageOlsrdLog(View view) {
		boolean on = ((ToggleButton) view).isChecked();

		if (on){
			Log.i("OLSRD log launcher","Button trying to turn it on");
	        try{
	        	EditText OLSRfileLog = (EditText) findViewById(R.id.paramOLSRfileLog);
	        	NumberPicker OLSRSampletimelog = (NumberPicker) findViewById(R.id.numberPickerOlsr);
	        	Intent intentOlsrdLog = new Intent(this, olsrdLog.class);	        											
	        	String filename = OLSRfileLog.getText().toString();
	        	String sampletime = Integer.toString(OLSRSampletimelog.getValue());
	        	
	            intentOlsrdLog.putExtra(olsrdLog.FILENAME, filename);	            
	            intentOlsrdLog.putExtra(olsrdLog.TIME, sampletime);
	            
	        	startService(intentOlsrdLog); // make the request!
	        }catch(Exception e){
	    		Log.i("OLSRD Log err", e.toString());
	        }
		}else{
			Log.i("OLSRD log launcher","Button trying to switch off olsr");
	        try{
	        	stopService(new Intent(this, olsrdLog.class)); // make the request!
	        }catch(Exception e){
	    		Log.i("OLSRD Log err", e.toString());
	        }
		}
    }
    
    
    
    
    
    public void openMAC(View view) {

        EditText editText = (EditText) findViewById(R.id.statusInfo);
        String message = editText.getText().toString();
        EditText urlText = (EditText) findViewById(R.id.paramUrl);
        String url = urlText.getText().toString();
        EditText tText = (EditText) findViewById(R.id.paramT);
        int t = Integer.parseInt(tText.getText().toString());
        EditText iText = (EditText) findViewById(R.id.paramI);
        int i = Integer.parseInt(iText.getText().toString());
        //EditText portText = (EditText) findViewById(R.id.paramPort);
        //int port = Integer.parseInt(portText.getText().toString());
        //EditText sleepText = (EditText) findViewById(R.id.paramSleep);
        //int sleep = Integer.parseInt(sleepText.getText().toString());
        Spinner macSpinner = (Spinner) findViewById(R.id.mac_select);
        String macaddress = macSpinner.getSelectedItem().toString();//TODO check if isset
//        boolean verbose = ((RadioButton) findViewById(R.id.notverbose)).isChecked();
        //String verbose = editText.getText().toString();
        //intent.putExtra(EXTRA_MESSAGE, message);
        Intent intentGraph = new Intent(this, MACActivity.class);
        intentGraph.putExtra(SHELL_RESULTS, message);
        intentGraph.putExtra(PARAM_URL, url);
        intentGraph.putExtra(PARAM_MAC, macaddress);
        intentGraph.putExtra(PARAM_T, t);
        intentGraph.putExtra(PARAM_I, i);
        //intentGraph.putExtra(PARAM_PORT, port);
        //intentGraph.putExtra(PARAM_SLEEP, sleep);
//        intentGraph.putExtra(PARAM_VERBOSE, verbose ? 0 : 1);
        
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
        //startActivity(intent);
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("graph err", e.toString());
        }
    }
    
    public void openTCP(View view) {

        EditText editText = (EditText) findViewById(R.id.statusInfo);
        String message = editText.getText().toString();
        EditText urlText = (EditText) findViewById(R.id.paramUrl);
        String url = urlText.getText().toString();
        EditText tText = (EditText) findViewById(R.id.paramT);
        int t = Integer.parseInt(tText.getText().toString());
        EditText iText = (EditText) findViewById(R.id.paramI);
        int i = Integer.parseInt(iText.getText().toString());
        //EditText portText = (EditText) findViewById(R.id.paramPort);
        //int port = Integer.parseInt(portText.getText().toString());
        //EditText sleepText = (EditText) findViewById(R.id.paramSleep);
        //int sleep = Integer.parseInt(sleepText.getText().toString());
//        boolean verbose = ((RadioButton) findViewById(R.id.verbose)).isChecked();
        //String verbose = editText.getText().toString();
        //intent.putExtra(EXTRA_MESSAGE, message);
        Intent intentGraph = new Intent(this, TCPActivity.class);
        intentGraph.putExtra(SHELL_RESULTS, message);
        intentGraph.putExtra(PARAM_URL, url);
        intentGraph.putExtra(PARAM_T, t);
        intentGraph.putExtra(PARAM_I, i);
        //intentGraph.putExtra(PARAM_PORT, port);
        //intentGraph.putExtra(PARAM_SLEEP, sleep);
//        intentGraph.putExtra(PARAM_VERBOSE, verbose ? 0 : 1);
        
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
        //startActivity(intent);
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("graph err", e.toString());
        }
    }
    
    
    
    public void openGraph(View view) {

//        EditText editText = (EditText) findViewById(R.id.pingresults);
//        String message = editText.getText().toString();
        EditText urlText = (EditText) findViewById(R.id.paramUrl);
        String url = urlText.getText().toString();
        EditText tText = (EditText) findViewById(R.id.paramT);
        String t = tText.getText().toString();
        EditText iText = (EditText) findViewById(R.id.paramI);
        String i = iText.getText().toString();
        //EditText portText = (EditText) findViewById(R.id.paramPort);
        //String port = portText.getText().toString();
//        boolean verbose = ((RadioButton) findViewById(R.id.verbose)).isChecked();
        //String verbose = editText.getText().toString();
        //intent.putExtra(EXTRA_MESSAGE, message);
        Intent intentGraph = new Intent(this, GraphActivity.class);
//        intentGraph.putExtra(SHELL_RESULTS, message);
        intentGraph.putExtra(PARAM_URL, url);
        intentGraph.putExtra(PARAM_T, t);
        intentGraph.putExtra(PARAM_I, i);
        //intentGraph.putExtra(PARAM_PORT, port);
//        intentGraph.putExtra(PARAM_VERBOSE, verbose ? 1 : 0);
        
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
        //startActivity(intent);
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("graph err", e.toString());
        }
    }
    
    
    public void openFlow(View view) {
        Intent intentGraph = new Intent(this, DataFlow.class);
        try{
            startActivity(intentGraph); // make the request!
        }catch(Exception e){
    		Log.i("graph err", e.toString());
        }
    }
    public void openSensor(View view) {
        Intent intentSensor = new Intent(this, SensorPlot.class);
        try{
            startActivity(intentSensor); // make the request!
        }catch(Exception e){
    		Log.i("sensor err", e.toString());
        }
    }
    public void openExperimentMaster(View view) {
        Intent ExperimentMasterActivityLauncher = new Intent(this, ExperimentMasterActivity.class);
        EditText portText = (EditText) findViewById(R.id.paramMyPort);
        String port = portText.getText().toString();
        //EditText portText = (EditText) findViewById(R.id.paramPort);
        //String port = portText.getText().toString();
//        boolean verbose = ((RadioButton) findViewById(R.id.verbose)).isChecked();
        //String verbose = editText.getText().toString();
        //intent.putExtra(EXTRA_MESSAGE, message);
        ExperimentMasterActivityLauncher.putExtra(PARAM_PORT, port);
        
        try{
            startActivity(ExperimentMasterActivityLauncher); // make the request!
        }catch(Exception e){
    		Log.i("ExperimentMasterActivity err", e.toString());
        }
    }
    
    
    
    
  //PARTE DEL SENSORE
    
	protected void onPlot(int nr) {
		
		
//		setContentView(R.layout.activity_sensor_charts);
//		Intent myIntent = getIntent();
//		nr = myIntent.getIntExtra( "SensorName" ,1 );
		
//		System.out.println("NR value" + nr);
		
		            
        switch(nr){
            case 1:            
	            labels = new String[] {"X","Y","Z","Accelerometer" , "m/s^2"};	           
	            boundRange[0] =-20;
	            boundRange[1] =20;
	            break;

            case 2:       
	            labels = new String[] {"X-axis","Y-axis","Z-axis","Gyroscope" , "rad/sec" };
	            boundRange[0] =-15;
	            boundRange[1] =15;
	            break;
            case 3:
	            labels = new String[] {"Azimuth","Pitch","Roll","Orientation" , "Deg"};
	            break;
	            //A three dimensional vector indicating acceleration along each device axis, 
//	            not including gravity. All values have units of m/s^2
            case 4:
	            labels = new String[] {"X","Y","Z","Linear Acceleration" ,"m/s^2"};
	            boundRange[0] =-20;
	            boundRange[1] =20;
	            break;
            case 5:
	            labels = new String[] {"Z+X+Y/rad+Z/rad","Y+Z/rad+X/rad","Null->orientation","Gravity" , "m/s^2"};
	            boundRange[0] =-20;
	            boundRange[1] =20;
	            break;
            case 6:
	            labels = new String[] {"Y.Z tgGround East","tgGround North","PerpGround","Rotation Vector" , "TBD"};
	            boundRange[0] =-20;
	            boundRange[1] =20;
	            break;
            case 7:
	            labels = new String[] {"Light","None","None","Light" , "Si"};
	            boundRange[0] =0;
	            boundRange[1] =3000;
	            break;
	            
        }


        	aprHistoryPlot = (XYPlot)findViewById(R.id.timeserieChart2);
        	
        	
        	azimuthHistorySeries = new SimpleXYSeries(labels[0]);
        	azimuthHistorySeries.useImplicitXVals();
        	pitchHistorySeries = new SimpleXYSeries(labels[1]);
        	pitchHistorySeries.useImplicitXVals();
        	rollHistorySeries = new SimpleXYSeries(labels[2]);
        	rollHistorySeries.useImplicitXVals();

        	        	
        	if (nr < 7){
        		aprHistoryPlot.setRangeBoundaries(boundRange[0], boundRange[1], BoundaryMode.FIXED);
        		aprHistoryPlot.addSeries(azimuthHistorySeries, new LineAndPointFormatter(Color.rgb(100, 100, 200), Color.BLUE, null));
        		aprHistoryPlot.addSeries(pitchHistorySeries, new LineAndPointFormatter(Color.rgb(100, 200, 100), Color.BLACK, null));
        		aprHistoryPlot.addSeries(rollHistorySeries, new LineAndPointFormatter(Color.rgb(200, 100, 100), Color.RED, null ));
        	}else{
        		aprHistoryPlot.setRangeBoundaries(boundRange[0], boundRange[1], BoundaryMode.AUTO);
        		aprHistoryPlot.addSeries(azimuthHistorySeries, new LineAndPointFormatter(Color.rgb(100, 100, 200), Color.BLUE, null));
        	}
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


        	registerSensor(nr);
	}
    
    
    
    private void registerSensor(int nr){
        // register for orientation sensor events:
//        sensorMgr = managerSensor;
        System.out.println("REGISTER SENSOR");
        plotNumber = nr;
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
    

    
    // Called whenever a new orSensor reading is taken.
    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent) {

        // update instantaneous data:
        if(plotNumber < 7){
        		if (rollHistorySeries.size() > HISTORY_SIZE) {
        			rollHistorySeries.removeFirst();
        			pitchHistorySeries.removeFirst();
        			azimuthHistorySeries.removeFirst();
        		}

        		// add the latest history sample:
        		azimuthHistorySeries.addLast(null, sensorEvent.values[0]);
        		pitchHistorySeries.addLast(null, sensorEvent.values[1]);
        		rollHistorySeries.addLast(null, sensorEvent.values[2]);
        		
        		
        		textVoltage.setText("X" + sensorEvent.values[0]);
				textCPU0.setText("Y" + sensorEvent.values[1]);
				textCPU1.setText("Z" + sensorEvent.values[2]);

        		aprHistoryPlot.redraw();
//        	}
    	}else{
        		if (azimuthHistorySeries.size() > HISTORY_SIZE) {
//        			rollHistorySeries.removeFirst();
//        			pitchHistorySeries.removeFirst();
        			azimuthHistorySeries.removeFirst();
        		}
        		textVoltage.setText("Light" + sensorEvent.values[0]);
        		// add the latest history sample:
//        		System.out.println(sensorEvent.values[0]);
        		azimuthHistorySeries.addLast(null, sensorEvent.values[0]);
//        		pitchHistorySeries.addLast(null, 0);
//        		rollHistorySeries.addLast(null, 0);
        		aprHistoryPlot.redraw();   
//        	}
    	}
        
    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {
        // Not interested in this event
    }
//BATTERY PART
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
    

private void plotBattery(int pos) {


	plotNumber =pos;
	aprHistoryPlot = (XYPlot)findViewById(R.id.timeserieChart2);

	xBattery = new SimpleXYSeries("Battery");
	xBattery.useImplicitXVals();


	aprHistoryPlot.setRangeBoundaries(-1000, 200, BoundaryMode.FIXED);
	aprHistoryPlot.setDomainBoundaries(0, 300, BoundaryMode.AUTO);
	PointLabelFormatter point1 = new  PointLabelFormatter(Color.rgb(255, 255, 255)); 
	aprHistoryPlot.addSeries(xBattery, new LineAndPointFormatter(Color.rgb(100, 100, 200), Color.BLUE, null , point1));

	aprHistoryPlot.setDomainStep(XYStepMode.INCREMENT_BY_VAL ,5);
	aprHistoryPlot.setTicksPerRangeLabel(3);
	                
	aprHistoryPlot.getDomainLabelWidget().pack();
	aprHistoryPlot.getRangeLabelWidget().pack();	               

	aprHistoryPlot.setTitle("Current Consumption");
	aprHistoryPlot.setDomainLabel("[S]");
	aprHistoryPlot.setRangeLabel("mA");


	r = new runnableChart();
	new Thread(r).start();

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


		FileInputStream fs = new FileInputStream(f);	      
		DataInputStream ds = new DataInputStream(fs);

		text = ds.readLine();

		ds.close();    
		fs.close();  

//		Long value = null;

//		System.out.println(text);
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


private class runnableChart implements Runnable {

	private boolean doRun = true;
		 Handler handler = new Handler();
	@Override
	public void run(){
				
		while(doRun){		            	
			try {
				//				System.out.println("Value " + getValueFromFile());

				if (xBattery.size() > HISTORY_SIZE) 
				{
					xBattery.removeFirst();
				}
				// add the latest history sample:
				xBattery.addLast(null, getValueFromFile());
				//        		System.out.println(getValueMHzCPUFile(0));
				//        		System.out.println(getValueMHzCPUFile(1));
				//        		System.out.println(getValueMHzCPUFile(2));
				//        		System.out.println(getValueMHzCPUFile(3));

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
				Thread.sleep(1*1000);
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


public void stopChart(View view) {
	if (plotNumber <= 7){
		cleanup();
//		aprHistoryPlot.clear();
		
	}else{
		
//		aprHistoryPlot.clear();
		r.stopThread();
		this.unregisterReceiver(this.mBatInfoReceiverMain);
	}

}


}

