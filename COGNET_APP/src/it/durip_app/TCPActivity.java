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
//import android.widget.EditText;
//import android.widget.TextView;
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

//import java.io.BufferedReader;
//import java.io.IOException;
//import java.io.InputStreamReader;
import java.text.DecimalFormat;
import java.util.Observable;
import java.util.Observer;
//import com.androidplot.Plot;
//import com.androidplot.xy.SimpleXYSeries;
//import com.androidplot.series.XYSeries;
//import com.androidplot.xy.*;
//
//import java.util.Arrays;
//import java.util.Observable;
//import java.util.Observer;
//import java.util.regex.Matcher;
//import java.util.regex.Pattern;

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
public class TCPActivity extends FragmentActivity implements ActionBar.TabListener {
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
    public static TCPSource data;
    //public static XYPlot dynamicPlot;
//    private XYPlot staticPlot;
//    private static int frequency = 0;
    private static String paramUrl;
    public static boolean isIperfRunning;
    private static int paramT,paramI,paramPort,paramVerbose, paramSleep;
    //public MyPlotUpdater plotUpdater;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_graph);
        data = new TCPSource();
   	    // Get the message from the intent
        isIperfRunning = isIperfRunning();
   	    Intent intent = getIntent();
   	    paramUrl = intent.getStringExtra(MainActivity.PARAM_URL);
	   	paramT = intent.getIntExtra(MainActivity.PARAM_T,200);
	   	paramI = intent.getIntExtra(MainActivity.PARAM_I,5);
	   	paramPort = intent.getIntExtra(MainActivity.PARAM_PORT,4000);
	   	paramSleep = intent.getIntExtra(MainActivity.PARAM_SLEEP,500);
	   	paramVerbose = intent.getIntExtra(MainActivity.PARAM_VERBOSE,1);
//	   	frequency = paramPort/1000;
        data.setUrl(paramUrl);
        data.setT(paramT);
        data.setI(paramI);
        data.setPort(paramPort);
        data.setSleep(paramSleep);
        data.setVerbose(paramVerbose);
        data.startTCP();
        // kick off the data generating thread:
        /*
        mySource = new Thread(data);
        mySource.start();
		*/
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
		data.stopTCP();
	}
	
	
	public void onResume(){
		super.onResume();
		if(!data.isRunning()) data.startTCP();
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
            if (i < 5){
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
            return 6;
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
    private boolean isIperfRunning() {
    	String serviceName = "it.durip_app.Iperf";
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
        try{
        	stopService(new Intent(this, Iperf.class));
        }catch(Exception e){
    		Log.i("stop err", e.toString());
        }
    }   
    
    public void openIperf(View view) {
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
//        private XYPlot staticPlot;
        //private MyPlotUpdater plotUpdater;
        public static final String ARG_SECTION_NUMBER = "section_number";
        public static final String CWND = "CWND";
        public static final String AW = "AW";
        public static final String SSHTHRESH = "SSHTHRESH";
        public static final String RTT = "RTT";
        public static final String SRTT = "SRTT";
        public static final String RTO = "RTO";
        public static final String SRTTJTU = "SRTTJTU";
        public static final String FLIGHT = "FLIGHT";
        public static final String PACKETOUT = "PACKETOUT";
        public static final String PACKETLOST = "PACKETLOST";
        public static final String PACKETRET = "PACKETRET";
        public static final String PRRDEV = "PRRDEV";
        public static final String PRROUT = "PRROUT";
        public static final String TOTRET = "TOTRET";
        public static final String BYTESACK = "BYTESACK";
        public static final String PKTACKED = "PKTACKED";
        public static final String MSS = "MSS";
        
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

                       
            TCPDynamicGraph seriesA;
            TCPDynamicGraph seriesB;
            TCPDynamicGraph seriesC;
            TCPDynamicGraph seriesD;
            
            switch (nr){
            //TCP
            case 1:
            	seriesA = new TCPDynamicGraph(data, TCPSource.CWND, CWND);
            	seriesB = new TCPDynamicGraph(data, TCPSource.SSHTHRESH, SSHTHRESH);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(150, 150, 0),Color.rgb(150, 0, 0), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(0, 150, 150),Color.rgb(0, 0, 150), null, new PointLabelFormatter(0)));
            	break;
            case 2:
            	seriesA = new TCPDynamicGraph(data, TCPSource.RTT,RTT);
            	seriesB = new TCPDynamicGraph(data, TCPSource.SRTT, SRTT);
            	seriesC = new TCPDynamicGraph(data, TCPSource.SRTTJTU, SRTTJTU);
            	seriesD = new TCPDynamicGraph(data, TCPSource.RTO, RTO);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesC, new LineAndPointFormatter(Color.rgb(100, 50, 150),Color.rgb(200, 150, 150), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesD, new LineAndPointFormatter(Color.rgb(150, 0, 50),Color.rgb(250, 0, 0), null, new PointLabelFormatter(0)));
            	break;
            case 3:
            	seriesA = new TCPDynamicGraph(data, TCPSource.PACKETOUT,PACKETOUT);
            	seriesB = new TCPDynamicGraph(data, TCPSource.PACKETLOST, PACKETLOST);
            	seriesC = new TCPDynamicGraph(data, TCPSource.PACKETRET, PACKETRET);
            	seriesD = new TCPDynamicGraph(data, TCPSource.FLIGHT, FLIGHT);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesC, new LineAndPointFormatter(Color.rgb(100, 50, 150),Color.rgb(200, 150, 150), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesD, new LineAndPointFormatter(Color.rgb(150, 0, 50),Color.rgb(250, 0, 0), null, new PointLabelFormatter(0)));
            	break;
            case 4:
            	seriesA = new TCPDynamicGraph(data, TCPSource.PRRDEV,PRRDEV);
            	seriesB = new TCPDynamicGraph(data, TCPSource.PRROUT, PRROUT);
            	seriesC = new TCPDynamicGraph(data, TCPSource.TOTRET, TOTRET);
            	seriesD = new TCPDynamicGraph(data, TCPSource.MSS, MSS);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesC, new LineAndPointFormatter(Color.rgb(100, 50, 150),Color.rgb(200, 150, 150), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesD, new LineAndPointFormatter(Color.rgb(150, 0, 50),Color.rgb(250, 0, 0), null, new PointLabelFormatter(0)));
            	break;
            case 5:
            	seriesA = new TCPDynamicGraph(data, TCPSource.BYTESACK,BYTESACK);
            	seriesB = new TCPDynamicGraph(data, TCPSource.PKTACKED, PKTACKED);
            	seriesC = new TCPDynamicGraph(data, TCPSource.CWND, CWND);
            	seriesD = new TCPDynamicGraph(data, TCPSource.SRTT, SRTT);
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(0, 150, 100),Color.rgb(0, 250, 250), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesB, new LineAndPointFormatter(Color.rgb(50, 100, 0),Color.rgb(100, 50, 200), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesC, new LineAndPointFormatter(Color.rgb(100, 50, 150),Color.rgb(200, 150, 150), null, new PointLabelFormatter(0)));
                dynamicPlot.addSeries(seriesD, new LineAndPointFormatter(Color.rgb(150, 0, 50),Color.rgb(250, 0, 0), null, new PointLabelFormatter(0)));
            	break;
        	//MAC

            default:
            	seriesA = new TCPDynamicGraph(data, TCPSource.PACKETLOST, "PACKETLOST");
                dynamicPlot.addSeries(seriesA, new LineAndPointFormatter(Color.rgb(40, 70, 0),Color.rgb(0, 40, 70), null, new PointLabelFormatter(0)));
            }
            
            // hook up the plotUpdater to the data model:
            data.addObserver(plotUpdater);
            
            

            // only display whole numbers in domain labels
            dynamicPlot.getGraphWidget().setDomainValueFormat(new DecimalFormat("0"));
            // update our domain and range axis labels:
            dynamicPlot.setDomainLabel("Data");
            dynamicPlot.getDomainLabelWidget().pack();
            dynamicPlot.setRangeLabel("Packets");
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
        public static final String ARG_SECTION_NUMBER = "section_number";

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.tcp_values, container, false);
//            Bundle args = getArguments();
//            int nr = args.getInt(ARG_SECTION_NUMBER);
    	    return rootView;
            
        }
    }
}
