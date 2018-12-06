/**
 * \file sphere.cpp
 * \date  ноябрь 2018 года
 * \author ptrwww@mail.ru
 * \version 2.0
 * \brief Программа автономной работы установки СФЕРА
 * 
 * Данный файл содержит в себе основную программу автономной работы установки СФЕРА-2 c SiPM. 
*/

#define LINUX 1
#define NO_SCOUT
#define PROC_TO_KILL "diag"
#define LOG_FILE "/home/data18/log/log.txt"
#define GPStoSTDOUT
//#define NO_BOOT
//#define NO_SET_HIGH

#include "include/include.h"

int main (void)
{
    int  res   = 0, ii = 0;
    long delta = 0;
    //unsigned int   tmp = 0;
    struct timeval tv0, tv1, tv0sec;
    time_t time0;
    char info[200];
    FILE *flog = NULL;

    Last.temp_top = 0;
    Last.temp_bot = 0;
    Last.high_inn = 0;
    Last.high_out = 0;

    GetIOPortsAccess();
    open_debug_file();
    print_time();
    res = read_input_files();
    if(res < 0)
    {
         printf("Errors in reading input TIME file!");
         return -1;
    }
    //printf("Onscreen: %i", Work.onscreen);   

    // -------------------------------------
    // -------------------------------------
    if(search_competitor()) // if LOG_FILE is changing
    {
        kill_competitor();  // alpha:  kill process PROC_TO_KILL 
    }

    // ------- declare and init devices  -------
    hvps_test  vip;      // class vip, on  // init vip
    compass Compass;     // class compass
    fadc_board Fadc ;    // class fadc on  // init and boot fadc
    lvps_dev   Vent;     // class lvps - ventillator
    hvps_dev   Mos;      // class hvps_dev - mosaic termometr
    inclinometer Clin;   // class inclinometer
    trigger_board Trigger; // class trigger // boot and init trigger
    lcd  LCD;
    barometer Bar[3];

    Trigger.fill_TriggerOnOff( Work.trigger_onoff); // fill TriggerOnOff[] according to input file
    bars_init(LCD, Bar);
    GPS_connect_init();

    // ----------- files for online telemetry monitoring ------
    res = open_telemetry_files();
    if(res < 0)
         printf("Errors to open telemetry files %d!", res);
    // ----------- files for online telemetry monitoring ------

    Every_sec(Fadc, vip, Trigger, Vent, Compass, Clin);
    gettimeofday(&tv1,    NULL);
    gettimeofday(&tv0sec, NULL);
    time(&time0);
    if(dout) fflush(dout);

    // ----------- wait start time ------
    if(Work.wait)
    {
        printf("start= %ld, now= %ld time0= %ld\n", Work.timeOnOff.time_on, tv1.tv_sec, time0);
        while( (Work.wait) && (tv1.tv_sec < Work.timeOnOff.time_on))
        {
            if((tv1.tv_sec - tv0sec.tv_sec) > 0)
            {
                Every_sec(Fadc, vip, Trigger, Vent, Compass,Clin);
                gettimeofday(&tv0sec, NULL);
            }
            if(!(sec5 % SEC60))
            {
                GPS_read_buffer(gps_out); // in
                // ----------- read barometers --------------
                bars_read(LCD, Bar, bar_out);

                // ----------- read other parameters --------
                Vent.read_vip_ADC(adc_out);
                Vent.read_power_temp(pwr_out);
                Mos.read_mosaic_temp(msc_out);
                LCD.read_ADC(lcd_out);

                // --- stdout: print all  parameters --------
                print_log_parameters(stdout);

                // --- ffmin: print all  parameters --------
                ffmin = freopen("1m.data", "wt", ffmin);
                if(ffmin)
                {
                    print_log_parameters(ffmin);
                    fflush(ffmin);
                }
                // ------ log: print all  parameters --------
                if((flog = fopen(LOG_FILE, "at")) != NULL)
                {
                    print_log_parameters(flog);
                    fclose(flog);
                }
            }

            gettimeofday(&tv1, NULL);

            // ---------   read command file -----------
            res = read_command_file();
            if(res > 0)
            {
                if(stdout) fprintf(stdout, "read_command_file: %d commands\n", res);
                if(  dout) fprintf(  dout, "read_command_file: %d commands\n", res);
                for(ii = 1; ii <= res; ii++)
                {
                    strcpy(info,"");
                    if(commandin[ii] == 1) // exit
                    {
                        strcpy(info, "\ncommand EXIT in file\n");
                        if(stdout) fprintf(stdout, info);
                        if(  dout) fprintf(  dout, info);
                        goto exit;
                    }
                    if(commandin[ii] == 3) //start
                    {
                        strcpy(info, "\ncommand START in file\n");
                        Work.wait = 0;
                    }
                    if(stdout) fprintf(stdout, info);
                    if(  dout) fprintf(  dout, info);
                }
            }
            // --------- end of read command file -----------
            res = 0;
        }
    }

    // ----------- start --------------
    // ----------- set high voltage ----------
#ifndef NO_SET_HIGH
    gettimeofday(&tv0, NULL);
#ifndef NO_SCOUT
    //res = vip.high_scout(1,1);  // probe if high voltage setting is possible
    res = vip.high_scout(ADDR,SUBADDR);  // !!! 2018: SiPM: probe if high voltage setting is possible
#endif
    if(!res) vip.high();  // set high voltage
    else
    {
        printf("No conditions for high on!!: res = %i\n", res);
        if(dout) fprintf(dout, "No conditions for high on!!!: res = %i\n", res);
        goto exit;
    }
    gettimeofday(&tv1, NULL);
    delta = tv1.tv_sec - tv0.tv_sec;
    printf("\n==high==> delta = %ld sec\n", delta);
    if(dout) fprintf(dout, "\n==high==> delta = %ld sec\n", delta);
    if(dout) fflush(dout);
#endif
    // --------- set levels --------
LEVELS:
#ifndef NO_SET_LEVELS
    gettimeofday(&tv0, NULL);
    Fadc.levels();
    gettimeofday(&tv1, NULL);
    delta = tv1.tv_sec - tv0.tv_sec;
    printf("\n==levels==>delta = %ld sec\n", delta);
    if(dout) fprintf(dout, "\n==levels==> delta = %ld sec\n", delta);
    if(dout) fflush(dout);
#endif
    Trigger.status();
    printf("debug close:\n");
    if(dout) fflush(dout);
    sleep(10); // 10sec
    if(dout) fclose(dout);
    printf("                    closed\n");
    open_debug_file();
    printf("debug open! \n");

    // ------------ run -------------------
    res = Operate(Fadc, vip, Trigger, Vent, Mos, Clin, LCD, Bar, Compass);
    if(res == 2) // levels
        goto LEVELS;

exit:
    // ------------ end -------------------
#ifndef NO_OFF
    if( Work.off)
    {
        // turn off
        vip.turn_off();
        Fadc.turn_off_fadc_boards();
    }
#endif

    printf("\ninn_vent off:\n");    
    Vent.set_inn_vent(0);
    printf("\nout vent off:\n");    
    Vent.set_out_vent(0);

    if(dout)  fclose(dout);
    if(fgps)  close(fgps);
    fclose(stderr);
    fclose(stdout);
    fclose(fkadr);
    fclose(ffmin);
    fclose(f5sec);
    CloseIOPortsAccess();

    return res;
}
