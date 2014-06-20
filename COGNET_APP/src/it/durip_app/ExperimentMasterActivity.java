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
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;

//import android.widget.ExpandableListView;
//import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.NumberPicker;
//import android.widget.NumberPicker.OnValueChangeListener;
import it.durip_app.util.SystemUiHider;

import android.annotation.TargetApi;
import android.app.Activity;
//import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.MenuItem;
import android.support.v4.app.NavUtils;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 * 
 * @see SystemUiHider
 */
public class ExperimentMasterActivity extends Activity {
	
	static {
		System.loadLibrary("nl-3"); // "libmyjni.so" in Unixes
		System.loadLibrary("nl-genl-3"); // "libmyjni.so" in Unixes
		System.loadLibrary("ExperimentManager"); //
    }
	public native int startExperiment(int n, String []s);
	public native int stopExperiment(int n, String []s);
	private static String[] s;
    ArrayList<ManagedIp> managedIps = new ArrayList<ManagedIp>();
    private static final String fileIPExperiment = Environment.getExternalStorageDirectory().getPath()+"/COGNET_TESTBED/CONFIG/fileIP";
    private static final String fileIPExperimentStop= Environment.getExternalStorageDirectory().getPath()+"/COGNET_TESTBED/CONFIG/fileIPKill";
    IpListAdapter boxAdapter;

    
	
	/**
	 * Whether or not the system UI should be auto-hidden after
	 * {@link #AUTO_HIDE_DELAY_MILLIS} milliseconds.
	 */
	private static final boolean AUTO_HIDE = false;

	/**
	 * If {@link #AUTO_HIDE} is set, the number of milliseconds to wait after
	 * user interaction before hiding the system UI.
	 */
	private static final int AUTO_HIDE_DELAY_MILLIS = 3000;

	/**
	 * If set, will toggle the system UI visibility upon interaction. Otherwise,
	 * will show the system UI visibility upon interaction.
	 */
	private static final boolean TOGGLE_ON_CLICK = false;

	/**
	 * The flags to pass to {@link SystemUiHider#getInstance}.
	 */
	private static final int HIDER_FLAGS = SystemUiHider.FLAG_HIDE_NAVIGATION;

	/**
	 * The instance of the {@link SystemUiHider} for this activity.
	 */
	private SystemUiHider mSystemUiHider;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		s = new String[3];
		
		setContentView(R.layout.activity_experiment_master);
		setupActionBar();
        boxAdapter = new IpListAdapter(this, managedIps);

        ListView lvMain = (ListView) findViewById(R.id.IpListToManage);
        lvMain.setAdapter(boxAdapter);

//		final View controlsView = findViewById(R.id.fullscreen_content_controls);
//		final View contentView = findViewById(R.id.fullscreen_content);

		NumberPicker ip1 = (NumberPicker) findViewById(R.id.ip1);
		NumberPicker ip2 = (NumberPicker) findViewById(R.id.ip2);
		NumberPicker ip3 = (NumberPicker) findViewById(R.id.ip3);
		NumberPicker ip4 = (NumberPicker) findViewById(R.id.ip4);
	    ip1.setMinValue(1);
	    ip1.setMaxValue(254);
	    ip1.setWrapSelectorWheel(true); 
	    ip2.setMinValue(0);
	    ip2.setMaxValue(254);
	    ip2.setWrapSelectorWheel(true); 
	    ip3.setMinValue(0);
	    ip3.setMaxValue(254);
	    ip3.setWrapSelectorWheel(true); 
	    ip4.setMinValue(0);
	    ip4.setMaxValue(254);
	    ip4.setWrapSelectorWheel(true);
	    
	    BufferedReader br;
	    String newIp;
	    boolean addme;
		try {
			br = new BufferedReader(new FileReader(fileIPExperiment));
			while ((newIp = br.readLine()) != null) {
				addme = true;
				// if already in the list do not add another one
				if(managedIps.contains(new ManagedIp(newIp, newIp,true))) 
					addme = false;
				
				Log.d("managermanager", "writing");
				if(addme){
					managedIps.add(new ManagedIp(newIp, newIp,true));
				}
			}
			br.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		/*
		// Set up an instance of SystemUiHider to control the system UI for
		// this activity.
		mSystemUiHider = SystemUiHider.getInstance(this, contentView,
				HIDER_FLAGS);
		mSystemUiHider.setup();
		mSystemUiHider
				.setOnVisibilityChangeListener(new SystemUiHider.OnVisibilityChangeListener() {
					// Cached values.
					int mControlsHeight;
					int mShortAnimTime;

					@Override
					@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR2)
					public void onVisibilityChange(boolean visible) {
						if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR2) {
							// If the ViewPropertyAnimator API is available
							// (Honeycomb MR2 and later), use it to animate the
							// in-layout UI controls at the bottom of the
							// screen.
							if (mControlsHeight == 0) {
								mControlsHeight = controlsView.getHeight();
							}
							if (mShortAnimTime == 0) {
								mShortAnimTime = getResources().getInteger(
										android.R.integer.config_shortAnimTime);
							}
							controlsView
									.animate()
									.translationY(visible ? 0 : mControlsHeight)
									.setDuration(mShortAnimTime);
						} else {
							// If the ViewPropertyAnimator APIs aren't
							// available, simply show or hide the in-layout UI
							// controls.
							controlsView.setVisibility(visible ? View.VISIBLE
									: View.GONE);
						}

						if (visible && AUTO_HIDE) {
							// Schedule a hide().
							delayedHide(AUTO_HIDE_DELAY_MILLIS);
						}
					}
				});

		// Set up the user interaction to manually show or hide the system UI.
		contentView.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View view) {
				if (TOGGLE_ON_CLICK) {
					mSystemUiHider.toggle();
				} else {
					mSystemUiHider.show();
				}
			}
		});

		// Upon interacting with UI controls, delay any scheduled hide()
		// operations to prevent the jarring behavior of controls going away
		// while interacting with the UI.
		findViewById(R.id.startexperiment).setOnTouchListener(
				mDelayHideTouchListener);*/
	}

	@Override
	protected void onPostCreate(Bundle savedInstanceState) {
		super.onPostCreate(savedInstanceState);

		// Trigger the initial hide() shortly after the activity has been
		// created, to briefly hint to the user that UI controls
		// are available.
		//delayedHide(100);
	}

	/**
	 * Set up the {@link android.app.ActionBar}, if the API is available.
	 */
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	private void setupActionBar() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
			// Show the Up button in the action bar.
			getActionBar().setDisplayHomeAsUpEnabled(true);
		}
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
			// TODO: If Settings has multiple levels, Up should navigate up
			// that hierarchy.
			NavUtils.navigateUpFromSameTask(this);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	/**
	 * Touch listener to use for in-layout UI controls to delay hiding the
	 * system UI. This is to prevent the jarring behavior of controls going away
	 * while interacting with activity UI.
	 */
	View.OnTouchListener mDelayHideTouchListener = new View.OnTouchListener() {
		@Override
		public boolean onTouch(View view, MotionEvent motionEvent) {
			if (AUTO_HIDE) {
				delayedHide(AUTO_HIDE_DELAY_MILLIS);
			}
			return false;
		}
	};

	Handler mHideHandler = new Handler();
	Runnable mHideRunnable = new Runnable() {
		@Override
		public void run() {
			mSystemUiHider.hide();
		}
	};

	/**
	 * Schedules a call to hide() in [delay] milliseconds, canceling any
	 * previously scheduled calls.
	 */
	private void delayedHide(int delayMillis) {
		mHideHandler.removeCallbacks(mHideRunnable);
		mHideHandler.postDelayed(mHideRunnable, delayMillis);
	}
	
    public void addStation(View view) {
    	boolean addme = true;
		NumberPicker ip1 = (NumberPicker) findViewById(R.id.ip1);
		NumberPicker ip2 = (NumberPicker) findViewById(R.id.ip2);
		NumberPicker ip3 = (NumberPicker) findViewById(R.id.ip3);
		NumberPicker ip4 = (NumberPicker) findViewById(R.id.ip4);
		String newIp = ip1.getValue() + "." + ip2.getValue() + "." + ip3.getValue() + "." + ip4.getValue();
		/*
    	ListView ipList = (ListView)findViewById(R.id.IpToManage);
    	long[] listIp = ipList.getCheckedItemIds();
    	for (int i = 0; i < listIp.length;i++){
    		if(newIp.equals(ipList.getItemAtPosition((int)(listIp[i])))){
    			addme = false;
    		}
    	}
    	*/
		if(managedIps.contains(new ManagedIp(newIp, newIp,true))) 
			addme = false;
				if(addme){
			managedIps.add(new ManagedIp(newIp, newIp,true));
		}
    }
	
    public void startExperimentButton(View view) {

    	String fileContent = "";
        for (ManagedIp p : boxAdapter.getBox()) {
			if (p.selected){
				fileContent += p.name+"\n";
			}
        }
        Log.d("managermanagermanagermanager", "got file content " + fileContent);
        PrintWriter out;
		try {
			out = new PrintWriter(fileIPExperiment);
	        out.write(fileContent);
	        out.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


		Log.d("manager", "starting");
		s = new String[3];
	
		//PARAMETRI
		s[0] = "4000";
		s[1] = "fileIP";
		
		//DEVE ESSERE UN PARAMETRO
		s[2] = "0";
    	this.startExperiment(3,s);
    }
    public void stopExperimentButton(View view) {
    	//write out fileIP
    	//native function with port, filename, delay
		Log.d("manager", "stopping");
		
    	String fileContentKill = "", fileContentNormal = "";
        for (ManagedIp p : boxAdapter.getBox()) {
			if (p.selected){
				fileContentKill += p.name+"\n";
			}else{
				fileContentNormal += p.name+"\n";
			}
        }
        Log.d("manager", "got file content kill " + fileContentKill);
        Log.d("manager", "got file content normal " + fileContentNormal);
        PrintWriter outKill,outNormal;
		try {
			outKill = new PrintWriter(fileIPExperimentStop);
			outKill.write(fileContentKill);
			outKill.close();
			outNormal = new PrintWriter(fileIPExperiment);
			outNormal.write(fileContentNormal);
			outNormal.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		s = new String[2];
		s[0] = "4001";
		s[1] = "fileIPKill";
    	this.stopExperiment(2,s);
    }
    
}
