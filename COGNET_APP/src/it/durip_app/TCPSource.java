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

public class TCPSource implements Runnable {
 
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
    //tcp_output_durip
    //tcp_event_durip
    public static final int CWND = 1;
    public static final int AW = 2;
    public static final int SSHTHRESH = 3;
    public static final int RTT = 4;
    public static final int SRTT = 5;
    public static final int SRTTJTU = 6;
    public static final int RTO = 7;
    public static final int FLIGHT = 8;
    public static final int PACKETOUT = 9;
    public static final int PACKETLOST = 10;
    public static final int PACKETRET = 11;
    public static final int PRRDEV = 12;
    public static final int PRROUT = 13;
    public static final int TOTRET = 14;
    public static final int BYTESACK = 15;
    public static final int PKTACKED = 16;
    public static final int MSS = 17;
   
    Process p,p2;

    private static final int SAMPLE_SIZE_TCP = 30;
    float[][] ping_values_tcp = new float[MSS+1][SAMPLE_SIZE_TCP+1];
    private int ping_ok_tcp = 0;
    private String pUrl;
    private int pT,pI,pPort,pVerbose,pSleep;
    	//NO more void, need array!!
    private void ping(){
    	ping(pUrl);
    }
    private void ping(String paramUrl){
    	ping(paramUrl,pT,pI,pVerbose,pPort,pSleep);
    }
    public void setUrl(String paramUrl){
    	
        if(paramUrl.length() < 7 /*!IPAddressUtil.isIPv4LiteralAddress(paramUrl)*/)
        	pUrl = "192.168.1.120";
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
		if (paramPort>=1024 || paramPort <50) pPort=500;
		else pPort=paramPort;
    }
    public void setVerbose(int paramVerbose){
		if (paramVerbose!=1 && paramVerbose!=0) pVerbose=1;
		else pVerbose = paramVerbose;
    }
    public void setSleep(int paramSleep){
		if (paramSleep<200 && paramSleep>10000) pSleep=1000;
		else pSleep = paramSleep;
    }
    private static Thread myTCPThread;
    private static boolean running =false;
    private static int iperfCount = 0;
    public boolean isRunning() {
    	return running;
    }
    public void startTCP() {
		if (!running){
	    	running = true;
	    	myTCPThread = new Thread(this);
	    	myTCPThread.start();
		}

    }

    public void stopTCP() {
		if(running){
	    	running = false;
	        // Interrupt the thread so it unblocks any blocking call
	    	if (myTCPThread != null )myTCPThread.interrupt();
	        // Change the states of variable
	    	myTCPThread = null;
		}
    }
    
    public void addIperf() {
    	iperfCount +=1;
    	
    }

	private void ping(String paramUrl, int paramT, int paramI, int paramVerbose, int paramPort, int paramSleep) {
		//String result="";
		iperfCount = 0;
        String inputLine = null,
        		//ServerCmd = "tcpREADDURIP "+paramVerbose+":"+(paramPort/1000), 
//        		ServerCmd = "sh /sdcard/SCRIPT_TABLET/tcpOutput.sh " +((float)paramSleep/1000),
        		ServerCmd = "sh /sdcard/COGNET_TESTBED/SCRIPT/tcpOutput.sh " +((float)paramSleep/1000),        				 
        		IperfCmd = "iperf -c "+ paramUrl +" -t "+paramT+" -i "+paramI;//"ping -s 3000 -c 1 -l 1 " + url;
        // "/sdcard/SCRIPT/tcpOutput.sh" + (paramPort/1000)
        //left shift of the values --> can become more powerful with incremental history
        
        Runtime r = Runtime.getRuntime();
        
        // Create a Pattern object
        Pattern dataPatternTCP = Pattern.compile("^.*CWND:(\\d+):AW:(\\d+):SSHTHRESH:(\\d+):RTT:(\\d+):SRTT:(\\d+):SRTTJTU:(\\d+):RTO:(\\d+):FLIGHT:(\\d+):PACKETOUT:(\\d+):PACKETLOST:(\\d+):PACKETRET:(\\d+):PRRDEV:(\\d+):PRROUT:(\\d+):TOTRET:(\\d+):BYTESACK:(\\d+):PKTACKED:(\\d+):MSS:(\\d+).*$");
        BufferedReader tcp;
        //System.out.println(p.exitValue());
        Matcher datas_tcp;
        Thread currentThread = Thread.currentThread();
    	float[] ping_values_tmp_tcp;
        try {
			p = r.exec(ServerCmd);
	        //p2 = r.exec(IperfCmd);	
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
    	// launch IPREF
    	tcp = new BufferedReader(new InputStreamReader(p.getInputStream()));
    	float tmp = 0;
        while (currentThread == myTCPThread && !currentThread.isInterrupted()) {

        	//tcp = new BufferedReader(new FileReader("/proc/tcp_input_durip"));
        	//tcp = new BufferedReader(new InputStreamReader(Runtime.getRuntime().exec("cat /proc/tcp_input_durip").getInputStream()));
	        try {
				while ((inputLine = tcp.readLine()) != null) {
				    //System.out.println(inputLine);
					datas_tcp = dataPatternTCP.matcher(inputLine);
				    if (datas_tcp.find( )) {
						Log.i("TCP", "From file " + inputLine);
				    	if (ping_ok_tcp > SAMPLE_SIZE_TCP){
				        	// rotation and override last position if graph is filled
				    		for (int i = 1 ; i <= MSS; i++){
						        ping_values_tmp_tcp = new float[SAMPLE_SIZE_TCP+1];
						        System.arraycopy(ping_values_tcp[i], 1, ping_values_tmp_tcp, 0, SAMPLE_SIZE_TCP);
						        ping_values_tcp[i] = ping_values_tmp_tcp;
				    		}
				    		for (int i = 1 ; i < MSS; i++){
				    			tmp = Float.parseFloat(datas_tcp.group(i));
				    			if (i==SSHTHRESH && tmp > 1000) tmp = 0;
				    			ping_values_tcp[i][SAMPLE_SIZE_TCP] = tmp;
				    			System.out.print(" * " + i+"=" + ping_values_tcp[i][ping_values_tcp[i].length - 1]);
				    		}
				    	}else{
				        	//fill the graph in the beginning
				    		for (int i = 1 ; i < MSS; i++){
				    			tmp = Float.parseFloat(datas_tcp.group(i));
				    			if (i==SSHTHRESH && tmp > 1000) tmp = 0;
				    			ping_values_tcp[i][ping_ok_tcp] = tmp;
				    			//System.out.print(" * " + i+"=" + ping_values_tcp[i][ping_ok_tcp]);
				    		}
				    	}
				    	
				    	ping_ok_tcp++;
				    	notifier.notifyObservers();
						if (currentThread != myTCPThread) break;
				    	//System.out.println("OK TCP" + inputLine);
				    	//t.append("\nicmp_seq: " + datas.group(1) + " ttl: " + datas.group(2) + " time: " + datas.group(3));
				    }
				    try{
				        Thread.sleep(paramPort);
				    } catch (Exception e) {
				        e.printStackTrace();
				    }
			    
				}
			} catch (NumberFormatException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
				p.destroy(); 
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
				p.destroy(); 
			}finally{
				//p.destroy();
				//p2.destroy();
				
			}
            if (Thread.currentThread().isInterrupted()){
    	        try {
    				tcp.close();
    			} catch (IOException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
            	p.destroy();  
            	//p2.destroy();  
            	break;
            }
        }
        try {
			tcp.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	p.destroy();  
    	//p2.destroy();  

	}
    private MyObservable notifier;
 
    {
        notifier = new MyObservable();
    }
 
    //@Override
    public void run() {
        try {
        	ping();
        	p.destroy();  
        	//p2.destroy(); 
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
 
    public int getItemCount(int series) {
		return SAMPLE_SIZE_TCP;
    }
 
    public Number getX(int series, int index) {
        if (index >= SAMPLE_SIZE_TCP) {
            throw new IllegalArgumentException();
        }
        return index;
    }
 
    public Number getY(int series, int index) {
         if (index >= SAMPLE_SIZE_TCP){
             throw new IllegalArgumentException();
         }

         switch (series) {
	            case CWND:
	            	if (ping_ok_tcp==0) return ping_values_tcp[CWND][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[CWND][ping_ok_tcp];
	            	else return ping_values_tcp[CWND][index];
	            case AW:
	            	if (ping_ok_tcp==0) return ping_values_tcp[AW][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[AW][ping_ok_tcp];
	            	else return ping_values_tcp[AW][index];
	            case SSHTHRESH:
	            	if (ping_ok_tcp==0) return ping_values_tcp[SSHTHRESH][ping_ok_tcp];// > 1000 ? 0 : ping_values_tcp[SSHTHRESH][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[SSHTHRESH][ping_ok_tcp];// > 1000 ? 0 : ping_values_tcp[4][ping_ok_tcp-1];
	            	else return ping_values_tcp[SSHTHRESH][index];// > 1000 ? 0 : ping_values_tcp[SSHTHRESH][index];
	            case RTT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[RTT][ping_ok_tcp]/10000;
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[RTT][ping_ok_tcp]/10000;
	            	else return ping_values_tcp[RTT][index]/10000;
	            case SRTT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[SRTT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[SRTT][ping_ok_tcp];
	            	else return ping_values_tcp[SRTT][index];
	            case RTO:
	            	if (ping_ok_tcp==0) return ping_values_tcp[RTO][ping_ok_tcp]/10000;
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[RTO][ping_ok_tcp]/10000;
	            	else return ping_values_tcp[RTO][index]/10000;
	            case SRTTJTU:
	            	if (ping_ok_tcp==0) return ping_values_tcp[SRTTJTU][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[SRTTJTU][ping_ok_tcp];
	            	else return ping_values_tcp[SRTTJTU][index];
	            case FLIGHT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[FLIGHT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[FLIGHT][ping_ok_tcp];
	            	else return ping_values_tcp[FLIGHT][index];
	            case PACKETOUT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PACKETOUT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PACKETOUT][ping_ok_tcp];
	            	else return ping_values_tcp[PACKETOUT][index];
	            case PACKETLOST:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PACKETLOST][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PACKETLOST][ping_ok_tcp];
	            	else return ping_values_tcp[PACKETLOST][index];
	            case PACKETRET:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PACKETRET][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PACKETRET][ping_ok_tcp];
	            	else return ping_values_tcp[PACKETRET][index];
	            case PRRDEV:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PRRDEV][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PRRDEV][ping_ok_tcp];
	            	else return ping_values_tcp[PRRDEV][index];
	            case PRROUT:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PRROUT][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PRROUT][ping_ok_tcp];
	            	else return ping_values_tcp[PRROUT][index];
	            case TOTRET:
	            	if (ping_ok_tcp==0) return ping_values_tcp[TOTRET][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[TOTRET][ping_ok_tcp];
	            	else return ping_values_tcp[TOTRET][index];
	            case BYTESACK:
	            	if (ping_ok_tcp==0) return ping_values_tcp[BYTESACK][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[BYTESACK][ping_ok_tcp];
	            	else return ping_values_tcp[BYTESACK][index];
	            case PKTACKED:
	            	if (ping_ok_tcp==0) return ping_values_tcp[PKTACKED][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[PKTACKED][ping_ok_tcp];
	            	else return ping_values_tcp[PKTACKED][index];
	            case MSS:
	            	if (ping_ok_tcp==0) return ping_values_tcp[MSS][ping_ok_tcp];
	            	if (ping_ok_tcp<SAMPLE_SIZE_TCP && index > ping_ok_tcp) return ping_values_tcp[MSS][ping_ok_tcp];
	            	else return ping_values_tcp[MSS][index];
	            default:
	                throw new IllegalArgumentException();
	        }

    }
 
    public void addObserver(Observer observer) {
        notifier.addObserver(observer);
    }
 
    public void removeObserver(Observer observer) {
        notifier.deleteObserver(observer);
    }
 
}
