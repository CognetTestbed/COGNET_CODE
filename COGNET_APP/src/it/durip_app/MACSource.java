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
//import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
//import java.net.ConnectException;
//import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
//import java.nio.ByteBuffer;
//import java.nio.channels.SocketChannel;
import java.util.Observable;
import java.util.Observer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.util.Log;

public class MACSource implements Runnable {
 
    // encapsulates management of the observers watching this datasource for update events:
    class MyObservable extends Observable {
	    @Override
	    public void notifyObservers() {
	        setChanged();
	        super.notifyObservers();
	        
	    }
    }
    
    public static final int MAC = 1;
    public static final int RSSi = 2;
    public static final int RXB = 3;
    public static final int RXDROP= 4;
    public static final int RXDUPL= 5;
    public static final int RXFRAG= 6;
    public static final int RXPACKS= 7;
    public static final int TXB = 8;
    public static final int TXFILT = 9;
    public static final int TXFRAG = 10;
    public static final int TXPACKS = 11;
    public static final int TXRETRYCOUNT = 12;
    public static final int TXRETRYF = 13;
    private Socket sc = null;
    public static final String KILL_REQUEST = "kill";
    private PrintWriter out = null;
    
  //MAC:00:1d:7d:49:2e:4e;RSSi:-77;RXB:21600;RXDROP:0;RXDUPL:0;RXFRAG:270;RXPACKS:540;TXB:0;TXFILT:0;TXFRAG:0;TXPACKS:0;TXRETRYCOUNT:0;TXRETRYF:0;INMS:60
    public static void getData(String outputMacAll){
    	
    	
    }
    
    Process p,p2;

    private static final int SAMPLE_SIZE_MAC = 30;
    float[][] ping_values_mac = new float[1+TXRETRYF][SAMPLE_SIZE_MAC+1];
    int ping_ok_mac = 0;
    private String pUrl,pMAC;
    private int pT,pI,pPort,pVerbose, pSleep;
    	//NO more void, need array!!
    private void ping(){
    	ping(pUrl);
    }
    private void ping(String paramUrl){
    	ping(paramUrl,pT,pI,pVerbose,pPort,pSleep);
    }
    public void setUrl(String paramUrl){
    	
        if(paramUrl.length() < 7 /*!IPAddressUtil.isIPv4LiteralAddress(paramUrl)*/)
        	pUrl = "192.168.1.90";
        else pUrl = paramUrl;
    }
    public void setMAC(String paramMAC){
    	Pattern dataPatternMAC = Pattern.compile("^.*([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2}).*$");
    	if (paramMAC.length()<15/*dataPatternMAC.matcher(paramMAC) != null*/)
        	pMAC = "";
        else 
        	pMAC = paramMAC;    		
    }
    public void setT(int paramT){
		if (paramT <= 0) pT = 200;
		else pT=paramT;
    }
    public void setI(int paramI){
		if (paramI <= 0) pI = 5;
		else pI=paramI;
    }
    public void setPort(int paramPort){
		if (paramPort<=1024 || paramPort >50000) pPort=4000;
		else pPort=paramPort;
    }
    public void setSleep(int paramSleep){
		if (paramSleep<200 || paramSleep >10000) pSleep=1000;
		else pSleep=paramSleep;
    }
    public void setVerbose(int paramVerbose){
		if (paramVerbose!=1 && paramVerbose!=0) pVerbose=1;
		else pVerbose = paramVerbose;
    }
    
    private static Thread myMACThread;
    private static boolean running =false;
    private static int iperfCount = 0;
    
    public boolean isRunning() {
    	return running;
    }

    public void startMAC() {
		if (!running){
	    	myMACThread = new Thread(this);
	    	running = true;
	    	myMACThread.start();
		}
    }

    public void stopMAC() {
		if(running){
//        	if (mac != null)
//        		mac.close();
			if (out != null){
	            out.println(KILL_REQUEST);
	        	out.close();
			}
			if (sc != null)
				try {
					sc.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
	      // Interrupt the thread so it unblocks any blocking call
	    	if (myMACThread != null )myMACThread.interrupt();
	      // Change the states of variable
	    	running = false;
	    	myMACThread = null;
		}
    }
    
    public void addIperf() {
    	iperfCount +=1;
    	
    }

	private void ping(String paramUrl, int paramT, int paramI, int paramVerbose, int paramPort, int paramSleep) {
		//String result="";
		iperfCount = 0;
		//TODO parametric phyx --> the last one
        String inputLine = null,
        		//ServerCmd = "sh /sdcard/SCRIPT_TABLET/mac.sh 1 "+paramSleep+" "+paramPort+" phy1 wlan1", ////+paramVerbose+":"+(paramPort/1000), 
        		ServerCmd = "macREADDURIP 1:"+paramSleep+" "+ paramPort +" phy1 wlan1", ////+paramVerbose+":"+(paramPort/1000), 
        		IperfCmd = "iperf -c "+ paramUrl +" -t "+paramT+" -i "+paramI;//"ping -s 3000 -c 1 -l 1 " + url;
        		//useless ex style..keep it until socket system is working
        Runtime r = Runtime.getRuntime();
        //Runtime r2 = Runtime.getRuntime();
        
        try {
			p = r.exec(ServerCmd);
	        //p2 = r.exec(IperfCmd);	
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
        Pattern dataPatternMAC;
        
        //MAC:00:1d:7d:49:2e:4e;RSSi:-77;RXB:21600;RXDROP:0;RXDUPL:0;RXFRAG:270;RXPACKS:540;TXB:0;TXFILT:0;TXFRAG:0;TXPACKS:0;TXRETRYCOUNT:0;TXRETRYF:0;INMS:60
        //if (pMAC.length() > 1){
        	dataPatternMAC = Pattern.compile("^.*MAC:()()("+ pMAC +"):RSSi:(-?\\d+):RXB:(\\d+):RXDROP:(\\d+):RXDUPL:(\\d+):RXFRAG:(\\d+):RXPACKS:(\\d+):TXB:(\\d+):TXFILT:(\\d+):TXFRAG:(\\d+):TXPACKS:(\\d+):TXRETRYCOUNT:(\\d+):TXRETRYF:(\\d+):INMS:(\\d+).*$");
        //}else{
        //	dataPatternMAC = Pattern.compile("^.*MAC:(([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2}));RSSi:(-?\\d+);RXB:(\\d+);RXDROP:(\\d+);RXDUPL:(\\d+);RXFRAG:(\\d+);RXPACKS:(\\d+);TXB:(\\d+);TXFILT:(\\d+);TXFRAG:(\\d+);TXPACKS:(\\d+);TXRETRYCOUNT:(\\d+);TXRETRYF:(\\d+);INMS:(\\d+).*$");	
        //}
        
        //Pattern for averages --> keep for future improvements
        //Pattern dataPatternMAC2 = Pattern.compile("^.*time:(\\d+);RXPACK:(\\d+);RXB:(\\d+);TXPACK:(\\d+);TXB:(\\d+);COLLISION:(\\d+).*$");
        BufferedReader mac = null;
        //System.out.println(p.exitValue());
        Matcher datas_mac;
        Thread currentThread = Thread.currentThread();
        //int ki = 0;
    	float[] ping_values_tmp_mac;
        //BufferedReader maccheccazz = new BufferedReader(new InputStreamReader(p.getErrorStream()));


        try {
            sc = new Socket("127.0.0.1", (paramPort+2));
            out = new PrintWriter(sc.getOutputStream(), true);
            mac = new BufferedReader(new InputStreamReader(
                                        sc.getInputStream()));
        } catch (UnknownHostException e) {
            System.err.println("Don't know about host");
            System.exit(1);
        } catch (IOException e) {
            System.err.println("Couldn't get I/O for "
                               + "the connection to");
            System.exit(1);
        }
        //send to socket the selected MAC address
        out.println(pMAC);
        //out.write(pMAC+"\n");
        //mac = new BufferedReader(new InputStreamReader(p.getInputStream()));
        //mac = new BufferedReader(new InputStreamReader(sc.getInputStream()));
        Log.i("MAC KAROLY", "PRE WHILE MAC " + running);
        while (currentThread == myMACThread  && running) {
        	// launch IPREF
        	//System.out.println("DVC " + ki++);
	        try {
	        	Log.i("MAC KAROLY", "INSIDE WHILE MAC " + running);
	        	/*
	        	System.out.println(p.exitValue());
	        	
				while ((inputLine = maccheccazz.readLine()) != null) {
					if (currentThread != myMACThread) break;
				    System.out.println(inputLine);
				}
				*/
//	        	int c;
				while ((inputLine = mac.readLine()) != null) { //TODO change
//	        	while ((c = mac.read()) != 13) { //TODO change
//					Log.i("MAC KAROLY","AA" );
					if (currentThread != myMACThread) 
						break;
					
					datas_mac = dataPatternMAC.matcher(inputLine);
				    if (datas_mac.find( )) {
						Log.i("MAC", "from terminal "+inputLine);
				    	if (ping_ok_mac > SAMPLE_SIZE_MAC){
				        	// rotation and override last position if graph is filled
				            // TODO FIX  
			        		for (int i = 1 ; i < 1+TXRETRYF; i++){
			    		        ping_values_tmp_mac = new float[SAMPLE_SIZE_MAC+1];
			    		        System.arraycopy(ping_values_mac[i], 1, ping_values_tmp_mac, 0, SAMPLE_SIZE_MAC);
			    		        ping_values_mac[i] = ping_values_tmp_mac;
			        		}
				    		
				    		for (int i = 1 ; i < 1+TXRETRYF; i++){
				    			if (i==1)ping_values_mac[i][SAMPLE_SIZE_MAC] = Float.parseFloat(macTofloat(datas_mac.group(i)));
				    			else ping_values_mac[i][SAMPLE_SIZE_MAC] = Float.parseFloat(datas_mac.group(i+2));
				    		}
				    	}else{
				        	//fill the graph in the beginning
				    		for (int i = 1 ; i < 1+TXRETRYF; i++){
				    			if (i==1)ping_values_mac[i][ping_ok_mac] = Float.parseFloat(macTofloat(datas_mac.group(i)));
				    			else ping_values_mac[i][ping_ok_mac] = Float.parseFloat(datas_mac.group(i+2));
				    		}
				    	}
				    	ping_ok_mac++;
				    	notifier.notifyObservers();
				    	//System.out.println("OK MAC" + inputLine);
				    	//t.append("\nicmp_seq: " + datas.group(1) + " ttl: " + datas.group(2) + " time: " + datas.group(3));
					    
					    try{
					        Thread.sleep(paramSleep);
					    } catch (Exception e) {
					        e.printStackTrace();
					    }
				    
				    }
				}//while
				Log.i("MAC KAROLY", "no exceptions");
			} catch (NumberFormatException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	        
            if (Thread.currentThread().isInterrupted()){
            	if (mac != null)
					try {
						mac.close();
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
    			if (out != null){
    	            out.println(KILL_REQUEST);
    	        	out.close();
    			}
    			if (sc != null)
    				try {
    					sc.close();
    				} catch (IOException e) {
    					// TODO Auto-generated catch block
    					e.printStackTrace();
    				}
 
            	break;
            }
        }
        try {
        	if (mac != null)
        		mac.close();
			if (out != null){
	            out.println(KILL_REQUEST);
	        	out.close();
			}
			if (sc != null)
				try {
					sc.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			//p2 = r.exec("clientHandleMac 127.0.0.1 "+pPort+" 0");	
        	//p.destroy();  
        	//p2.destroy();   
		} catch (IOException e) {
			// TODO Auto-generated catch block
			//e.printStackTrace();
		}
	}
    private String macTofloat(String macAddr) {
    	int s = 0;
    	for (char ch: macAddr.toCharArray()) {
    		if (ch != ':')
    			s += (1 + ch);
    	}
		return "" + s;
	}

	private MyObservable notifier;
 
    {
        notifier = new MyObservable();
    }
 
    //@Override
    public void run() {
        try {
        	ping();/*
            while (true) {
                Thread.sleep(50); // refresh rate slowdown --> human readable
                //notifier.notifyObservers();
                if (Thread.currentThread().isInterrupted()){
                	p.destroy();  
                	p2.destroy();  
                	break;
                }
            }*/
        	try {
        		Runtime r = Runtime.getRuntime();
				p2 = r.exec("clientHandleMac 127.0.0.1 "+pPort+" 0");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}	
        	p.destroy();  
        	p2.destroy(); 
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
 
    public int getItemCount(int series) {
		return SAMPLE_SIZE_MAC;
    }
 
    public Number getX(int series, int index) {
        if (index >= SAMPLE_SIZE_MAC) {
            throw new IllegalArgumentException();
        }
        return index;
		
    }
 
    public Number getY(int series, int index) {
         if (index >= SAMPLE_SIZE_MAC){
             throw new IllegalArgumentException();
         }
         /*
         if(series >= 0 && series <=10){
         	if (ping_ok==0) return ping_values[series][ping_ok];
         	if (ping_ok<SAMPLE_SIZE && index > ping_ok) return ping_values[series][ping_ok-1];
         	else return ping_values[series][index];
         }else
        	 throw new IllegalArgumentException();
         */
         //return 1;
        //if (series <= PACKETRET){
         switch (series) {

	            case MAC:
	            	String macAddress = "AA:BB:CC:DD:EE:FF";
	            	String[] macAddressParts = macAddress.split(":");

	            	// convert hex string to byte values
	            	Byte[] macAddressBytes = new Byte[6];
	            	for(int i=0; i<6; i++){
	            	    Integer hex = Integer.parseInt(macAddressParts[i], 16);
	            	    macAddressBytes[i] = hex.byteValue();
	            	}
	            	/*
	            	if (ping_ok_mac==0) return ping_mac[ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RSSi-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RSSi-PACKETRET][index];
	            	*/
	            case RSSi:
	            	if (ping_ok_mac==0) return ping_values_mac[RSSi][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RSSi][ping_ok_mac];
	            	else return ping_values_mac[RSSi][index];
	            case RXB:
	            	if (ping_ok_mac==0) return ping_values_mac[RXB][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXB][ping_ok_mac];
	            	else return ping_values_mac[RXB][index];
	            case RXDROP:
	            	if (ping_ok_mac==0) return ping_values_mac[RXDROP][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXDROP][ping_ok_mac];
	            	else return ping_values_mac[RXDROP][index];
	            case RXDUPL:
	            	if (ping_ok_mac==0) return ping_values_mac[RXDUPL][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXDUPL][ping_ok_mac];
	            	else return ping_values_mac[RXDUPL][index];
	            case RXFRAG:
	            	if (ping_ok_mac==0) return ping_values_mac[RXFRAG][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXFRAG][ping_ok_mac];
	            	else return ping_values_mac[RXFRAG][index];
	            case RXPACKS:
	            	if (ping_ok_mac==0) return ping_values_mac[RXPACKS][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXPACKS][ping_ok_mac];
	            	else return ping_values_mac[RXPACKS][index];
	            case TXB:
	            	if (ping_ok_mac==0) return ping_values_mac[TXB][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXB][ping_ok_mac];
	            	else return ping_values_mac[TXB][index];
	            case TXFILT:
	            	if (ping_ok_mac==0) return ping_values_mac[TXFILT][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXFILT][ping_ok_mac];
	            	else return ping_values_mac[TXFILT][index];
	            case TXFRAG:
	            	if (ping_ok_mac==0) return ping_values_mac[TXFRAG][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXFRAG][ping_ok_mac];
	            	else return ping_values_mac[TXFRAG][index];
	            case TXPACKS:
	            	if (ping_ok_mac==0) return ping_values_mac[TXPACKS][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXPACKS][ping_ok_mac];
	            	else return ping_values_mac[TXPACKS][index];
	            case TXRETRYCOUNT:
	            	if (ping_ok_mac==0) return ping_values_mac[TXRETRYCOUNT][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXRETRYCOUNT][ping_ok_mac];
	            	else return ping_values_mac[TXRETRYCOUNT][index];
	            case TXRETRYF:
	            	if (ping_ok_mac==0) return ping_values_mac[TXRETRYF][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXRETRYF][ping_ok_mac];
	            	else return ping_values_mac[TXRETRYF][index];
	            default:
	                throw new IllegalArgumentException();
	        }
	        /*
        }else{
        	
        	series -=10;
	        switch (series) {
	            case CWN:
	            	if (ping_ok_tcp==0) return ping_values_tcp[CWN][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[CWN][ping_ok_tcp-1];
	            	else return ping_values_tcp[CWN][index];
	            default:
            		throw new IllegalArgumentException();
	        }
        	
        }
        */
    }
 
    public void addObserver(Observer observer) {
        notifier.addObserver(observer);
    }
 
    public void removeObserver(Observer observer) {
        notifier.deleteObserver(observer);
    }

 
}
