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

import android.os.Bundle;
//import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
//import android.view.ViewGroup.LayoutParams;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.support.v4.app.NavUtils;
//import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;
import android.graphics.Color;
//import android.app.Activity;
//import android.graphics.Color;
//import android.graphics.LinearGradient;
//import android.graphics.Shader;
//import android.os.Bundle;
import com.androidplot.Plot;
import com.androidplot.xy.*;

import java.text.DecimalFormat;
import java.util.Observable;
import java.util.Observer;
//import com.androidplot.Plot;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.series.XYSeries;
//import com.androidplot.xy.*;

import java.util.Arrays;
//import java.util.Observable;
//import java.util.Observer;

import android.app.ActionBar;
import android.app.ActivityManager;
import android.app.FragmentTransaction;
import android.app.ActivityManager.RunningServiceInfo;
//import android.content.Intent;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class MACActivity extends FragmentActivity implements ActionBar.TabListener {
    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide fragments for each of the
     * three primary sections of the app. We use a {@link android.support.v4.app.FragmentPagerAdapter}
     * derivative, which will keep every loaded fragment in memory. If this becomes too memory
     * intensive, it may be best to switch to a {@link android.support.v4.app.FragmentStatePagerAdapter}.
     */
    AppSectionsPagerAdapter mAppSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will display the three primary sections of the app, one at a
     * time.
     */
    ViewPager mViewPager;
//    private Thread mySource;
    public static MACSource data;
    //public static XYPlot dynamicPlot;
//    private XYPlot staticPlot;
    public static boolean isIperfRunning;
    public static boolean isOlsrRunning;
    private static String paramUrl, paramMAC;
    private static int paramT,paramI,paramPort,paramVerbose, paramSleep;
    //public MyPlotUpdater plotUpdater;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_mac);
        data = new MACSource();
        isIperfRunning = isIperfRunning();
        isOlsrRunning = isOlsrRunning();
   	    // Get the message from the intent
   	    Intent intent = getIntent();
   	    paramUrl = intent.getStringExtra(MainActivity.PARAM_URL);
	   	paramT = intent.getIntExtra(MainActivity.PARAM_T,200);
	   	paramI = intent.getIntExtra(MainActivity.PARAM_I,5);
	   	paramMAC = intent.getStringExtra(MainActivity.PARAM_MAC);
	   	paramSleep = intent.getIntExtra(MainActivity.PARAM_SLEEP,500);
	   	paramPort = intent.getIntExtra(MainActivity.PARAM_PORT,4000);
	   	paramVerbose = intent.getIntExtra(MainActivity.PARAM_VERBOSE,1);
        data.setUrl(paramUrl);
        data.setT(paramT);
        data.setI(paramI);
        data.setPort(paramPort);
        data.setSleep(paramSleep);
        data.setVerbose(paramVerbose);
        data.setMAC(paramMAC);
        // kick off the data generating thread:
        //mySource = new Thread(data);
        //mySource.start();
        data.startMAC();
		
        // Create the adapter that will return a fragment for each of the three primary sections
        // of the app.
        mAppSectionsPagerAdapter = new AppSectionsPagerAdapter(getSupportFragmentManager());

        // Set up the action bar.
        final ActionBar actionBar = getActionBar();

        // Specify that the Home/Up button should not be enabled, since there is no hierarchical
        // parent.
        actionBar.setHomeButtonEnabled(false);

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
        for (int i = 0; i < mAppSectionsPagerAdapter.getCount(); i++) {
            // Create a tab with text corresponding to the page title defined by the adapter.
            // Also specify this Activity object, which implements the TabListener interface, as the
            // listener for when this tab is selected.
            actionBar.addTab(
                    actionBar.newTab()
                            .setText(mAppSectionsPagerAdapter.getPageTitle(i))
                            .setTabListener(this));
        }
        // Show the Up button in the action bar.

	}
	
	public void onStop(){
		super.onStop();
		data.stopMAC();
	}
	
	
	public void onResume(){
		super.onResume();
		if(!data.isRunning()) data.startMAC();
	}
	
    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        // When the given tab is selected, switch to the corresponding page in the ViewPager.
        mViewPager.setCurrentItem(tab.getPosition());
    }

    @Override
    public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }
    
    private boolean isOlsrRunning() {
    	String serviceName = "it.durip_app.Olsr";
    	ActivityManager manager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
    	for (RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
    	    if (serviceName.equals(service.service.getClassName())) {
    	        return true;
    	    }
    	}
    	return false;
    }
    
	public void handleOlsr(View view){
		boolean on = ((ToggleButton) view).isChecked();
		if (on){
			openOlsr(view);
		}else{
			closeOlsr(view);
		}
		isOlsrRunning = isOlsrRunning();
	}
	
    public void closeOlsr(View view) {
		if(isOlsrRunning()){
	        try{
	        	stopService(new Intent(this, Olsr.class));
	        }catch(Exception e){
	    		Log.i("stop err", e.toString());
	        }
		}
    }   
    
    public void openOlsr(View view) {
		if(!isOlsrRunning()){
	        Intent intentOlsr = new Intent(this, Olsr.class);
	        intentOlsr.putExtra(Olsr.LOOP, false);
	        try{
	        	startService(intentOlsr); // make the request!
	        }catch(Exception e){
	    		Log.i("superusering err", e.toString());
	        }
		}
    }
    
    private boolean isIperfRunning() {
    	String serviceName = "it.durip_app.Olsr";
    	ActivityManager manager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
    	for (RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
    	    if (serviceName.equals(service.service.getClassName())) {
    	        return true;
    	    }
    	}
    	return false;
    }
    
	public void handleIperf(View view){
		boolean on = ((ToggleButton) view).isChecked();
		if (on){
			openIperf(view);
		}else{
			closeIperf(view);
		}
		isIperfRunning = isIperfRunning();
	}
	
    public void closeIperf(View view) {
		if(isIperfRunning()){
	        try{
	        	stopService(new Intent(this, Iperf.class));
	        }catch(Exception e){
	    		Log.i("stop err", e.toString());
	        }
		}
    }   
    
    public void openIperf(View view) {
		if(!isIperfRunning()){
	        Intent intentGraph = new Intent(this, Iperf.class);
	        intentGraph.putExtra(Iperf.DESTINATION, paramUrl);
	        intentGraph.putExtra(Iperf.LOOP, false);
	        intentGraph.putExtra(Iperf.TIME, paramT+"");
	        intentGraph.putExtra(Iperf.INTERVAL, paramI+"");
	        try{
	        	startService(intentGraph); // make the request!
	        }catch(Exception e){
	    		Log.i("superusering err", e.toString());
	        }
		}
    }

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the primary
     * sections of the app.
     */
    public static class AppSectionsPagerAdapter extends FragmentPagerAdapter {
        public AppSectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int i) {
        	Fragment fragment;
            Bundle args = new Bundle();
            if (i < 4){
                fragment = new GraphTCPMACFragment();
                args.putInt(GraphTCPMACFragment.ARG_SECTION_NUMBER, i + 1);
            }else{
                fragment = new AccelerometerFragment();
                args.putInt(AccelerometerFragment.ARG_SECTION_NUMBER, i + 1);
            }
            fragment.setArguments(args);
            return fragment;
        }

        @Override
        public int getCount() {
            return 4;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return "Section " + (position + 1);
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
		getMenuInflater().inflate(R.menu.graph, menu);
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
     * A fragment that launches graph for TCP.
     */
    public static class GraphTCPMACFragment extends Fragment {
        // redraws a plot whenever an update is received:
        private class MyPlotUpdater implements Observer {
            Plot plot;
            public MyPlotUpdater(Plot plot) {
                this.plot = plot;
            }
            @Override
            public void update(Observable o, Object arg) {
                try {
                    plot.redraw();
                } catch (Exception e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
        private XYPlot dynamicPlot;
        private XYPlot staticPlot;
        //private MyPlotUpdater plotUpdater;
        public static final String ARG_SECTION_NUMBER = "section_number";     
        
        public static final String MAC = "MAC";
        public static final String RSSi = "RSSi";
        public static final String RXB = "RXB";
        public static final String RXDROP= "RXDROP";
        public static final String RXDUPL= "RXDUPL";
        public static final String RXFRAG= "RXFRAG";
        public static final String RXPACKS= "RXPACKS";
        public static final String TXB = "TXB";
        public static final String TXFILT = "TXFILT";
        public static final String TXFRAG = "TXFRAG";
        public static final String TXPACKS = "TXPACKS";
        public static final String TXRETRYCOUNT = "TXRETRYCOUNT";
        public static final String TXRETRYF = "TXRETRYF";

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.graphs, container, false);
            Bundle args = getArguments();
            int nr = args.getInt(ARG_SECTION_NUMBER);
            //((TextView) rootView.findViewById(R.id.texxt)).setText(
            //        getString(R.string.plotNr, nr));
            ((ToggleButton) rootView.findViewById(R.id.iperfManager)).setChecked(isIperfRunning);
            // get handles to our View defined in layout.xml:
            dynamicPlot = (XYPlot) rootView.findViewById(R.id.mySimpleXYPlot);
            
            MyPlotUpdater plotUpdater = new MyPlotUpdater(dynamicPlot);
            // getInstance and position datasets:

                       
            MACDynamicGraph seriesA;
            MACDynamicGraph seriesB;
            MACDynamicGraph seriesC;
            MACDynamicGraph seriesD;
            
            switch (nr){
          
            case 1:
            	seriesA = new MACDynamicGraph(data, MACSource.RXB, RXB);
            	seriesB = new MACDynamicGraph(data, MACSource.TXB, TXB);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
            	break;
            case 2:
            	seriesA = new MACDynamicGraph(data, MACSource.RXDROP, RXDROP);
            	seriesB = new MACDynamicGraph(data, MACSource.RXDUPL, RXDUPL);
            	seriesC = new MACDynamicGraph(data, MACSource.RXFRAG, RXFRAG);
            	seriesD = new MACDynamicGraph(data, MACSource.RXPACKS, RXPACKS);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesC, new LineAndPointFormatter(Color.rgb(100, 50, 150),Color.rgb(200, 150, 150), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesD, new LineAndPointFormatter(Color.rgb(150, 0, 50),Color.rgb(250, 0, 0), null, new PointLabelFormatter(0)));
            	break;
            case 3:
                //TXRETRYF
            	seriesA = new MACDynamicGraph(data, MACSource.TXFILT, TXFILT);
            	seriesB = new MACDynamicGraph(data, MACSource.TXFRAG, TXFRAG);
            	seriesC = new MACDynamicGraph(data, MACSource.TXPACKS, TXPACKS);
            	seriesD = new MACDynamicGraph(data, MACSource.TXRETRYCOUNT, TXRETRYCOUNT);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesC, new LineAndPointFormatter(Color.rgb(100, 50, 150),Color.rgb(200, 150, 150), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesD, new LineAndPointFormatter(Color.rgb(150, 0, 50),Color.rgb(250, 0, 0), null, new PointLabelFormatter(0)));
            	break;
            default:
            	seriesA = new MACDynamicGraph(data, MACSource.RSSi, RSSi);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(40, 70, 0),Color.rgb(0, 40, 70), null, new PointLabelFormatter(0)));
            }
            
            // hook up the plotUpdater to the data model:
            data.addObserver(plotUpdater);
            
            

            // only display whole numbers in domain labels
            dynamicPlot.getGraphWidget().setDomainValueFormat(new DecimalFormat("0"));
            // update our domain and range axis labels:
            dynamicPlot.setDomainLabel("Packets");
            dynamicPlot.getDomainLabelWidget().pack();
            dynamicPlot.setRangeLabel("Time (ms)");
            dynamicPlot.getRangeLabelWidget().pack();
            dynamicPlot.setGridPadding(5, 0, 5, 0);
            dynamicPlot.setDomainStepMode(XYStepMode.SUBDIVIDE);
            dynamicPlot.setDomainStepValue(seriesA.size());
            // thin out domain/range tick labels so they dont overlap each other:
            dynamicPlot.setTicksPerDomainLabel(100);
            dynamicPlot.setTicksPerRangeLabel(5);
            // freeze the range boundaries:
            dynamicPlot.setRangeBoundaries(0, 50, BoundaryMode.AUTO);

            return rootView;
        }
        
    }

    /**
     * A dummy fragment representing a section of the app, but that simply displays dummy text.
     */
    public static class AccelerometerFragment extends Fragment {
        private XYPlot mySimpleXYPlot;
        public static final String ARG_SECTION_NUMBER = "section_number";

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.graphs, container, false);
            Bundle args = getArguments();
            int nr = args.getInt(ARG_SECTION_NUMBER);
            ((TextView) rootView.findViewById(R.id.texxt)).setText(
                    getString(R.string.plotNr, nr));

 
    	    
            // initialize our XYPlot reference:
            //mySimpleXYPlot = new XYPlot(this,"great plot"); 
            mySimpleXYPlot = (XYPlot) rootView.findViewById(R.id.mySimpleXYPlot);
     
            // Create a couple arrays of y-values to plot:
            Number[] series1Numbers = {nr*1, nr*3, nr*5, nr*2, nr*7, nr*4};
            Number[] series2Numbers = {6/nr, 12/nr, 6/nr, 24/nr, 12/nr, 6/nr};
     
            // Turn the above arrays into XYSeries':
            XYSeries series1 = new SimpleXYSeries(
                    Arrays.asList(series1Numbers),          // SimpleXYSeries takes a List so turn our array into a List
                    SimpleXYSeries.ArrayFormat.Y_VALS_ONLY, // Y_VALS_ONLY means use the element index as the x value
                    "Series1");                             // Set the display title of the series
     
            // same as above
            XYSeries series2 = new SimpleXYSeries(Arrays.asList(series2Numbers), SimpleXYSeries.ArrayFormat.Y_VALS_ONLY, "Series2");
     
            // Create a formatter to use for drawing a series using LineAndPointRenderer:

    		LineAndPointFormatter series1Format = new LineAndPointFormatter(
                    Color.rgb(0, 200, 0),                   // line color
                    Color.rgb(0, 100, 0),                   // point color
                    null,									// fill color
                    new PointLabelFormatter(0));            // fill color (none)
     
            // add a new series' to the xyplot:
            mySimpleXYPlot.addSeries(series1, series1Format);
     
            // same as above:
            mySimpleXYPlot.addSeries(series2, new LineAndPointFormatter(Color.rgb(0, 0, 200), Color.rgb(0, 0, 100), null,new PointLabelFormatter(0)));
     
            // reduce the number of range labels
            mySimpleXYPlot.setTicksPerRangeLabel(3);
            
    		//setupActionBar();
            
            
            return rootView;
        }
    }
}
