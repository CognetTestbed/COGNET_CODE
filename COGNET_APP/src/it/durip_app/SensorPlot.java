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


import android.annotation.TargetApi;
import android.app.ActionBar;
//import android.app.Activity;
import android.app.FragmentTransaction;
import android.content.Context;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.NavUtils;
import android.support.v4.view.ViewPager;
//import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
//import android.widget.CheckBox;
//import android.widget.CompoundButton;
//import android.widget.TextView;


//import com.androidplot.series.XYSeries;
//import com.androidplot.util.PlotStatistics;
import com.androidplot.xy.*;

import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
//import java.io.FileNotFoundException;
//import java.io.FileOutputStream;
//import java.io.IOException;
//import java.io.OutputStream;
//import java.io.OutputStreamWriter;
//import java.io.UnsupportedEncodingException;
//import java.io.Writer;
//import java.text.FieldPosition;
//import java.text.Format;
//import java.text.ParsePosition;
import java.util.Arrays;

// Monitor the phone's orientation sensor and plot the resulting azimuth pitch and roll values.
// See: http://developer.android.com/reference/android/hardware/SensorEvent.html
//@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class SensorPlot extends FragmentActivity implements ActionBar.TabListener
{
    AppSectionsPagerAdapter mAppSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will display the three primary sections of the app, one at a
     * time.
     */
    ViewPager mViewPager;
    public static SensorManager managerSensor;

    //private LinkedList<Number> azimuthHistory;
    //private LinkedList<Number> pitchHistory;
    //private LinkedList<Number> rollHistory;

    /*{
        azimuthHistory = new LinkedList<Number>();
        pitchHistory = new LinkedList<Number>();
        rollHistory = new LinkedList<Number>();
    }*/

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensors);
        managerSensor = (SensorManager) getApplicationContext().getSystemService(Context.SENSOR_SERVICE);
        
        // Create the adapter that will return a fragment for each of the three primary sections
        // of the app.
        mAppSectionsPagerAdapter = new AppSectionsPagerAdapter(getSupportFragmentManager());

        // Set up the action bar.
        final ActionBar actionBar = getActionBar();

        // Specify that the Home/Up button should not be enabled, since there is no hierarchical
        // parent.
        actionBar.setHomeButtonEnabled(true);

        // Specify that we will be displaying tabs in the action bar.
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

        // Set up the ViewPager, attaching the adapter and setting up a listener for when the
        // user swipes between sections.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mAppSectionsPagerAdapter);
        mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                // When swiping between different app sections, select the corresponding tab.
                // We can also use ActionBar.Tab#select() to do this if we have a reference to the
                // Tab.
                actionBar.setSelectedNavigationItem(position);
            }
        });
        
        // For each of the sections in the app, add a tab to the action bar.
        for (int ii = 0; ii < mAppSectionsPagerAdapter.getCount(); ii++) {
            // Create a tab with text corresponding to the page title defined by the adapter.
            // Also specify this Activity object, which implements the TabListener interface, as the
            // listener for when this tab is selected.
            actionBar.addTab(actionBar.newTab().setText(mAppSectionsPagerAdapter.getPageTitle(ii+1))
                            .setTabListener(this));
            
        }
        // Show the Up button in the action bar.        
        
        System.out.println("OnCreate \n");


    }
    @Override
    protected void onStart(){
        super.onStart();
        //finish();	
    }
    
    @Override
    protected void onRestart(){
        super.onRestart();
        //finish();	
    }
    
    @Override
    protected void onResume(){
        super.onResume();
        //finish();
    }
    
    @Override
    protected void onPause(){
        super.onPause();
        finish();
    }
    
    @Override
    protected void onStop(){
        super.onStop();
        finish();
    	
    }
    
    @Override
    protected void onDestroy(){
        super.onDestroy();
        finish();
    }

    
    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    	System.out.println("onTabUnselected \n");
//        mViewPager.setCurrentItem(13);
    }

    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        // When the given tab is selected, switch to the corresponding page in the ViewPager.
    	System.out.println("onTabSelected \n");
        mViewPager.setCurrentItem(tab.getPosition());
        
    }

    @Override
    public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the primary
     * sections of the app.
     */
    public static class AppSectionsPagerAdapter extends FragmentPagerAdapter {
    	
    	
    	private final static int numberFrame = 12;
    	
        public AppSectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int i) {
        	Fragment fragment;
        	
            Bundle args = new Bundle();
//            System.out.println(args.);
            //if (i < 6){
                fragment = new AccelerometerFragment();
                args.putInt(AccelerometerFragment.ARG_SECTION_NUMBER, i + 1);
            //}
            fragment.setArguments(args);
            return fragment;
        }

        @Override
        public int getCount() {
            return numberFrame;
        }

        @Override
        public CharSequence getPageTitle(int position) {
        	String namePlot = new String();
        	switch(position){
        	case 1:
        	case 2:
        		namePlot = "ACCELEROMETER";
        		break;
        	case 3:        		
        	case 4:
        		namePlot = "GYROSCOPE";
        		break;
        	case 5:
        	case 6:
        		namePlot = "ORIENTATION";
        		break;
        	case 7:
        	case 8:
        		namePlot = "FUSION ACCELEROMETER";
        		break;
        	case 9:
        	case 10:
        		namePlot = "GRAVITY";
        		break;
        	case 11:
        	case 12:
        		namePlot = "ORIENTATION VECT";
        		break;
        	 
        	}
        	
            return namePlot;
        }
    }
    
    

	/**
	 * Set up the {@link android.app.ActionBar}, if the API is available.
	 */
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	private void setupActionBar() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
			getActionBar().setDisplayHomeAsUpEnabled(true);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.sensor, menu);
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

    /**
     * A dummy fragment representing a section of the app, but that simply displays dummy text.
     */
    public static class AccelerometerFragment extends Fragment implements SensorEventListener {
        /**
         * A simple formatter to convert bar indexes into sensor names.
         */
//        private class APRIndexFormat extends Format {
//            @Override
//            public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
//                Number num = (Number) obj;
//                
//                // using num.intValue() will floor the value, so we add 0.5 to round instead:
//                int roundNum = (int) (num.floatValue() + 0.5f);
//                switch(roundNum) {
//                    case 0:
//                        toAppendTo.append(labels[0]);
//                        break;
//                    case 1:
//                        toAppendTo.append(labels[1]);
//                        break;
//                    case 2:
//                        toAppendTo.append(labels[2]);
//                        break;
//                    default:
//                        toAppendTo.append("Unknown");
//                }
//                return toAppendTo;
//            }
//
//            @Override
//            public Object parseObject(String source, ParsePosition pos) {
//                return null;  // We don't use this so just return null for now.
//            }
//        }//private class
        
        private static final int HISTORY_SIZE = 50;            // number of points to plot in history
        private SensorManager sensorMgr = null;
//        private Sensor orSensor3 = null;
//        private Sensor orSensor2 = null;
        private Sensor orSensor = null;

        private XYPlot aprLevelsPlot = null;
        private XYPlot aprHistoryPlot = null;
        private String[] labels;
//        private CheckBox hwAcceleratedCb;
//        private CheckBox showFpsCb;
        private SimpleXYSeries aprLevelsSeries = null;
        private SimpleXYSeries azimuthHistorySeries = null;
        private SimpleXYSeries pitchHistorySeries = null;
        private SimpleXYSeries rollHistorySeries = null;
        
        final String[] barLabel = new String[] {
                "X","Y","Z"
            };
//        private OutputStream outGravity = null;
//        private OutputStream outLinear = null;
//        private OutputStream outRotation = null;
//        private Writer writeGravity;
//        private Writer writeRotation;
//        private Writer writeLinear;
        //Number to identify sensor to plot
        private int nr;
        public static final String ARG_SECTION_NUMBER = "section_number";
//    	private TextView[] dataEntries;
    	
    	
        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
        	
            Bundle args = getArguments();
            nr = args.getInt(ARG_SECTION_NUMBER);            
            switch(nr){
	            case 1:
	            case 2:
		            labels = new String[] {"X","Y","Z","Accelerometer" , "m/s^2"};
		            break;

	            case 3:
	            case 4:
		            labels = new String[] {"X-axis","Y-axis","Z-axis","Gyroscope" , "rad/sec" };
		            break;
	            case 5:
	            case 6:
		            labels = new String[] {"Azimuth","Pitch","Roll","Orientation" , "Deg"};
		            break;
		            //A three dimensional vector indicating acceleration along each device axis, 
//		            not including gravity. All values have units of m/s^2
	            case 7:
	            case 8:
		            labels = new String[] {"X","Y","Z","Linear Acceleration" ,"m/s^2"};
		            break;
	            case 9:
	            case 10:
		            labels = new String[] {"Z+X+Y/rad+Z/rad","Y+Z/rad+X/rad","Null->orientation","Gravity" , "m/s^2"};
		            break;
	            case 11:
	            case 12:
		            labels = new String[] {"Y.Z tgGround East","tgGround North","PerpGround","Rotation Vector" , "TBD"};
		            break;

	        }
 
            View rootView;

            rootView = inflater.inflate(R.layout.sensors, container, false);

            //                ((TextView) rootView.findViewById(R.id.texxt)).setText(
            //                        getString(R.string.plotNr, nr));

            //                hwAcceleratedCb = (CheckBox) rootView.findViewById(R.id.hwAccelerationCb);
            //                showFpsCb = (CheckBox) rootView.findViewById(R.id.showFpsCb);
            if (nr%2 == 0){
            	aprLevelsPlot = (XYPlot) rootView.findViewById(R.id.aprHistoryPlot);

            	aprLevelsSeries = new SimpleXYSeries(labels[3]);
            	//	                aprLevelsSeries.useImplicitXVals();
            	//	                aprLevelsSeries.s
            	aprLevelsPlot.addSeries(aprLevelsSeries, new BarFormatter(Color.argb(100, 0, 200, 0), Color.rgb(0, 80, 0)));
            	aprLevelsPlot.setDomainStepValue(3);
            	aprLevelsPlot.setTicksPerRangeLabel(3);
            	aprLevelsPlot.setRangeLabel(labels[4]);

//aprLevelsPlot.
            	// per the android documentation, the minimum and maximum readings we can get from
            	// any of the orientation sensors is -180 and 359 respectively so we will fix our plot's
            	// boundaries to those values.  If we did not do this, the plot would auto-range which
            	// can be visually confusing in the case of dynamic plots.
            	//	                aprLevelsPlot.setRangeBoundaries(-180, 359, BoundaryMode.AUTO);

            	// use our custom domain value formatter:
            	//	                aprLevelsPlot.setDomainValueFormat(new APRIndexFormat());

            	// update our domain and range axis labels:
            	//	                aprLevelsPlot.setDomainLabel("Axis");
            	aprLevelsPlot.getDomainLabelWidget().pack();

            	aprLevelsPlot.getRangeLabelWidget().pack();
            	aprLevelsPlot.setTitle(labels[3]);
            	aprLevelsPlot.setGridPadding(15, 0, 15, 0);
            	//aprLevelsPlot.addListener(new PlotStatistics(1000, true));

            
            	aprLevelsPlot.setDomainValueFormat(new Format() {            		 
                    @Override
                    public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
                        return new StringBuffer( barLabel[ ( (Number)obj).intValue() ]  );
                    }
         
                    @Override
                    public Object parseObject(String source, ParsePosition pos) {
                        return null;
                    }
                });
            	
            	
            	BarRenderer barRenderer = (BarRenderer) aprLevelsPlot.getRenderer(BarRenderer.class);
            	if(barRenderer != null) {
            		// make our bars a little thicker than the default so they can be seen better:
            		barRenderer.setBarWidth(100);
//            		barRenderer.
            	}
            }else{

            	// setup the APR History plot:
            	aprHistoryPlot = (XYPlot) rootView.findViewById(R.id.aprHistoryPlot);

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

            }
            registerSensor();
            return rootView;
        }
        
        @Override
        public void onStop(){
        	super.onStop();
        	sensorMgr.unregisterListener(this);
        }
        public void onPause(){
        	super.onPause();
        	sensorMgr.unregisterListener(this);
        }
        @Override
        public void onResume(){
        	super.onResume();
        	sensorMgr.unregisterListener(this);
        	registerSensor();
        }
        public void onStart(){
        	super.onStart();
        	sensorMgr.unregisterListener(this);
        	registerSensor();
        }
        private void cleanup() {
            // unregister with the orientation sensor before exiting:
            sensorMgr.unregisterListener(this);
            //finish();
        }
        
        
        
        //PARTE DEL SENSORE
        private void registerSensor(){
            // register for orientation sensor events:
            sensorMgr = managerSensor;
            System.out.println("REGISTER SENSOR");
            switch(nr){
	            case 1:
	            case 2:
		            for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_ACCELEROMETER)) {
		                if (sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
		                    orSensor = sensor;
		                }
		            }
		            break;
	            case 3:
	            case 4:
		            for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_GYROSCOPE)) {
		                if (sensor.getType() == Sensor.TYPE_GYROSCOPE) {
		                    orSensor = sensor;
		                }
		            }
		            break;
	            case 5:
	            case 6:
		            for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_ORIENTATION)) {
		                if (sensor.getType() == Sensor.TYPE_ORIENTATION) {
		                    orSensor = sensor;
		                }
		            }
		            break;
		            
//FUSION VALUE		            
	            case 7:
	            case 8:
		            for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_LINEAR_ACCELERATION)) {
		                if (sensor.getType() == Sensor.TYPE_LINEAR_ACCELERATION) {
		                    orSensor = sensor;
		                }
		            }
		            break;
	            case 9:
	            case 10:
		            for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_GRAVITY)) {
		                if (sensor.getType() == Sensor.TYPE_GRAVITY) {
		                    orSensor = sensor;
		                }
		            }
		            break;
	            case 11:
	            case 12:
		            for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_ROTATION_VECTOR)) {
		                if (sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
		                    orSensor = sensor;
		                }
		            }
		            break;            
            }
            

            // if we can't access the orientation sensor then exit:
            if (orSensor == null) {
                System.out.println("Failed to attach to orSensor.");
                cleanup();
            }

            sensorMgr.registerListener(this, orSensor, SensorManager.SENSOR_DELAY_UI);
        	
        	
        }

        
        
        //TO CHECK
        
        // Called whenever a new orSensor reading is taken.
        @Override
        public synchronized void onSensorChanged(SensorEvent sensorEvent) {

            // update instantaneous data:
            Number[] series1Numbers = {sensorEvent.values[0], sensorEvent.values[1], sensorEvent.values[2]};
            	
            if (nr%2==0){
            	//TO DRAW ON BAR PLOT
            	aprLevelsSeries.setModel(Arrays.asList(series1Numbers), SimpleXYSeries.ArrayFormat.Y_VALS_ONLY);
            	// redraw the Plots:
            	aprLevelsPlot.redraw();
            }else{
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
            }
        }


        @Override
        public void onAccuracyChanged(Sensor sensor, int i) {
            // Not interested in this event
        }
    }
}