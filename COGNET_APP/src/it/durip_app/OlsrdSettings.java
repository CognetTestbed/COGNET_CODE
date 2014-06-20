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
//import java.io.File;
import java.io.FileInputStream;
//import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
//import java.nio.charset.Charset;

//import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.os.Bundle;
//import android.provider.MediaStore.Files;
import android.app.Activity;
//import android.content.Intent;
//import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
//import android.view.View;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Toast;
import android.support.v4.app.NavUtils;

public class OlsrdSettings extends Activity {
	String path = "/data/local/etc/olsrd.conf";
	private SeekBar hellointerval = null;
	private SeekBar tcinterval = null;
	private SeekBar hellovalidity = null;
	private SeekBar tcvalidity = null;
	public int HelloIntervalSet = 0;
	public int TCIntervalSet = 0;
	public int HelloValiditySet = 0;
	public int TCValiditySet = 0;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_olsrd_settings);
		
		readConfig(path);
        hellointerval = (SeekBar) findViewById(R.id.HelloInterval);
        hellointerval.setProgress(HelloIntervalSet); 
        //hellointerval.incrementProgressBy(5);
        hellointerval.setMax(30);
        hellointerval.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
 
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
                progressChanged = progress;
            }
 
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
            }
 
            public void onStopTrackingTouch(SeekBar seekBar) {
            	HelloIntervalSet = progressChanged;
            	commander("echo \"hellointerval "+progressChanged+"00\" | telnet localhost 50023");
                Toast.makeText(OlsrdSettings.this,"Set Hello Interval to:"+progressChanged,
                        Toast.LENGTH_SHORT).show();
            }
        });
        
        hellovalidity = (SeekBar) findViewById(R.id.HelloV);
        hellovalidity.setProgress(HelloValiditySet); 
        //hellointerval.incrementProgressBy(5);
        hellovalidity.setMax(100);
        hellovalidity.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
 
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
                progressChanged = progress;
            }
 
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
            }
 
            public void onStopTrackingTouch(SeekBar seekBar) {
            	HelloValiditySet = progressChanged;
            	commander("echo \"hellovalidity "+progressChanged+"00\" | telnet localhost 50023");
                Toast.makeText(OlsrdSettings.this,"Set Hello Validity to:"+progressChanged,
                        Toast.LENGTH_SHORT).show();
            }
        });
        
        tcinterval = (SeekBar) findViewById(R.id.TCI);
        tcinterval.setProgress(TCIntervalSet); 
        //hellointerval.incrementProgressBy(5);
        tcinterval.setMax(50);
        tcinterval.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
 
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
                progressChanged = progress;
            }
 
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
            }
 
            public void onStopTrackingTouch(SeekBar seekBar) {
            	TCIntervalSet = progressChanged;
            	commander("echo \"tcinterval "+progressChanged+"00\" | telnet localhost 50023");
                Toast.makeText(OlsrdSettings.this,"Set TC Interval to:"+progressChanged,
                        Toast.LENGTH_SHORT).show();
            }
        });
        
        tcvalidity = (SeekBar) findViewById(R.id.TCV);
        tcvalidity.setProgress(TCValiditySet); 
        //hellointerval.incrementProgressBy(5);
        tcvalidity.setMax(200);
        tcvalidity.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
 
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
                progressChanged = progress;
            }
 
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
            }
 
            public void onStopTrackingTouch(SeekBar seekBar) {
            	TCValiditySet = progressChanged;
            	commander("echo \"tcvalidity "+progressChanged+"00\" | telnet localhost 50023");
                Toast.makeText(OlsrdSettings.this,"Set Hello Interval to:"+progressChanged,
                        Toast.LENGTH_SHORT).show();
            }
        });
		// Show the Up button in the action bar.
		setupActionBar();
	}
	
	private void commander(String command){
//        Process p = null;
		Process p = null;
        Runtime r = Runtime.getRuntime();
        try {
			p = r.exec(command);
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}	
	}
	private String commanderReturn(String command){
        Process p = null;
        Runtime r = Runtime.getRuntime();
        try {
			p = r.exec(command);
	        BufferedReader ls;
			ls = new BufferedReader(new InputStreamReader(p.getInputStream()));	
			String inputLine;
			if ((inputLine = ls.readLine()) != null) 
					return inputLine;
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
		return "";	
	}

	/**
	 * Set up the {@link android.app.ActionBar}.
	 */
	private void setupActionBar() {

		getActionBar().setDisplayHomeAsUpEnabled(true);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.olsrd_settings, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			// This ID represents the Home or Up button. In the case of this
			// activity, the Up button is shown. Use NavUtils to allow users
			// to navigate up one level in the application structure. For
			// more details, see the Navigation pattern on Android Design:
			//
			// http://developer.android.com/design/patterns/navigation.html#up-vs-back
			//
			NavUtils.navigateUpFromSameTask(this);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	/*
	public void saveConfig(View view){
	    Pattern helloIntervalPattern = Pattern.compile("^.*(HelloInterval|TcInterval|HelloValidityTime|TcValidityTime)\\s+([\\d\\.]+).*$");
	    Matcher matcher = null;
	    // Check all occurrences
        try {
            // Open the file that is the first
            // command line parameter
            FileInputStream fstream = new FileInputStream(path);
            BufferedReader br = new BufferedReader(new InputStreamReader(fstream));
            String line;
            StringBuilder fileContent = new StringBuilder();
            //Read File Line By Line
            String tmp = null;
            FileWriter writer1 = new FileWriter(path+".new");
            while ((line = br.readLine()) != null) {
            	matcher=helloIntervalPattern.matcher(line.trim());
	            if(matcher.find()){
	            	//put line with correct value
	            	int value = 0;
	            	tmp = matcher.group(1).trim();
	            	if(tmp.equals("HelloInterval")){
	            		value = HelloIntervalSet;
	            	}else 
	            		if(tmp.equals("TcInterval")){
	            			value = TCIntervalSet;
	            		}else
		            		if(tmp.equals("HelloValidityTime")){
		            			value = HelloValiditySet;
		            		}else{
		            			value = TCValiditySet;
		            		}
	            	line = "  "+ tmp +"  "+value+".0";
	            	//put line with: tmp +" "+ value+".0"  
	            	//HelloIntervalSet = (int) Double.parseDouble( tmp );
	            }
	            writer1.write(line + System.getProperty("line.separator"));
	        }
            writer1.flush();
            writer1.close();
            br.close();
            fstream.close();
            
            File oldFile = new File(path);
            File trashFile = new File(path+".trash");
            oldFile.renameTo(trashFile);
            oldFile.delete();
            trashFile.delete();
            
            File newFile = new File(path+".new");
            File targetFile = new File(path);
            newFile.renameTo(targetFile);
            
            
            
	    }catch(Exception ex){
	        //ex.printStackTrace();
	        System.out.println("Exception...");
	    }
        
		Log.i("OLSRD killer","trying to kill olsrd");
        try{
        	stopService(new Intent(this, Olsr.class)); // make the request!
        }catch(Exception e){
    		Log.i("OLSRD err", e.toString());
        }
		Log.i("OLSRD restart","trying to restart olsrd");
        try{
            Intent intentOlsrd = new Intent(this, Olsr.class);
        	startService(intentOlsrd); // make the request!
        }catch(Exception e){
    		Log.i("OLSRD err", e.toString());
        }

	}
	*/
	public String olsrdConfiguration(String command){
		return commanderReturn("echo \""+command+"\" | telnet localhost 50023");
	}
	
	private boolean readConfig(String path){ 
	    Pattern helloIntervalPattern = Pattern.compile("^.*HelloInterval\\s+([\\d\\.]+).*$");
	    Pattern tcIntervalPattern = Pattern.compile("^.*TcInterval\\s+([\\d\\.]+).*$");
	    Pattern helloValidityPattern = Pattern.compile("^.*HelloValidityTime\\s+([\\d\\.]+).*$");
	    Pattern tcValidityPattern = Pattern.compile("^.*TcValidityTime\\s+([\\d\\.]+).*$");
	    Matcher matcher = null;
	    // Check all occurrences
        try {
            // Open the file that is the first
            // command line parameter
            FileInputStream fstream = new FileInputStream(path);
            BufferedReader br = new BufferedReader(new InputStreamReader(fstream));
            String line;
//            StringBuilder fileContent = new StringBuilder();
            //Read File Line By Line
            String tmp = null;
            while ((line = br.readLine()) != null) {
            	matcher=helloIntervalPattern.matcher(line);
	            if(matcher.find()){
	            	tmp = matcher.group(1);
	            	HelloIntervalSet = (int) Double.parseDouble( tmp );
	            }
            	matcher=helloValidityPattern.matcher(line);
	            if(matcher.find()){
	            	tmp = matcher.group(1);
	            	HelloValiditySet = (int) Double.parseDouble( tmp);
	            }
            	matcher=tcIntervalPattern.matcher(line);
	            if(matcher.find()){
	            	tmp = matcher.group(1);
	            	TCIntervalSet = (int) Double.parseDouble( tmp);
	            }
            	matcher=tcValidityPattern.matcher(line);
	            if(matcher.find()){
	            	tmp = matcher.group(1);
	            	TCValiditySet = (int) Double.parseDouble( tmp);
	            }
	        }
            br.close();
            fstream.close();
	    }catch(Exception ex){
	        //ex.printStackTrace();
	        System.out.println("Exception...");
	    }
        return true;
		
	}

}
