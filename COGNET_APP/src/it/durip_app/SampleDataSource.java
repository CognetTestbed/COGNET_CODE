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

import java.util.Observable;
import java.util.Observer;
//import java.util.concurrent.locks.ReadWriteLock;
//import java.util.concurrent.locks.ReentrantReadWriteLock;
 
public class SampleDataSource implements Runnable {
 
    // encapsulates management of the observers watching this datasource for update events:
    class MyObservable extends Observable {
    @Override
    public void notifyObservers() {
        setChanged();
        super.notifyObservers();
    }
}
 
    private static final int MAX_AMP_SEED = 100;
    private static final int MIN_AMP_SEED = 10;
    private static final int AMP_STEP = 5;
    public static final int SINE1 = 0;
    public static final int SIN = 1;
    public static final int COS = 2;
    private static final int SAMPLE_SIZE = 30;
    private int phase = 0;
    private int sinAmp = 20;
    private MyObservable notifier;
 
    {
        notifier = new MyObservable();
    }
 
    //@Override
    public void run() {
        try {
            boolean isRising = true;
            while (true) {
 
                Thread.sleep(500); // decrease or remove to speed up the refresh rate.
                phase++;
                if (sinAmp >= MAX_AMP_SEED) {
                    isRising = false;
                } else if (sinAmp <= MIN_AMP_SEED) {
                    isRising = true;
                }
 
                if (isRising) {
                    sinAmp += AMP_STEP;
                } else {
                    sinAmp -= AMP_STEP;
                }
                notifier.notifyObservers();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
 
    public int getItemCount(int series) {
        return 30;
    }
 
    public Number getX(int series, int index) {
        if (index >= SAMPLE_SIZE) {
            throw new IllegalArgumentException();
        }
        return index;
    }
 
    public Number getY(int series, int index) {
         if (index >= SAMPLE_SIZE) {
             throw new IllegalArgumentException();
         }
        double amp = sinAmp * Math.sin(index + phase + 4);
        switch (series) {
            case SIN:
               return amp;
            case COS:
                return -amp;
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