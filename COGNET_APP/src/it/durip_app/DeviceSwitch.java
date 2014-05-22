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
import java.io.IOException;
import java.io.InputStreamReader;
//import java.lang.reflect.Array;
import java.util.ArrayList;
//import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import it.durip_app.util.SystemUiHider;

//import android.annotation.TargetApi;
import android.app.Activity;
//import android.content.Context;
import android.graphics.Color;
//import android.os.AsyncTask;
//import android.os.Build;
import android.os.Bundle;
//import android.os.Handler;
import android.util.Log;
//import android.view.LayoutInflater;
//import android.view.MotionEvent;
import android.view.View;
//import android.view.ViewParent;
//import android.widget.AdapterView;
//import android.widget.LinearLayout;
import android.widget.ListView;
//import android.widget.TextView;
//import android.widget.Toast;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 * 
 * @see SystemUiHider
 */
public class DeviceSwitch extends Activity {
	
	static {
		System.loadLibrary("hostsLib"); //
    }
	public native String[] activeHosts(int n);
	public native String[] unactiveHosts(int n);
	public native int hostOn(String s);
	public native int hostOff(String s);
	public native int flush();
	public native int fill();
	
	/**
	 * Whether or not the system UI should be auto-hidden after
	 * {@link #AUTO_HIDE_DELAY_MILLIS} milliseconds.
	 */
	private static final boolean AUTO_HIDE = true;

	/**
	 * If {@link #AUTO_HIDE} is set, the number of milliseconds to wait after
	 * user interaction before hiding the system UI.
	 */
	private static final int AUTO_HIDE_DELAY_MILLIS = 3000;

	/**
	 * If set, will toggle the system UI visibility upon interaction. Otherwise,
	 * will show the system UI visibility upon interaction.
	 */
	private static final boolean TOGGLE_ON_CLICK = true;

	/**
	 * The flags to pass to {@link SystemUiHider#getInstance}.
	 */
	private static final int HIDER_FLAGS = SystemUiHider.FLAG_HIDE_NAVIGATION;

	/**
	 * The instance of the {@link SystemUiHider} for this activity.
	 */
	private SystemUiHider mSystemUiHider;
	ListView hostList;
	HostCustomAdapter hostAdapter;
	ArrayList<Host> hostArray = new ArrayList<Host>();

	//Read more: http://www.androidhub4you.com/2013/02/muftitouch-listview-multi-click.html#ixzz2gYu7xsiW
	public String[] JActiveHosts(int i) {
        ArrayList<String> addme = new ArrayList<String>();
		//String result="";
	    try {
	        String inputLine, pingCmd = "head -"+i+" /sdcard/hosts";
	        String []result= null;

	        //Choose runtime or process builder, they are exactly the same!!
	        
	        Runtime r = Runtime.getRuntime();
	        Process p = r.exec(pingCmd);
	        
	        /*
			Process p = new ProcessBuilder()
		    .command("ping", "android.com")
		    .start();
			*/
	        BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
	        
	        // Create a Pattern object
	        Pattern dataPattern = Pattern.compile("^.*((\\d{1,3}\\.){3}\\d{1,3})\\s(([\\da-fA-F]{2}[:-]){5}[\\da-fA-F]{2}).*$");
	        while ((inputLine = in.readLine()) != null) {
		        System.out.println(inputLine);
		        Matcher datas = dataPattern.matcher(inputLine);
		        if (datas.find( )) {
		        	addme.add(inputLine);
		         } else {
		            System.out.println("NO MATCH");
		         }
		        //result += inputLine;

		        //t.setText(result);
	        }
        	in.close();
        }catch (IOException e) {
    		Log.i("read response", e.toString());
        }
	    String[] result = new String[addme.size()];
	    result = addme.toArray(result);
	    return result;
	}
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_device_switch);
		Pattern p = Pattern.compile("((\\d{1,3}\\.){3}(\\d){1,3})\\s*(([\\da-fA-F]{2}[-: ]){5}[\\da-fA-F]{2})");
		Pattern mac = Pattern.compile(".*\\sMAC\\s(([\\da-fA-F]{2}[-: ]){5}[\\da-fA-F]{2}).*");
		Matcher m = null, mmac = null;
		String []hosts = this.JActiveHosts(20); //activeHosts(20) <--<--< NEED JNI TO WORK
		/**
		* add item in arraylist
		*/
		String []iptablesArray = unactiveHosts(100);
		List<String> macList = new LinkedList<String>();
		for( int k = 0; k < iptablesArray.length; k++)
		{
			if (iptablesArray[k] == null) break;
			mmac = mac.matcher(iptablesArray[k]);
			if(mmac.find()) macList.add( (mmac.group(1)).toUpperCase());
		}
		//List<String> macList = Arrays.asList(iptablesArray);
		
		//macList.removeAll(Arrays.asList(new String[]{null}));
		for (int j =0;j<hosts.length;j++){
			m = p.matcher(hosts[j]);
			if(m.find()){
				if (macList.contains((m.group(4)).toUpperCase()) )
					hostArray.add(new Host(m.group(1),(m.group(4)).toUpperCase(),false));
				else
					hostArray.add(new Host(m.group(1),(m.group(4)).toUpperCase(),true));
			}
		}
		/**
		* set item into adapter
		*/
		hostAdapter = new HostCustomAdapter(DeviceSwitch.this, R.layout.row, hostArray);
		hostList = (ListView) findViewById(R.id.hosts);
		hostList.setItemsCanFocus(false);
		hostList.setAdapter(hostAdapter);
		/**
		* get on item click listener
		*/
		/*
		hostList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View v, final int position, long id) {
				Log.i("List View Clicked", "**********");
				Toast.makeText(DeviceSwitch.this,
				"List View Clicked:" + position, Toast.LENGTH_LONG)
				.show();
			}
		});*/

		

		final View controlsView = findViewById(R.id.fullscreen_content_controls);
		final View contentView = findViewById(R.id.fullscreen_content);
		
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
*/
		// Upon interacting with UI controls, delay any scheduled hide()
		// operations to prevent the jarring behavior of controls going away
		// while interacting with the UI.
		/*
		findViewById(R.id.dummy_button).setOnTouchListener(
				mDelayHideTouchListener);
				*/
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
	 * Touch listener to use for in-layout UI controls to delay hiding the
	 * system UI. This is to prevent the jarring behavior of controls going away
	 * while interacting with activity UI.
	 */
	/*
	View.OnTouchListener mDelayHideTouchListener = new View.OnTouchListener() {
		@Override
		public boolean onTouch(View view, MotionEvent motionEvent) {
			if (AUTO_HIDE) {
				delayedHide(AUTO_HIDE_DELAY_MILLIS);
			}
			return false;
		}
	};
*/
	/*
	Handler mHideHandler = new Handler();
	Runnable mHideRunnable = new Runnable() {
		@Override
		public void run() {
			mSystemUiHider.hide();
		}
	};*/

	/**
	 * Schedules a call to hide() in [delay] milliseconds, canceling any
	 * previously scheduled calls.
	 *//*
	private void delayedHide(int delayMillis) {
		mHideHandler.removeCallbacks(mHideRunnable);
		mHideHandler.postDelayed(mHideRunnable, delayMillis);
	}*/
	
	public void flushIptables(View view){
		int i = flush();
		ListView tmp = (ListView)((View)(view.getParent().getParent())).findViewById(R.id.hosts);
		for(int j=0; j<tmp.getChildCount(); ++j) {
		    View nextChild = tmp.getChildAt(j);
		    nextChild.findViewById(R.id.disableHost).setEnabled(true);
		    nextChild.findViewById(R.id.disableHost).setBackgroundColor(Color.parseColor("#FFFFFF"));
		    nextChild.findViewById(R.id.enableHost).setEnabled(false);
		    nextChild.findViewById(R.id.enableHost).setBackgroundColor(Color.parseColor("#0099CC"));
		}
	}
	public void fillIptables(View view){
		int i = fill();
		ListView tmp = (ListView)((View)(view.getParent().getParent())).findViewById(R.id.hosts);
		for(int j=0; j<tmp.getChildCount(); ++j) {
		    View nextChild = tmp.getChildAt(j);
		    nextChild.findViewById(R.id.enableHost).setEnabled(true);
		    nextChild.findViewById(R.id.enableHost).setBackgroundColor(Color.parseColor("#FFFFFF"));
		    nextChild.findViewById(R.id.disableHost).setEnabled(false);
		    nextChild.findViewById(R.id.disableHost).setBackgroundColor(Color.parseColor("#0099CC"));
		}
	}
}
