/*
Corsair AXi Series PSU Monitor
Copyright (C) 2014 Andras Kovacs - andras@sth.sze.hu

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>
#include <time.h>

int open_usb(char *device);
int xread(int f, void * b, int s, int timeout);
int xwrite(int fd, void * buffer, unsigned int len);
void dump(unsigned char *buf, int dlen);
unsigned char * decode_answer(unsigned char *data, int size, int * nsize);
unsigned char * encode_answer(unsigned char command, unsigned char *data, int size, int * nsize);
unsigned char * data_read_dongle(int fd, int size, int * command);
int data_write_dongle(int fd, unsigned char * datain, int size);
unsigned char * read_dongle_name(int fd);
int read_dongle_version(int fd, float *f);
int setup_dongle(int fd);
