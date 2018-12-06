/**
\file
\date ноябрь 2018 года
\brief Константы, библиотеки

Данный файл содержит в себе подключение стандартных библиотек, а также определения основных констант
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <sys/perm.h>

// --- for levels.cpp
/// number of FADC boards with pmt [1-8]
#define BOARD 8
/// number of PMT channels on one FADC board
#define CHANPMT 8
/// number of ADC channels on one FADC board
#define CHANMAX 16

// --- for high.cpp
//#define HVMIN  2
/// number of vip channels
#define HVCHANN 64
/// maximum current, 100 mkA
#define MAXCURR 10
/// work current, 30 mkA
#define WORKCURR 3
/// LPT FADC input/output ports
#define LPT_IO     0x378
/// FADC input/output ports
#define FADC_IO    0x320
/// Base Address for LCD 
#define BaseAddrLCD  0x4320


// --- FADCBOARD
// trigger2_width + int2_width + led_delay2 (RG0,3,6,9)
// int2_width: 7=200ns (25ns x 8) - dlitelnost vorot integririvanija
//#define INT2_WIDTH 3
//#define INT2_WIDTH 2
/// 
#define INT2_WIDTH 0x7           // !!!!!!! change here   and in levels.cpp  !!!!!!!
#define TRG2 (100+0x700+0x3000)  // !!!!!!! change here   and in levels.cpp  !!!!!!!

/// glubina buffera 2-go urovnja (RG2,5,8,11)
#define BUFF2  510 //512
/// glubina buffera 1-go urovnja (ADC1(5,9,13)/DP, ADC3(7,11,15)/DP)
#define BUF1  256

/// device to find GPS on
#define  GPSDEVICE "/dev/ttyS0"

/// Union to conver Int to bytes
union CharInt_converter
{
  unsigned char tChar[4]; ///< 4 bytes for integer
  unsigned int  tInt;     ///< integer
} Conv;

/// Structure to hold onOff data
struct time_onoff
{
    time_t time_on; ///< on time
    time_t time_of; ///< off time
};

/// Structure to hold work parameters
struct input_parameters
{
    struct time_onoff timeOnOff;       ///< Structure to hold onOff data in all parameters
    time_t   period;                   ///< period to change data files and checvk apparatus, 1200 sec
    unsigned short trigger_onoff[112]; ///< channels trigger array
    unsigned short hvtrig[112];        ///< hv triggers
    unsigned short buf2;    ///< buf2 
    unsigned short master;  ///< trigger
    unsigned short gmaster; ///< global trigger
    unsigned short lmin;    ///< min level in fadc channel
    unsigned short lmax;    ///< max level in fadc channel
    unsigned char  umax;    ///< max U_high in hv channel
    unsigned char  umin;    ///< min U_high in hv channel
    unsigned char  h_umax;  ///< max U_high for HAMAMATSU
    unsigned char  h_umin;  ///< min U_high for HAMAMATSU
    unsigned char  h_vip;   ///< for HAMAMATSU
    unsigned char  h_chan;  ///< for HAMAMATSU
    float rate;             ///< rate in one channel
    int maxcur;             ///< maximal current
    int workcur;            ///< work current
    int h_maxcur;           ///< max current for HAMAMATSU  
    int h_workcur;          ///< work current for HAMAMATSU
    int hvchan;             ///< number of vip channels
    int onscreen;           ///< printf to screen(1) or no (0)
    int wait;               ///< wait begin time or start now
    int off;                ///< off apparatus or no
} Input, Default, Work;

FILE *dout;   ///< debug out file
FILE *fwork;  ///< debug out file
FILE *fout;   ///< data out file
FILE *fkadr;  ///< one event file
FILE *ffmin;  ///< every min file
FILE *f5sec;  ///< every 5min file

int  fgps;
int  EventNumber = 0;
int  commandin[21] = {0};

char gps_out[4096]  = {0};
char incl_out[1024] = {0};
char adc_out[1024]  = {0};
char pwr_out[1024]  = {0};
char msc_out[1024]  = {0};
char bar_out[1024]  = {0};
char comp_out[1024] = {0};
char lcd_out[4096]  = {0};

#include "bit.cpp"
#include "ioports.cpp"
#include "i2c.cpp"
#include "inclin.cpp"
#include "fadc.cpp"
#include "fadcbord.cpp" //includes "levels.cpp"
#include "files.cpp"
#include "hvps.cpp"
//#include "hvpstest.cpp"
#include "hvpssipm.cpp"
#include "trigger.cpp"
#include "lvps.cpp"
#include "lvpsdev.cpp"
#include "hvpsdev.cpp" // mosaic temperature
#include "gps.cpp"
#include "readinp.cpp"
#include "lcd.cpp"
#include "bar.cpp"
#include "compass_null.cpp"
#include "operate.cpp" // uncludes "nograph.cpp"

