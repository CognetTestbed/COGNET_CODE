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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
//import java.util.Observable;
//import java.util.Observer;
//import java.util.regex.Matcher;
//import java.util.regex.Pattern;

//import com.androidplot.Plot;







import android.os.Bundle;
import android.os.PowerManager;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
//import android.widget.Button;
import android.widget.EditText;
import android.widget.NumberPicker;
import android.widget.RadioButton;
import android.widget.Spinner;
//import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.AdapterView.OnItemSelectedListener;
public class MainActivity extends Activity {

	
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
	private int ts;
	private int tsOLSR;
//	private void ping(String url) {
//
//		//String result="";
//	    try {
//	        String inputLine, pingCmd = "ping -c 5 -l 3 " + url;
//	        //Choose runtime or process builder, they are exactly the same!!
//	        
//	        Runtime r = Runtime.getRuntime();
//	        Process p = r.exec(pingCmd);
//	        
//	        /*
//			Process p = new ProcessBuilder()
//		    .command("ping", "android.com")
//		    .start();
//			*/
//	        BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
//	        
//	        TextView t=(TextView)findViewById(R.id.statusInfo);
//	        t.setText("Pinging....\n");
//	        
//	        // Create a Pattern object
//	        Pattern dataPattern = Pattern.compile("^.*icmp_seq=(\\d+) ttl=(\\d+) time=(\\d+\\.?\\d?\\d?) ms.*$");
//	        while ((inputLine = in.readLine()) != null) {
//		        System.out.println(inputLine);
//		        Matcher datas = dataPattern.matcher(inputLine);
//		        if (datas.find( )) {
//		        	t.append("\nicmp_seq: " + datas.group(1) + 
//		        			" ttl: " + datas.group(2) + 
//		        			" time: " + datas.group(3));
//		         } else {
//		            //t.append("\nBad ping");
//		        	 System.out.println("NO MATCH");
//		         }
//		        t.invalidate();
//		        //result += inputLine;
//
//		        //t.setText(result);
//	        }
//            t.append("\nPing done!");
//        	in.close();
//        }catch (IOException e) {
//    		Log.i("read response", e.toString());
//        }
//	    //return result;
//	}
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
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
        
        numberPickerSensor = (NumberPicker) findViewById(R.id.numberpickerSensor);
        numberPickerSensor.setMaxValue(10);       
        numberPickerSensor.setMinValue(1);      
        numberPickerSensor.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                // do something here
//                Log.d("OPEN", "Number of Nights change value.");
            	ts = newVal;
            }
        });
        
        ts = numberPickerSensor.getValue();
        
        

        
        numberPickerOLSR = (NumberPicker) findViewById(R.id.numberPickerOlsr);
        numberPickerOLSR.setMaxValue(10);       
        numberPickerOLSR.setMinValue(1);         
        
        numberPickerOLSR.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                // do something here
//                Log.d("OPEN", "Number of Nights change value.");
            	tsOLSR = newVal;
            }
        });
        
        tsOLSR = numberPickerSensor.getValue();
        
        
        
        
        spinner.setAdapter(adapter);
        
        spinner.setOnItemSelectedListener(new OnItemSelectedListener() {
        	public void onItemSelected(AdapterView<?> parent, View view,int pos, long id ) {
        		if(checkOndemandSpinner){
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
	                		
//	                		Log.i("OPEN" , +ts +"A");
	                		Intent intentSensor = new Intent(MainActivity.this ,  BatteryCharts.class);	                		
	                		intentSensor.putExtra("timesample",  ts);
	                		startActivity(intentSensor); // make the request!
	                	}catch(Exception e){
	                		Log.i("sensor err", e.toString());
	                	}
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
    }
    
    @Override
    public void onDestroy(){
    	super.onDestroy();
    	closeIperf(this.findViewById(R.id.iperf));
    	((ToggleButton) this.findViewById(R.id.manageSensors)).setChecked(false);
    	manageSensors(((ToggleButton) this.findViewById(R.id.manageSensors)));
    	wl.release();
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
			p = r.exec("ls sys/kernel/debug/ieee80211/");
			ls = new BufferedReader(new InputStreamReader(p.getInputStream()));	
			while ((inputLine = ls.readLine()) != null) 
				phy = inputLine;
			ls.close();
			p.destroy();
			p = r.exec("ls sys/kernel/debug/ieee80211/"+phy+"/netdev:wlan1/stations");
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
        String inputLine = null, wlan = "wlan1";
        StringBuilder infos = new StringBuilder();
        
        Runtime r = Runtime.getRuntime();
        BufferedReader ls;
        
        try {
			p = r.exec("iwconfig " + wlan);
			ls = new BufferedReader(new InputStreamReader(p.getInputStream()));	
			while ((inputLine = ls.readLine()) != null) 
				infos.append(inputLine+ "\n");
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
	
		Log.d("MAC READ", "Launch Server MAC READ");			
		
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
        //if (IPAddressUtil.isIPv4LiteralAddress(s[0]))
        try{
        	//ServerSocketCmd.java
        	startService(intentSocket); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
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
	        	EditText OLSRSampletimelog = (EditText) findViewById(R.id.numberPickerOlsr);
	        	Intent intentOlsrdLog = new Intent(this, olsrdLog.class);	        											
	        	String filename = OLSRfileLog.getText().toString();
//	        	String sampletime = OLSRSampletimelog.getText().toString();
	        	
	            intentOlsrdLog.putExtra(olsrdLog.FILENAME, filename);	            
	            intentOlsrdLog.putExtra(olsrdLog.TIME, tsOLSR);
	            
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
        Log.i("ExperimentMasterActivity", "boh");
        try{
            startActivity(ExperimentMasterActivityLauncher); // make the request!
        }catch(Exception e){
    		Log.i("ExperimentMasterActivity err", e.toString());
        }
    }
    
}

