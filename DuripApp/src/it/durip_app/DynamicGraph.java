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

import com.androidplot.series.XYSeries;

public class DynamicGraph implements XYSeries {
    private DataSource datasource;
    private int seriesIndex;
    private String title;
 
    public DynamicGraph(DataSource datasource, int seriesIndex, String title) {
        this.datasource = datasource;
        this.seriesIndex = seriesIndex;
        this.title = title;
    }
    @Override
    public String getTitle() {
        return title;
    }
 
    @Override
    public int size() {
        return datasource.getItemCount(seriesIndex);
    }
 
    @Override
    public Number getX(int index) {
        return datasource.getX(seriesIndex, index);
    }
 
    @Override
    public Number getY(int index) {
        return datasource.getY(seriesIndex, index);
    }
}
