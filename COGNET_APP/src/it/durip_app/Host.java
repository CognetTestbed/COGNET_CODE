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

public class Host {
	String mac;
	String ip;
	boolean active;
	
	public String getIp() {
		return ip;
	}
	
	public boolean isActive(){
		return active;
		
	}
	
	public void setIP(String ip) {
		this.ip = ip;
	}
	
	public String getMac() {
		return mac;
	}
	
	public void setMac(String mac) {
		this.mac = mac;
	}
	
	public void setActive(boolean active) {
		this.active = active;
	}
	
	public Host(String ip, String mac, boolean active) {
		super();
		this.ip = ip;
		this.mac = mac;
		this.active = active;
	}

}

