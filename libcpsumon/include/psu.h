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

#define TYPE_AX760  0
#define TYPE_AX860  1
#define TYPE_AX1200 2
#define TYPE_AX1500 3

#define FANMODE_AUTO 0
#define FANMODE_FIXED 1

#define true  1
#define false 0

typedef struct rail_12v_elem_t {
    float voltage;
    float current;
    float power;
    unsigned char ocp_enabled;
    float ocp_limit;
} rail_12v_elem_t;

typedef struct rail_misc_elem_t {
    float voltage;
    float current;
    float power;
} rail_misc_elem_t;

typedef struct rail_12v_t {
    rail_12v_elem_t pcie[10];
    rail_12v_elem_t atx;
    rail_12v_elem_t peripheral;
} rail_12v_t;


typedef struct rail_misc_t {
    rail_misc_elem_t rail_5v;
    rail_misc_elem_t rail_3_3v;
} rail_misc_t;

typedef struct psu_main_power_t {
    float voltage;
    float current;
    float inputpower;
    float outputpower;
    char cabletype;
    float efficiency;
} psu_main_power_t;

// extern int _psu_type;
// extern rail_12v_t _rail12v;
// extern rail_misc_t _railmisc;
// extern psu_main_power_t _psumain;

int _psu_type;
rail_12v_t _rail12v;
rail_misc_t _railmisc;
psu_main_power_t _psumain;

float convert_byte_float(unsigned char * data);
void convert_float_byte(float val, int exp, unsigned char *data);
int send_init(int fd);
unsigned char * read_data_psu(int fd, int reg, int len);
unsigned char * write_data_psu(int fd, int reg, unsigned char * data, int len);
int set_page(int fd, int main, int page);
int set_12v_page(int fd, int page);
int set_main_page(int fd, int page);
int read_psu_main_power(int fd);
int read_psu_rail12v(int fd);
int read_psu_railmisc(int fd);
int read_psu_fan_speed(int fd, float * f);
int read_psu_temp(int fd, float * f);
int read_psu_fan_fixed_percent(int fd, int * i);
int set_psu_fan_fixed_percent(int fd, float f);
int read_psu_fan_mode(int fd, int * m);
int set_psu_fan_mode(int fd, int m);
char * dump_psu_type(int type);
