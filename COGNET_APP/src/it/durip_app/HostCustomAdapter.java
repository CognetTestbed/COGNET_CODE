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
import java.util.ArrayList;
import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class HostCustomAdapter extends ArrayAdapter<Host> {
	
	Context context;
	int layoutResourceId;
	ArrayList<Host> data = new ArrayList<Host>();
	
	public HostCustomAdapter(Context context, int layoutResourceId, ArrayList<Host> data) {
		super(context, layoutResourceId, data);
		this.layoutResourceId = layoutResourceId;
		this.context = context;
		this.data = data;
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View row = convertView;
		UserHolder holder = null;
		
		if (row == null) {
			LayoutInflater inflater = ((Activity) context).getLayoutInflater();
			row = inflater.inflate(layoutResourceId, parent, false);
			holder = new UserHolder();
			holder.textIp = (TextView) row.findViewById(R.id.hostsIp);
			holder.textMac = (TextView) row.findViewById(R.id.hostsMac);
			holder.btnEnable = (Button) row.findViewById(R.id.enableHost);
			holder.btnDisable = (Button) row.findViewById(R.id.disableHost);
			row.setTag(holder);
		} else {
			holder = (UserHolder) row.getTag();
		}
		Host hostDurip = data.get(position);
		holder.textIp.setText(hostDurip.getIp());
		holder.textMac.setText(hostDurip.getMac());
		if (hostDurip.isActive()){
			holder.btnEnable.setEnabled(false);
			holder.btnEnable.setBackgroundColor(Color.parseColor("#0099CC"));
			holder.btnDisable.setEnabled(true);
			holder.btnDisable.setBackgroundColor(Color.parseColor("#FFFFFF"));
		}else{
			holder.btnEnable.setEnabled(true);
			holder.btnEnable.setBackgroundColor(Color.parseColor("#FFFFFF"));
			holder.btnDisable.setEnabled(false);
			holder.btnDisable.setBackgroundColor(Color.parseColor("#0099CC"));
		}
		holder.btnEnable.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				ViewParent x = v.getParent();
				if ((new DeviceSwitch()).hostOn((String)(((TextView) ((View) x).findViewById(R.id.hostsMac)).getText())) == 1) {
					// TODO Auto-generated method stub
					Log.i("ENABLED", "success");
					Toast.makeText(context, "enabled host",
					Toast.LENGTH_LONG).show();
					(((View) x).findViewById(R.id.disableHost)).setEnabled(true);
					((Button) ((View) x).findViewById(R.id.disableHost)).setBackgroundColor(Color.parseColor("#FFFFFF"));
					//
					(((View) x).findViewById(R.id.enableHost)).setEnabled(false);
					((Button) ((View) x).findViewById(R.id.enableHost)).setBackgroundColor(Color.parseColor("#0099CC"));
				}else{
					Log.i("ENABLED", " not enabled");
					Toast.makeText(context, "tryed and failed to enable host",
					Toast.LENGTH_LONG).show();
				}
			}
		});
		holder.btnDisable.setOnClickListener(new OnClickListener() {
		
			@Override
			public void onClick(View v) {
				ViewParent x = v.getParent();
				if ((new DeviceSwitch()).hostOff((String)(((TextView) ((View) x).findViewById(R.id.hostsMac)).getText())) == 1) {
					// TODO Auto-generated method stub
					Log.i("DISABLED", "success");
					Toast.makeText(context, "Disabled host",
					Toast.LENGTH_LONG).show();
					(((View) x).findViewById(R.id.disableHost)).setEnabled(false);
					((Button) ((View) x).findViewById(R.id.disableHost)).setBackgroundColor(Color.parseColor("#0099CC"));
					(((View) x).findViewById(R.id.enableHost)).setEnabled(true);
					((Button) ((View) x).findViewById(R.id.enableHost)).setBackgroundColor(Color.parseColor("#FFFFFF"));
					
				}else{
					Log.i("DISABLED", " not disabled");
					Toast.makeText(context, "tryed and failed to disable host",
					Toast.LENGTH_LONG).show();
				}
			}
		});
		return row;
	
	}
	
	static class UserHolder {
		TextView textIp;
		TextView textMac;
		Button btnEnable;
		Button btnDisable;
	}
}
