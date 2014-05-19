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
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Observable;
import java.util.Observer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.util.Log;

public class DataSource implements Runnable {
 
    // encapsulates management of the observers watching this datasource for update events:
    class MyObservable extends Observable {
	    @Override
	    public void notifyObservers() {
	        setChanged();
	        super.notifyObservers();
	        
	    }
    }
    //1  CWN:0000000187;
    //2  AW:0000014656;
    //3  CWASP:0000773248;
    //4  SSHTHRESH:139;
    //5  RTT:190000;
    //6  SRTT:36;
    //7  FLIGHT:1;
    //8  PACKETOUT:0;
    //9  PACKETLOST:0;
    //10 PACKETRET:0
    
    public static final int CWN = 1;
    public static final int AW = 2;
    public static final int CWASP = 3;
    public static final int SSHTHRESH = 4;
    public static final int RTT = 5;
    public static final int SRTT = 6;
    public static final int FLIGHT = 7;
    public static final int PACKETOUT = 8;
    public static final int PACKETLOST = 9;
    public static final int PACKETRET = 10;
    
    public static final int MAC = 11;
    public static final int RSSi = 12;
    public static final int RXB = 13;
    public static final int RXDROP= 14;
    public static final int RXDUPL= 15;
    public static final int RXFRAG= 16;
    public static final int RXPACKS= 17;
    public static final int TXB = 18;
    public static final int TXFILT = 19;
    public static final int TXFRAG = 20;
    public static final int TXPACKS = 21;
    public static final int TXRETRYCOUNT = 22;
    public static final int TXRETRYF = 23;
    
  //MAC:00:1d:7d:49:2e:4e;RSSi:-77;RXB:21600;RXDROP:0;RXDUPL:0;RXFRAG:270;RXPACKS:540;TXB:0;TXFILT:0;TXFRAG:0;TXPACKS:0;TXRETRYCOUNT:0;TXRETRYF:0;INMS:60
    
    
    Process p,p2;

    private static final int SAMPLE_SIZE_MAC = 30;
    float[][] ping_values_mac = new float[1+TXRETRYF-PACKETRET][SAMPLE_SIZE_MAC+1];
    int ping_ok_mac = 0;
    private static final int SAMPLE_SIZE_TCP = 300;
    float[][] ping_values_tcp = new float[PACKETRET+1][SAMPLE_SIZE_TCP+1];
    private int ping_ok_tcp = 0;
    private String pUrl;
    private int pT,pI,pPort,pVerbose;
    	//NO more void, need array!!
    private void ping(){
    	ping(pUrl);
    }
    private void ping(String paramUrl){
    	ping(paramUrl,pT,pI,pVerbose,pPort);
    }
    private void ping(String paramUrl, int T){
    	ping(paramUrl,T,pI,pVerbose,pPort);
    }
    private void ping(String paramUrl, int T, int I){
    	ping(paramUrl,T,I,pVerbose,pPort);
    }
    public void setUrl(String paramUrl){
    	
        if(paramUrl.length() < 7 /*!IPAddressUtil.isIPv4LiteralAddress(paramUrl)*/)
        	pUrl = "192.168.1.90";
        else pUrl = paramUrl;
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
    public void setVerbose(int paramVerbose){
		if (paramVerbose!=1 && paramVerbose!=0) pVerbose=1;
		else pVerbose = paramVerbose;
    }
	private void ping(String paramUrl, int paramT, int paramI, int paramVerbose, int paramPort) {
		//String result="";

	    try {
	        String inputLine = null,ServerCmd = "serverSYNCDURIP "+paramPort+" "+paramVerbose, IperfCmd = "iperf -c "+ paramUrl +" -t "+paramT+" -i "+paramI;//"ping -s 3000 -c 1 -l 1 " + url;
	        
	        //left shift of the values --> can become more powerful with incremental history
	        // TODO handle different zoom: 5 30 300 1000
	        if (ping_ok_tcp > SAMPLE_SIZE_TCP){
	        	float[] ping_values_tmp_tcp;
        		for (int i = 1 ; i <= PACKETRET; i++){
    		        ping_values_tmp_tcp = new float[SAMPLE_SIZE_TCP+1];
    		        System.arraycopy(ping_values_tcp[i], 1, ping_values_tmp_tcp, i, ping_values_tcp[i].length - 1);
    		        ping_values_tcp[i] = ping_values_tmp_tcp;
        		}
	        }
	        
	        if (ping_ok_mac > SAMPLE_SIZE_MAC){
	        	float[] ping_values_tmp_mac;
        		for (int i = 1 ; i <= 1+TXRETRYF-PACKETRET; i++){
    		        ping_values_tmp_mac = new float[SAMPLE_SIZE_MAC+1];
    		        System.arraycopy(ping_values_mac[i], 1, ping_values_tmp_mac, i, ping_values_mac[i].length - 1);
    		        ping_values_mac[i] = ping_values_tmp_mac;
        		}
	        }
	        
	        Runtime ru = Runtime.getRuntime();
	        Process op = ru.exec("su");
	        Runtime r = Runtime.getRuntime();
	        p = r.exec(ServerCmd);
	        Runtime r2 = Runtime.getRuntime();
	        p2 = r2.exec(IperfCmd);	
	        // Create a Pattern object
	        Pattern dataPatternTCP = Pattern.compile("^.*CWN:(\\d+);AW:(\\d+);CWASP:(\\d+);SSHTHRESH:(\\d+);RTT:(\\d+);SRTT:(\\d+);FLIGHT:(\\d+);PACKETOUT:(\\d+);PACKETLOST:(\\d+);PACKETRET:(\\d+).*$");
	        //TODO define pattern for MAC
	        //MAC:00:1d:7d:49:2e:4e;RSSi:-77;RXB:21600;RXDROP:0;RXDUPL:0;RXFRAG:270;RXPACKS:540;TXB:0;TXFILT:0;TXFRAG:0;TXPACKS:0;TXRETRYCOUNT:0;TXRETRYF:0;INMS:60
	        Pattern dataPatternMAC = Pattern.compile("^.*MAC:([0-9A-F]{2}[:-]){5}([0-9A-F]{2});RSSi:(-?\\d+);RXB:(\\d+);RXDROP:(\\d+);RXDUPL:(\\d+);RXFRAG:(\\d+);RXPACKS:(\\d+);TXB:(\\d+);TXFILT:(\\d+);TXFRAG:(\\d+);TXPACKS:(\\d+);TXRETRYCOUNT:(\\d+);TXRETRYF:(\\d+);INMS:(\\d+)");
	        Pattern dataPatternMAC2 = Pattern.compile("^.*time:(\\d+);RXPACK:(\\d+);RXB:(\\d+);TXPACK:(\\d+);TXB:(\\d+);COLLISION:(\\d+).*$");
	        BufferedReader mac, tcp;
	        //System.out.println(p.exitValue());
	        Matcher datas_mac, datas_tcp;
            while (true) {
            	mac = new BufferedReader(new InputStreamReader(p.getErrorStream()));
		        while ((inputLine = mac.readLine()) != null) {
			        System.out.println(inputLine);
		        }
            	// launch IPREF
            	mac = new BufferedReader(new InputStreamReader(p.getInputStream()));

		        while ((inputLine = mac.readLine()) != null) {
			        System.out.println(inputLine);
		        	datas_mac = dataPatternMAC.matcher(inputLine);
			        if (datas_mac.find( )) {
			    		Log.i("MAC", "from terminal "+inputLine);
			        	if (ping_ok_mac > SAMPLE_SIZE_MAC){
				        	// rotation and override last position if graph is filled
			        		for (int i = 1 ; i < 1+TXRETRYF-PACKETRET; i++){
			        			ping_values_mac[i][ping_values_mac[i].length - 1] = Float.parseFloat(datas_mac.group(i));
			        			//System.out.print(" * " + i+"=" + ping_values_mac[i][ping_values_mac[i].length - 1]);
			        		}
			        	}else{
				        	//fill the graph in the beginning
			        		for (int i = 1 ; i < 1+TXRETRYF-PACKETRET; i++){
			        			ping_values_mac[i][ping_ok_mac] = Float.parseFloat(datas_mac.group(i));
			        			//System.out.print(" * " + i+"=" + ping_values_mac[i][ping_ok_mac]);
			        		}
			        	}
			        	ping_ok_mac++;
			        	notifier.notifyObservers();
			        	//System.out.println("YEAH" + inputLine);
			        	//t.append("\nicmp_seq: " + datas.group(1) + " ttl: " + datas.group(2) + " time: " + datas.group(3));
			        }
			        Log.i("MAC",inputLine);
			       /* 
	    	        //tcp = new BufferedReader(new FileReader("/proc/tcp_input_durip"));
		        	//for (int k=0;k<10 ;k++){
		    	        //tcp = new BufferedReader(new InputStreamReader(Runtime.getRuntime().exec("cat /proc/tcp_input_durip").getInputStream()));
				        //if ((inputLine=mac.readLine()) != null){
					        datas_tcp = dataPatternTCP.matcher(inputLine);
					        if (datas_tcp.find( )) {
					    		Log.i("TCP", "From file " + inputLine);
					        	if (ping_ok_tcp > SAMPLE_SIZE_TCP){
						        	// rotation and override last position if graph is filled
					        		for (int i = 1 ; i < PACKETRET; i++){
					        			ping_values_tcp[i][ping_values_tcp[i].length - 1] = Float.parseFloat(datas_tcp.group(i));
					        			//System.out.print(" * " + i+"=" + ping_values_tcp[i][ping_values_tcp[i].length - 1]);
					        		}
					        	}else{
						        	//fill the graph in the beginning
					        		for (int i = 1 ; i < PACKETRET; i++){
					        			ping_values_tcp[i][ping_ok_tcp] = Float.parseFloat(datas_tcp.group(i));
					        			//System.out.print(" * " + i+"=" + ping_values_tcp[i][ping_ok_tcp]);
					        		}
					        	}
					        	System.out.println(inputLine+ "daa");
					        	ping_ok_tcp++;
					        	notifier.notifyObservers();
					        	//System.out.println("YEAH" + inputLine);
					        	//t.append("\nicmp_seq: " + datas.group(1) + " ttl: " + datas.group(2) + " time: " + datas.group(3));
					        }
				            try{
					            Thread.sleep(50);
					        } catch (Exception e) {
					            e.printStackTrace();
					        }
				            
			    	        //tcp = new BufferedReader(new FileReader("/proc/tcp_input_durip"));
				        	for (int k=0;k<10 ;k++){
				    	        tcp = new BufferedReader(new InputStreamReader(Runtime.getRuntime().exec("cat /proc/tcp_input_durip").getInputStream()));
						        if ((inputLine=tcp.readLine()) != null){
						        datas_tcp = dataPatternTCP.matcher(inputLine);
						        if (datas_tcp.find( )) {
						    		Log.i("TCP", "From file " + inputLine);
						        	if (ping_ok_tcp > SAMPLE_SIZE_TCP){
							        	// rotation and override last position if graph is filled
						        		for (int i = 1 ; i < PACKETRET; i++){
						        			ping_values_tcp[i][ping_values_tcp[i].length - 1] = Float.parseFloat(datas_tcp.group(i));
						        			//System.out.print(" * " + i+"=" + ping_values_tcp[i][ping_values_tcp[i].length - 1]);
						        		}
						        	}else{
							        	//fill the graph in the beginning
						        		for (int i = 1 ; i < PACKETRET; i++){
						        			ping_values_tcp[i][ping_ok_tcp] = Float.parseFloat(datas_tcp.group(i));
						        			//System.out.print(" * " + i+"=" + ping_values_tcp[i][ping_ok_tcp]);
						        		}
						        	}
						        	System.out.println(inputLine+ "daa");
						        	ping_ok_tcp++;
						        	notifier.notifyObservers();
						        	//System.out.println("YEAH" + inputLine);
						        	//t.append("\nicmp_seq: " + datas.group(1) + " ttl: " + datas.group(2) + " time: " + datas.group(3));
						        }
					            try{
						            Thread.sleep(1000);
						        } catch (Exception e) {
						            e.printStackTrace();
						        }

						        }
						        tcp.close();
					        }

*/
	                //Thread.sleep(600); // refresh rate slowdown --> human readable
	                //notifier.notifyObservers();
	                if (Thread.currentThread().isInterrupted()){
	                	//p.destroy();  
	                	p2.destroy();  
	                	break;
	                }
		        }
		        //mac.close();
		        
                if (Thread.currentThread().isInterrupted()){
                	//p.destroy();  
                	p2.destroy();  
                	break;
                }
            }
        }catch (IOException e) {
        	System.out.println(e.toString());
        }
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
        	//p.destroy();  
        	p2.destroy(); 
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
 
    public int getItemCount(int series) {
		if (series <= PACKETRET)
			return SAMPLE_SIZE_TCP;
		else
			return SAMPLE_SIZE_MAC;
    }
 
    public Number getX(int series, int index) {
		if (series <= PACKETRET){
	        if (index >= SAMPLE_SIZE_TCP) {
	            throw new IllegalArgumentException();
	        }
	        return index;
		}else{
	        if (index >= SAMPLE_SIZE_MAC) {
	            throw new IllegalArgumentException();
	        }
	        return index;
		}
    }
 
    public Number getY(int series, int index) {
         if ((series <= PACKETRET && index >= SAMPLE_SIZE_TCP) || (series > PACKETRET && index >= SAMPLE_SIZE_MAC) ){
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
	            case CWN:
	            	if (ping_ok_tcp==0) return ping_values_tcp[CWN][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[CWN][ping_ok_tcp-1];
	            	else return ping_values_tcp[CWN][index];
	            case AW:
	            	if (ping_ok_tcp==0) return ping_values_tcp[AW][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[AW][ping_ok_tcp-1];
	            	else return ping_values_tcp[AW][index];
	            case CWASP:
	            	if (ping_ok_tcp==0) return ping_values_tcp[CWASP][ping_ok_tcp];///10000;
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[CWASP][ping_ok_tcp-1];///10000;
	            	else return ping_values_tcp[CWASP][index];///10000;
	            case SSHTHRESH:
	            	if (ping_ok_tcp==0) return ping_values_tcp[SSHTHRESH][ping_ok_tcp];// > 1000 ? 0 : ping_values_tcp[SSHTHRESH][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[SSHTHRESH][ping_ok_tcp-1];// > 1000 ? 0 : ping_values_tcp[4][ping_ok_tcp-1];
	            	else return ping_values_tcp[SSHTHRESH][index];// > 1000 ? 0 : ping_values_tcp[SSHTHRESH][index];
	            case RTT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[RTT][ping_ok_tcp];///10000;
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[RTT][ping_ok_tcp-1];///10000;
	            	else return ping_values_tcp[RTT][index];///10000;
	            case SRTT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[SRTT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[SRTT][ping_ok_tcp-1];
	            	else return ping_values_tcp[SRTT][index];
	            case FLIGHT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[FLIGHT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[FLIGHT][ping_ok_tcp-1];
	            	else return ping_values_tcp[FLIGHT][index];
	            case PACKETOUT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PACKETOUT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PACKETOUT][ping_ok_tcp-1];
	            	else return ping_values_tcp[PACKETOUT][index];
	            case PACKETLOST:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PACKETLOST][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PACKETLOST][ping_ok_tcp-1];
	            	else return ping_values_tcp[PACKETLOST][index];
	            case PACKETRET:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PACKETRET][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PACKETRET][ping_ok_tcp-1];
	            	else return ping_values_tcp[PACKETRET][index];
	            	
	      /*
	       * 
	            public static final int RSSi = 12;
	            public static final int RXB = 13;
				public static final int RXDROP= 14;
				public static final int RXDUPL= 15;
				public static final int RXFRAG= 16;
				public static final int RXPACKS= 17;
				public static final int TXB = 18;
				public static final int TXFILT = 19;
				public static final int TXFRAG = 21;
				public static final int TXPACKS = 21;
				public static final int TXRETRYCOUNT = 22;
				public static final int TXRETRYF = 23;
	       * */   
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
	            	if (ping_ok_mac==0) return ping_values_mac[RSSi-PACKETRET][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RSSi-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RSSi-PACKETRET][index];
	            case RXB:
	            	if (ping_ok_mac==0) return ping_values_tcp[RXB-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXB-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RXB-PACKETRET][index];
	            case RXDROP:
	            	if (ping_ok_mac==0) return ping_values_tcp[RXDROP-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXDROP-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RXDROP-PACKETRET][index];
	            case RXDUPL:
	            	if (ping_ok_mac==0) return ping_values_tcp[RXDUPL-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXDUPL-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RXDUPL-PACKETRET][index];
	            case RXFRAG:
	            	if (ping_ok_mac==0) return ping_values_tcp[RXFRAG-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXFRAG-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RXFRAG][index];
	            case RXPACKS:
	            	if (ping_ok_mac==0) return ping_values_tcp[RXPACKS-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[RXPACKS-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[RXPACKS-PACKETRET][index];
	            case TXB:
	            	if (ping_ok_mac==0) return ping_values_mac[TXB-PACKETRET][ping_ok_mac];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXB-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[TXB-PACKETRET][index];
	            case TXFILT:
	            	if (ping_ok_mac==0) return ping_values_tcp[TXFILT-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXFILT-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[TXFILT-PACKETRET][index];
	            case TXFRAG:
	            	if (ping_ok_mac==0) return ping_values_tcp[TXFRAG-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXFRAG-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[TXFRAG-PACKETRET][index];
	            case TXPACKS:
	            	if (ping_ok_mac==0) return ping_values_tcp[TXPACKS-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXPACKS-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[TXPACKS-PACKETRET][index];
	            case TXRETRYCOUNT:
	            	if (ping_ok_mac==0) return ping_values_tcp[TXRETRYCOUNT-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXRETRYCOUNT-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[TXRETRYCOUNT-PACKETRET][index];
	            case TXRETRYF:
	            	if (ping_ok_mac==0) return ping_values_tcp[TXRETRYF-PACKETRET][ping_ok_tcp];
	            	if (ping_ok_mac<SAMPLE_SIZE_MAC && index > ping_ok_mac) return ping_values_mac[TXRETRYF-PACKETRET][ping_ok_mac-1];
	            	else return ping_values_mac[TXRETRYF-PACKETRET][index];
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
