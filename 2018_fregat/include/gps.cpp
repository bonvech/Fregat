/**
 * \file gps.cpp
 * \brief GPS
 * 
 * Описание функций для работы с GPS.
 */

//#define  GPStoSTDOUT
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

char gps_bstamp[6] = {"     "};
char gps_sstamp[16] = {0};

/*
$GPGGA,140807,5148.0720,N,10424.3270,E,1,08,0.9,457.8,M,-37.2,M,,*64
*/

int parse_gps(char *buf, char *time, double &n, double &e, int &num, float &error, float &height)
{
  unsigned int i;
  for(i=0;i<strlen(buf);i++)
  {
    if (buf[i] == ',') { buf[i] = ' '; }
  }

  int h,m,s;
  sscanf(buf, "%*s %2d%2d%2d %lf %*s %lf %*s %*s %d %f %f",&h,&m,&s, &n,&e,&num,&error,&height);
  sprintf(time, "%02d:%02d:%02d", h,m,s);
  height -= 452.2;
  return 0;
}


int GPS_write_to(char* sent)
{
    char string[150];
    int length = 0, n = 0;

    strcpy(string, sent);
    length = strlen(string);
    strcat(string, "\r\n");
    length += 2;

    printf("write=>\"%s\"<= l=%2x\n", string, length);
    //printf("write=>\"%s\"<= l=%i\n", string, length);
    n = write(fgps, string, length);
    //n = write(fgps, "$PGRMO,,2\r\n", 11);
    if(n < 0)
    {
        fprintf(stdout,"\nGPS Error:  write error - %i ", errno);
        return errno;
    }
    fprintf(stdout,"\nGPS Write: OK");
    return 0;
}



int GPS_set_baudrate(int rate)
{
    char string[150];
    //char tchar[5];
    int  res = 0;

    if(rate>7)  return 10;

    strcpy(string, "$PGRMC,,,,,,,,,,");
    switch (rate)
    {
        case 1: strcat(string, "1"); break;
        case 2: strcat(string, "2"); break;
        case 3: strcat(string, "3"); break;
        case 4: strcat(string, "4"); break;
        case 5: strcat(string, "5"); break;
        case 6: strcat(string, "6"); break;
        case 7: strcat(string, "7"); break;
        default: return -1;
    }
    //itoa(res, tchar, 10);
    //strcat(string, tchar);
    strcat(string, ",,2,0,");

    res = GPS_write_to(string);
    return res;

}

int GPS_set_defaults()
{
    char string1[150];
    strcpy(string1, "$PGRMO,,4");
    return GPS_write_to(string1);
}

int GPS_no_output()
{
    char string1[150];
    strcpy(string1, "$PGRMO,,2");
    return GPS_write_to(string1);
}

int GPS_set_GPGGA()
{
    char string1[150];
    strcpy(string1, "$PGRMO,GPGGA,1");
    return GPS_write_to(string1);
}

/////////////////////////////////////////////
int GPS_read_buffer(char *message)
{
  int  k = 0;
  int  bbytes = 0, rbytes = 0; //, mbytes = 0;
  char buf[1024];
  int  h = 0,m = 0,s = 0;
  unsigned char gph = 0,gpm = 0,gps = 0;
  float height = 0;
  //char message[100] = {"$GPGGA 230460 5147.8580 N 10423.3296 E 1 09 1.1 1762.4 M -37.2 M  *62"};
#ifdef  GPStoSTDOUT
  char infos[1024] = {0};
#endif

  ioctl(fgps, FIONREAD, &bbytes);  // read byte number in port buffer
  if(!bbytes) return 0;

  while (bbytes > 0)
  {
      // read from buffer
      if ((rbytes = read(fgps, buf, 255)) == -1)
      {
          fprintf(stdout, "Read Failed.\n");
          return -1;
      }
      bbytes -= rbytes;
      if(rbytes == 1)
      {
          if(buf[0]=='\n')
                continue;
      }

      // -- p.publichenko -- //
      // parsing GPS string..
      for(k = 0; k < rbytes-1; k++)
      {
          if(buf[k] == ',')   buf[k] = ' '; 
      }
      buf[rbytes] = '\0';
      //parse_gps(buf, time,n,e,num,error,height);
      //sprintf(message, "[%*s]\nTime=%s N=%.4lf E=%.4lf Num=%d error=%.1f Height=%.1f\n", rbytes-1, buf,time,n,e,num,error,height);      
      sprintf(message, "%*s", rbytes-1, buf);
      //sprintf(message, "%s\0", buf);

      // --- printf to stdout
#ifdef GPStoSTDOUT
/*      strcpy(infos,"");
      for(k = 0; k < rbytes-1; k++)
          strcat(infos,"\b");
      strcat(infos,"\r");
      if(stdout)  fprintf(stdout, "%s", infos);
      strcpy(infos, buf);
      if(stdout)  fprintf(stdout, "%s", infos);
*/

      for(k = 0; k < rbytes-1; k++)
          fprintf(stdout, "\b");
      fprintf(stdout, "\r");
      //for(k = 0; k < rbytes-1; k++)
        //  fprintf(stdout, "%c",buf[k]);
      strncpy(infos, buf, rbytes-1);
      if(stdout)  fprintf(stdout, "%s", infos);

      fflush(stdout);
#endif
      //if(dout) fprintf(dout, "%s", buf);
  }

  // -- print GPS STAMP 
  sscanf(message, "%*s %2d%2d%2d %*f %*s %*f %*s %*s %*d %*f %f",&h,&m,&s, &height);
  sprintf(gps_sstamp,"%02d:%02d:%02d %6.1f", h, m,s,height);  
  //printf("\nGPS_sstamp>%15s<\n", gps_sstamp);
  gph = (unsigned char) h;
  gpm = (unsigned char) m;
  gps = (unsigned char) s;
  height *= 10.;
  Conv.tInt = (unsigned short) height;
  sprintf(gps_bstamp,"%c%c%c%c%c",gph, gpm,gps,Conv.tChar[1],Conv.tChar[0]);  
  //printf("height = %.2f hh: %d=%d\n",height,Conv.tChar[1],Conv.tChar[0]);  
  //printf("\nGPS_stamp>%5s< >%d=%d=%d=%d=%d<\n", gps_stamp,gps_stamp[0],gps_stamp[1],gps_stamp[2],gps_stamp[3],gps_stamp[4]);

  return 0;
}

// int GPS_to_binary_file(FILE *fby, char *message)
// {
//     int  rbytes = 0; //, k = 0;
//     
//     rbytes = strlen(message);
//     if(rbytes > 255)
//     {
//         printf("\nWarning !! GPS sentense > 255 symbols !!!\n");
//         rbytes = 255;
//     }
//     
//     if(fby) fprintf(fby, "g%c", rbytes);
//     if(fby) fprintf(fby, "%*s", rbytes-1, message);
//     return 0;
// }

// =================================
int GPS_connect_init()
{
    int in = 0;
    struct termios options;

    in = ioperm(0x3f8,0x8,1);
    if(in) ;
    if( (fgps = open(GPSDEVICE, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    //if( (fgps = open(GPSDEVICE, O_RDWR | O_NOCTTY )) < 0)
    {
      if(dout) fprintf(dout, "GPS_connect_init: Error in COM1\n");
      fprintf(stdout, "GPS_connect_init: Error in COM1\n");
      exit(-1);
    }
    else 
    {
        fprintf(stdout, "GPS_connect_init: COM1 is open!!\n");
        if(dout) fprintf(dout, "GPS_connect_init: COM1 is open!!\n");
    }
    //fcntl(fgps, F_SETFL, FNDELAY);

    // options
    tcgetattr(fgps, &options);  // get current port options
    options.c_cflag |= (CLOCAL | CREAD); // local line and
    // 8N1
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    cfsetispeed(&options, B19200);
    cfsetospeed(&options, B19200);
    //
    tcsetattr(fgps, TCSANOW, &options);

    return 0;
}

