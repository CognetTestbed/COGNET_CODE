package it.durip_app;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import android.app.IntentService;
import android.content.Intent;
import android.os.IBinder;
//import android.util.Log;
import android.util.Log;

public class olsrdLog extends IntentService {

	public olsrdLog() {
		super("olsrdLog");
//		// TODO Auto-generated constructor stub
	}
	
	static {
		System.loadLibrary("olsrdLogLib"); 
    }
	public native int olsrdLogOff(int n);
	public native int olsrdLogOn(int n, String [] value);


	public static final String FILENAME="FILENAME";
	public static final String TIME="TIME";
  
	
	private boolean isPlaying=false;
	private static Process p=null,p1=null;
	private static BufferedReader lines=null;
	private static Runtime r = Runtime.getRuntime();
	private String inputLine="";
  
  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {

	  String s[];
	  s = new String[2];
	  
	  s[0]=intent.getStringExtra(FILENAME);
	  s[1]=intent.getStringExtra(TIME); 
	  Log.i("LOG","VALUE "+ s[1]);
	  play(s);
    
	  return(START_NOT_STICKY);
  }
  
  @Override
  public void onDestroy() {
    try {
		stop();
	} catch (IOException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
  }
  
  @Override
  public IBinder onBind(Intent intent) {
    return(null);
  }
  
  private void play(String [] param) {
	  
	  
	  if (lines != null){
          try {
			p1 = r.exec("pgrep olsrd");
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
          lines = new BufferedReader(new InputStreamReader(p1.getInputStream()));
		  try {
		  	  inputLine = lines.readLine();
			  if (inputLine != null){
				  isPlaying = true;
			  }else{
				  isPlaying = false;
				  lines.close();
				  p.destroy();
			  }
		  } catch (IOException e) {
			// TODO Auto-generated catch block
			  e.printStackTrace();
		  }

	  }else{
		  
	  }
	  
	  
    if (!isPlaying) {    	        
    	olsrdLogOn(2,param);
    	isPlaying=true;
    }
  }
  
  private void stop() throws IOException {
    if (isPlaying) {
    	
//        p1 = r.exec("ps | grep sh | awk '($1==\"u0_a49\" && $9==\"sh\") {print $2}");
//        lines = new BufferedReader(new InputStreamReader(p1.getInputStream()));
//	  	inputLine = lines.readLine();
//    	try {
//			lines.close();
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
    	olsrdLogOff(1);
    	//p.destroy();
    	isPlaying=false;
    }
  }
	@Override
	protected void onHandleIntent(Intent arg0) {
		// TODO Auto-generated method stub
		
	}


}
