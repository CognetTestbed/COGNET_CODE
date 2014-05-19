/*
Cognetive Network programm 
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
#ifndef COMMONF_H 
#define COMMONF_H 

#ifdef __ANDROID__
#include <android/log.h>
#endif


void error(const char *msg);
char ** checkComand(const char *str, size_t *len);

void findTime(char *s);
void functionCreateFolder(char * nameExperiment);
// timeval_subtract (struct timeval *result, *x, *y);
#ifdef __ANDROID__
void checkParameterAppAndroid(char ** argv);
#endif

#endif 