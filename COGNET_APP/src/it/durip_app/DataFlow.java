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
import android.app.Activity;
import android.graphics.Color;
//import android.graphics.LinearGradient;
//import android.graphics.Shader;
import android.os.Bundle;
import com.androidplot.Plot;
import com.androidplot.xy.*;

import java.text.DecimalFormat;
import java.util.Observable;
import java.util.Observer;

import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.support.v4.app.NavUtils;
import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;

public class DataFlow extends Activity {
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
    //private XYPlot staticPlot;
    private MyPlotUpdater plotUpdater;
    private Thread mySource;
    
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_data_flow);
        final int SUPERUSER_REQUEST = 2324; // arbitrary number of your choosing
        Intent intent = new Intent("android.intent.action.superuser"); // superuser request
        intent.putExtra("name", "durip_app"); // tell Superuser the name of the requesting app
        intent.putExtra("packagename", "it.durip_app"); // tel Superuser the name of the requesting package
        //intent.putExtra(SHELL_RESULT, "");
        
        try{
            startActivityForResult(intent, SUPERUSER_REQUEST); // make the request!
        }catch(Exception e){
    		Log.i("superusering err", e.toString());
        }
        
        // get handles to our View defined in layout.xml:
        dynamicPlot = (XYPlot) findViewById(R.id.myPingXYPlot);

        plotUpdater = new MyPlotUpdater(dynamicPlot);

        // only display whole numbers in domain labels
        dynamicPlot.getGraphWidget().setDomainValueFormat(new DecimalFormat("0"));

        // getInstance and position datasets:
        DataSource data = new DataSource();

        DynamicGraph CWN = new DynamicGraph(data, 1, "CWN");
        //DynamicGraph AW = new DynamicGraph(data, 2, "AW");
        //DynamicGraph CWASP = new DynamicGraph(data, 3, "CWASP");
        DynamicGraph SSHTHRESH = new DynamicGraph(data, 4, "SSHTHRESH");
        DynamicGraph RTT = new DynamicGraph(data, 5, "RTT");
        DynamicGraph SRTT = new DynamicGraph(data, 6, "SRTT");
        DynamicGraph FLIGHT = new DynamicGraph(data, 7, "FLIGHT");
        DynamicGraph PACKETOUT = new DynamicGraph(data, 8, "PACKETOUT");
        DynamicGraph PACKETLOST = new DynamicGraph(data, 9, "PACKETLOST");
        DynamicGraph PACKETRET = new DynamicGraph(data, 10, "PACKETRET");

        dynamicPlot.addSeries(CWN, new LineAndPointFormatter(Color.rgb(10, 100, 100),Color.rgb(100, 10, 100), null, new PointLabelFormatter(0)));
        dynamicPlot.addSeries(SSHTHRESH, new LineAndPointFormatter(Color.rgb(40, 70, 0),Color.rgb(0, 40, 70), null, new PointLabelFormatter(0)));
        dynamicPlot.addSeries(RTT, new LineAndPointFormatter(Color.rgb(50, 60, 100),Color.rgb(0, 50, 60), null, new PointLabelFormatter(0)));
        dynamicPlot.addSeries(SRTT, new LineAndPointFormatter(Color.rgb(60, 50, 50),Color.rgb(100, 60, 50), null, new PointLabelFormatter(0)));
        dynamicPlot.addSeries(FLIGHT, new LineAndPointFormatter(Color.rgb(70, 40, 0),Color.rgb(0, 70, 40), null, new PointLabelFormatter(0)));
        
        dynamicPlot.setGridPadding(5, 0, 5, 0);

        // hook up the plotUpdater to the data model:
        data.addObserver(plotUpdater);

        dynamicPlot.setDomainStepMode(XYStepMode.SUBDIVIDE);
        dynamicPlot.setDomainStepValue(CWN.size());

        // thin out domain/range tick labels so they dont overlap each other:
        dynamicPlot.setTicksPerDomainLabel(100);
        dynamicPlot.setTicksPerRangeLabel(30);

        // freeze the range boundaries:
        dynamicPlot.setRangeBoundaries(0, 300, BoundaryMode.FIXED);

        // kick off the data generating thread:
        mySource = new Thread(data);
        mySource.start();
		//myPingXYPlot
		// Show the Up button in the action bar.
		setupActionBar();
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
		getMenuInflater().inflate(R.menu.data_flow, menu);
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
	
	@Override
	public void onDestroy(){
		super.onDestroy();
		mySource.interrupt();		
	}

}
