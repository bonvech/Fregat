/**
 * \file operate.cpp
 * \brief 
 * Функции, выполняемые во время основного цикла измерений. Вентиллятор.
 */
/// Temperature to stabilize
#define TERMOSTAB 21.0
/// Ventillator max code
#define VENT_MAX 254
/// Ventillator work code
#define VENT_ON  253
/// time delta to make 5sec file
#define SEC5 10
/// time delta to make every minute file
#define SEC60 60

int FileNum = 0; // number of f_out data file
int NumBar = 3;  //number of barometers in use
unsigned int  sec5 = 0;  // number to maje flag to write 5sec file

// --- functions --- 
int Every_sec(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, compass &Compass, inclinometer &Clin);
int Every_min(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, hvps_dev &Mos, lcd &LCD, barometer Bar[]);
unsigned char GetEvent(fadc_board &Fadc, trigger_board &Trigger, inclinometer &Clin, hvps_test &Vip, compass &Compass);
int simulate_event(fadc_board &Fadc, trigger_board &Trigger);
int Before(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, lcd &LCD);
int After(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger);
unsigned short Operate(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, lcd &LCD, compass &Compass, inclinometer &Clin);
void print_time();
int check_current(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger);
int check_temperature(fadc_board &Fadc, lvps_dev &Vent);
int read_threshould_from_file();
unsigned int bars_init(lcd &LCD, barometer Bar[]);
unsigned int bars_read(lcd &LCD, barometer Bar[], char *message_out);
int save_eventnumber_to_file();
int read_command_file(void);
int print_log_parameters(FILE *fileout);

/// structure to hold termometeres and ventillators 
struct temp
{
    float  temp_top; // verxnij termometr
    float  temp_bot; // nizhnij termomert
    unsigned char high_inn; //  inn ventilator
    unsigned char high_out; //  out ventillator
} Last;

//================================================
// ================= Every_sec  ==================
/** -------------------------------------------------------
 *  \brief Every second function to control parameters
 */
int Every_sec(fadc_board &Fadc, hvps_test &vip,   trigger_board &Trigger, 
              lvps_dev &Vent,   compass &Compass, inclinometer &Clin)
{

#ifdef PIEDESTAL
    //simulate_event(Fadc, Trigger);
#endif

    Compass.every_sec(comp_out);
    GPS_read_buffer(gps_out); // in

    sec5 ++;
    if(!(sec5 % SEC5))
    {
        Clin.read_incl_angle(incl_out);

        f5sec = freopen("5s.data", "wt", f5sec);
        if(f5sec) fprintf(f5sec,  "%s%s%s\n", gps_out, incl_out, comp_out);
        if(dout)  fprintf(dout,   "%s%s%s\n", gps_out, incl_out, comp_out);
    }

    check_temperature(Fadc, Vent);

    if(!(sec5 % SEC5))
        if(f5sec) fflush(f5sec);

    Trigger.pps_read_time();
    Trigger.pps_read_time();
    return 0;
}

//================================================
// ================= Every_min  ==================
/** -------------------------------------------------------
 *  \brief Every minute function to control parameters
 */
int Every_min(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, hvps_dev &Mos, lcd &LCD, barometer Bar[])
{
    int  dk = 0;
    FILE *flog = NULL;

    Trigger.trigger_prohibit();

    //GPS_read_buffer(gps_out); // in // p.publichenko
    check_temperature(Fadc, Vent);
    dk = check_current(Fadc, vip,  Trigger);
    Trigger.status();
    Trigger.pps_read_time();
    Trigger.pps_read_time();

    //if(!(sec5 % SEC60))
    //{
        // ----------- read barometers --------------
        bars_read(LCD, Bar, bar_out);

        // ----------- read other parameters --------
        vip.read_vip_ADC(adc_out);
        Vent.read_power_temp(pwr_out);
        Mos.read_mosaic_temp(msc_out);
        LCD.read_ADC(lcd_out);

        // --- stdout: print all  parameters --------
        print_log_parameters(stdout);
        print_log_parameters(  dout);

        // --- ffmin: print all  parameters --------
        ffmin = freopen("1m.data", "wt", ffmin);
        if(ffmin)
        {
            print_log_parameters(ffmin);
            //fprintf(ffmin, "\n%s%s%s %s\nGPS: %s", adc_out, pwr_out, msc_out, bar_out, gps_out); // !!!!!!!
            fflush(ffmin);
        }
        // ------ log: print all  parameters --------
        if((flog = fopen(LOG_FILE, "at")) != NULL)
        {
            print_log_parameters(flog);
            fclose(flog);
        }
    //}

    printf("\n\n");
    fflush(stdout);
    fflush(  dout);

    if(dk == 2) return 2; // no channels to trigger
    //if(dk < 2) return 2;  // no channels to trigger
    Trigger.trigger_permit();
    return 0;
}

//================================================
// =================   Before   ==================
/** -------------------------------------------------------
 *  \brief Procedure to run before work period
 */
int Before(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, lcd &LCD)
{
//    int nchan;
    //char gps_out[4096];// p.publichenko

    fflush(dout);
    open_data_file(Fadc);
    // p.publichenko
    GPS_read_buffer(gps_out); // in
    check_temperature(Fadc, Vent);
    //Fadc.start_fadc_boards(); // start fadc boards
    Fadc.test_fadc_boards();
    Fadc.reset_channels();
    check_temperature(Fadc, Vent);

    //-----------------
    read_threshould_from_file();
    Trigger.set_local_threshould(Work.master);
    Trigger.set_global_threshould(Work.gmaster);
    Trigger.set_trigger();   // set EVNTB01-89
    // ------------------
    LCD.set_config_from_file();
    Fadc.set_THR_from_file();
    //-----------------
    //simulate_event(Fadc, Trigger);
    Trigger.status();
    Trigger.pps_read_time();
    Trigger.pps_read_time();
    Fadc.reset_channels();
    Fadc.reset_counters();
    Fadc.start_counters();

    check_temperature(Fadc, Vent);
    vip.measure_all_high(); // 2010.03.11

    printf("\nWaiting for signal ... (press [Esc] and [Enter] for exit)\n");
    if(dout) fprintf(dout,"\nWaiting for signal ... \n\n");
    fflush(dout);
    Trigger.trigger_permit();
    // for (nchan=0; nchan<20; nchan++) Fadc.set_THR_save(nchan);
    return 0;
}

//================================================
// =================    After   ==================
/** -------------------------------------------------------
 *  \brief Procedure to run after the work period
 */
int After(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent)
{
    //char gps_out[4096];// p.publichenko

    Trigger.trigger_prohibit();
    //simulate_event(Fadc, Trigger);
    Trigger.status();
    // p.publichenko
    GPS_read_buffer(gps_out); // in
    check_temperature(Fadc, Vent);

    //Fadc.print_THR_to_file();
    Fadc.stop_counters();
    Fadc.read_counters();

    vip.measure_all_high(); // 2010.03.11

    time_to_file();
    if(fout) fclose(fout);

    fflush(dout);
    return 0;
}

//================================================
// =================   Period   ==================
/** -------------------------------------------------------
 *  \brief Procedure to run every work period
 */
int Period(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, lcd &LCD)
{
    After( Fadc, vip, Trigger, Vent);
    Before(Fadc, vip, Trigger, Vent, LCD);

    return 0;
}

/** -------------------------------------------------------
 *  \brief Simulate event
 *  \deprecated закомментировано
 */
int simulate_event(fadc_board &Fadc, trigger_board &Trigger)
{
/*    int dk = 0;
    if(Trigger.buffer_is_not_empty(TG) )
    {
        printf("SIM: NotEmpty:");
        if(dout) fprintf(dout, "SIM: NotEmpty: ");
        dk = GetEvent(Fadc, Trigger);
        printf( "\n k+%i \n", dk);
        if(dout) fprintf(dout, "k+%i\n", dk);
    }
    Trigger.trigger_sim();   // simulate event to control
    if(fout) fprintf(fout, "s");
    if(Trigger.buffer_is_not_empty(TG) )
    {
        printf("SIM: NotEmpty:");
        if(dout) fprintf(dout, "SIM: NotEmpty: ");

        dk = GetEvent(Fadc, Trigger);
        printf( "\n k+%i \n", dk);
        if(dout) fprintf(dout, "k+%i\n", dk);
    }*/
    return 0;
}

//================================================
// ================= GetEvent  ===================
/** -------------------------------------------------------
 *  \brief Get event if there is any event in buffer
 *  \return number of detected events
 */
unsigned char GetEvent(fadc_board &Fadc, trigger_board &Trigger, inclinometer &Clin, hvps_test &Vip, compass &Compass)
{ //if Trigger bufer not empty!
//    int nchan;
    unsigned char Over = 0, Err = 0, num = 0, i = 0;
    struct timeval tv0 = {0};
    int Inclination = 0, Magnitation = 0;
    //char event_out[1000] = {0};
    //char tmp[10] = {0};
    char info[100] = {0};

    num = Trigger.buffer_event_number(TG);
    fprintf(stdout, " num = %i\n", num);
    if(dout) fprintf(dout, " num = %i\n", num);
    if(num == 4)
    {
        if(Trigger.buffer_is_overflow(TG))
        {
            Trigger.trigger_prohibit();
            Fadc.prohibit_channels();
            Over = 1;
            fprintf(stdout, " Over = 1\n");
            if(dout) fprintf(dout, " Over = 1");
        }
    }

    // ----------- read inclinimeter ------------
    Inclination = Clin.read_incl_angle(info); // p.publichenko
    if(stdout)  fprintf(stdout, "%s", info);
    //Clin.printf_incl_angle(Inclination);
    //Conv.tInt = Inclination;
    //  if(fout)   fprintf(fout, "I%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);

    // ----------- read magnitometer ------------
/*    if(Compass.OperatMode & 2) 
         Magnitation = Compass.read_data(); 
    else Magnitation = Compass.Heading;
    if(stdout) printf("Compass %d.%d grad\n", Magnitation/10, Magnitation%10); 
*/    
    // print kadr to virtual event file
    //fkadr = freopen("event", "wb", fkadr); // 2013
    fkadr = freopen("event", "w", fkadr); // 2018
    if(fkadr) fprintf(fkadr, "%2i\n", EventNumber + 1); 

    //  --------- get events from buffer -------
    for(i = 0; i < num; i++)
    {
        // ---------- read event  --------
        EventNumber ++;
        //sprintf(event_out, "<K%05d>g%5s", EventNumber, gps_bstamp);
        fprintf(fout, "<K%05d>g%c%c%c%c%c", EventNumber, gps_bstamp[0],gps_bstamp[1],gps_bstamp[2],gps_bstamp[3],gps_bstamp[4]);
        sprintf(info, "<K%05d>  GPS:%15s", EventNumber, gps_sstamp);
        printf("%s     ", info);
        if(dout) fprintf(dout, "%s  ", info);

        // -- print localtime (sec)
        gettimeofday(&tv0, NULL);
        Conv.tInt = tv0.tv_sec;
        //if(fout)   fprintf(fout, "%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
        fprintf(fout, "t%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]); // localtime
        //strncat(event_out, tmp, 5);

        // -- print trigger time
        Conv.tInt = Trigger.tg_read_time();
        fprintf(fout, "e%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]); // triggertime
        //strncat(event_out, tmp, 5);

        // -- print Inclinometer
        Conv.tInt = Inclination;
        fprintf(fout, "I%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]); // Inclinometer
        //strncat(event_out, tmp, 5);

        // -- print Magnitometer
        Conv.tInt = Magnitation;
        fprintf(fout, "m%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]); // Inclinometer
        //strncat(event_out, tmp, 5);

        // -- print event_out to event file
        //fprintf(fout, "%s", event_out); 

        // -- print currents to event file
        //Vip.print_currents(stdout);
        Vip.print_currents_to_binary(fout);

        // -- print fadc data to event file
        Fadc.get_event(i);
        fprintf(fout, "</K%05d>", EventNumber);
        //fprintf(fout, "%s", event_out);
        fflush(fout);

        if(stdout) fprintf(stdout, "t-%3i-%3i-%3i-%3i\n", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
        if(  dout) fprintf(  dout, "t-%3i-%3i-%3i-%3i\n", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
        // check fifo_err
        if(!Err)
        {
            if(Fadc.fifo_err())
            {
                if(!Over)
                {
                    Trigger.trigger_prohibit();
                    Fadc.prohibit_channels();
                    Err = 1;
                    printf("Err = 1\n");
                }
            }
        }
    }
    save_eventnumber_to_file();

    // if overflow
    if((Over) || (Err))
    {
        Trigger.clear_buffer_overflow(TG);
        Fadc.reset_channels();
        printf("Fadc.fifo_err() = %i after\n", Fadc.fifo_err());
        Trigger.trigger_permit();
    }

    return num;
}


//New version of GetEvent->only resets buffer
/*
unsigned char GetEvent(fadc_board &Fadc, trigger_board &Trigger)
{ //if Trigger bufer not empty!
    unsigned char num = 0;
    num = Trigger.buffer_event_number(TG);
    Trigger.trigger_prohibit();
    Fadc.prohibit_channels();
    Trigger.clear_buffer_overflow(TG);
    Fadc.reset_channels();
    Trigger.trigger_permit();
    return num;
}
*/
//================================================
// ================= operate  ====================
/** -------------------------------------------------------
 *  \brief 
 *  \return 0 - OK\par 
 *          код команды - если читался командный файл
 */
unsigned short Operate(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger, lvps_dev &Vent, hvps_dev &Mos, inclinometer &Clin, lcd &LCD, barometer Bar[], compass &Compass)
{
    struct timeval tv00, tv0min, tv0sec, tv1;
    unsigned int   kadr = 0, res = 0, ii = 0;
    unsigned char  dk   = 0;
    char info[255] = {0};
    long period = 300; // sec - period
    time_t timeoff = 0;
    timeoff = Work.timeOnOff.time_of;
    period  = Work.period;
    Before(Fadc, vip, Trigger, Vent, LCD);
    gettimeofday(&tv00, NULL);
    tv0sec.tv_sec = tv00.tv_sec;
    tv0min.tv_sec = tv00.tv_sec;

    while(1)
    {
        gettimeofday(&tv1, NULL);
        if((tv1.tv_sec-tv0sec.tv_sec)>0)
        {
            Every_sec(Fadc, vip, Trigger, Vent, Compass, Clin);
            gettimeofday(&tv0sec, NULL);
        }
        if((tv1.tv_sec-tv0min.tv_sec) > 60)
        {
            printf("\nEvery min");
            if( Every_min(Fadc, vip, Trigger, Vent, Mos, LCD, Bar) == 2) goto STOP;
            gettimeofday(&tv1, NULL);
            tv0min.tv_sec = tv1.tv_sec;
            printf("Every min end");
        }
        if((tv1.tv_sec-tv00.tv_sec) > period)
        {
            printf("\nEvery period:");
            if(dout) fprintf(dout, "\nEvery period:");
            Period(Fadc, vip, Trigger, Vent, LCD);
            gettimeofday(&tv00, NULL);
            gettimeofday(&tv1, NULL);
            tv0min.tv_sec = tv1.tv_sec;
            tv0sec.tv_sec = tv1.tv_sec;
        }
        if( tv1.tv_sec > timeoff )
        { //Run finished->exit
            printf("Time Off!!\n");
            if(dout) fprintf(dout, "Time Off!!\n");
            goto STOP;
        }
        if(Trigger.buffer_is_not_empty(TG) )
        { //new event!
            if (Fadc.read_buffer_flag())    //if there is data in FADC buffers
            {
                ;
            }
            printf("TG:NotEmpty:");
            if(dout) fprintf(dout, "TG:NotEmpty: ");

            dk = GetEvent(Fadc, Trigger, Clin, vip, Compass);
            kadr += dk;
            printf( "\n k+%i=%i \n\n", dk, kadr);
            if(dout) fprintf(dout, "k+%i=%i\n", dk, kadr);
        }

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
                        res = 1;
                    }
                    if(commandin[ii] == 2) //levels
                    {
                        strcpy(info, "\ncommand LEVELS in file\n");
                        res = 2;
                    }
                    if(commandin[ii] == 4) //high off
                    {
                        strcpy(info, "\ncommand HIGH OFF in file - === EXIT\n");
                        res = 1;
                    }
                    if(commandin[ii] == 5) //led off
                    {
                        strcpy(info, "\ncommand LED OFF in file - NOT SUPPORTED\n");
                        continue;
                    }
                    if(stdout) fprintf(stdout, info);
                    if(  dout) fprintf(  dout, info);
                    goto STOP;
                }
        }
        // --------- end of read command file -----------
        res = 0;

    }   // end of while
STOP:
    After(Fadc, vip, Trigger, Vent);
    printf("\n");
    return res;
}

/** -------------------------------------------------------
 *  \brief print_time\par
 *  Print actual time to stdout and debug file
 */
void print_time()
{
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];
    long milliseconds;

    gettimeofday(&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
    milliseconds = tv.tv_usec/1000;
    printf("%s.%03ld\n", time_string, milliseconds);
    if(dout) fprintf(dout, "%s.%03ld\n", time_string, milliseconds);
}

/** -------------------------------------------------------
 *  \brief check_current
 *  \return 0 - OK\par
 *          1 - выключенные каналы исключены из триггера\par
 *          2 -
 */
int check_current(fadc_board &Fadc, hvps_test &vip, trigger_board &Trigger)
{
    int res = 0, ii = 0;
    res = vip.check_current();
    if( !res) return 0;
    if(res == 2) return 2;
    if(res == 1)
    {
        // exclude off channels from trigger
        for(ii = 0; ii <= Work.hvchan-1; ii++)
        {
            if( vip.hvwork[ii] )  continue;
            if( Input.hvtrig[ii] >= Work.hvchan) continue;
            Trigger.addOnOff_channel(Input.hvtrig[ii], 0);
        }
    }
    return 1;
}

/** -------------------------------------------------------
 *  \brief  check_temperature in electronic box
 *  \return 0 - OK\par
 *          1 - error in termometere\par
 *  Check temperature in the electronic box and correct it with ventillator
 */
int check_temperature(fadc_board &Fadc, lvps_dev &Vent)
{
    float delta = 0, tt = 0.;
    struct temp Now;

    Now.temp_bot = Fadc.read1_average_fadc_temp(0x49); // bottom
    Now.temp_top = Fadc.read1_average_fadc_temp(0x4A); // top
    if( Now.temp_bot < -95) return 1; // error in termometre
    if( Now.temp_top < -95) return 1;
    if(Now.temp_top <= TERMOSTAB)
    {
        Now.high_out = 0;
        Now.high_inn = 0;
    }
    else
    {
        Now.high_inn = Last.high_inn;
        Now.high_out = Last.high_out;
        if(!Now.high_out) Now.high_out = VENT_ON;
        if(!Now.high_inn) Now.high_inn = VENT_ON;
        // OUT vent
        //
        delta = Now.temp_top - TERMOSTAB;
        if(delta > 3.0)
        {
            Now.high_out ++;
        }
        // control of rising of top temperature
        delta = Now.temp_top - Last.temp_top;
        if(delta > 0.2)  // T rise
        {
            printf(" del = %.2f ", delta);
            Now.high_out ++;
        }
        else
        {
            if(delta < -0.2) // T down
            {
                printf(" del = %.2f ", delta);
                if(Now.high_out > VENT_ON) Now.high_out --;
            }
        }
        // control of temperature delta
        // INN vent
        delta = Now.temp_top - Now.temp_bot;
        if(delta > 5.0)
        {
            //on inn vent
            if(Last.high_inn >= VENT_ON)
            {
                tt = Last.temp_top - Last.temp_bot;
                tt = delta - tt;
                if(tt > 0.2) Now.high_inn ++;
            }
        }
        else
        {
            if(delta < 3.0)
            {
                // decrease inn vent
                if(Last.high_inn > VENT_ON) Now.high_inn --;
            }
        }
    }
    if(Now.high_inn > VENT_MAX) Now.high_inn = VENT_MAX;
    if(Now.high_out > VENT_MAX) Now.high_out = VENT_MAX;
    if(Now.high_inn - Last.high_inn)   Vent.set_inn_vent(Now.high_inn);
    if(Now.high_out - Last.high_out)   Vent.set_out_vent(Now.high_out);

    if(dout) fprintf(dout, "B: %.1f T: %.1f ou: %i in: %i\n",
        Now.temp_bot, Now.temp_top, Now.high_out, Now.high_inn);
    if(!(sec5 % SEC5)) if(f5sec) fprintf(f5sec, "B: %.1f T: %.1f\n", Now.temp_bot, Now.temp_top);

    Last.temp_top = Now.temp_top;
    Last.temp_bot = Now.temp_bot;
    Last.high_inn = Now.high_inn;
    Last.high_out = Now.high_out;
    return 0;
}

/** -------------------------------------------------------
 *  \brief check_current
 *  \return 0 - OK\par
 *          -1 - no input master file\par        
 */
int read_threshould_from_file()
{
    int res = 0;

    res = read_master_from_file(Input);

    printf("Errors in read %s file: %i\n", MASTER_FILE, res);
    if(dout) fprintf(dout, "Errors in read %s file: %i\n", MASTER_FILE, res);
    printf("Input: master %d, gmaster %d\n", Input.master, Input.gmaster);
    if(dout) fprintf(dout, "Input: master %d, gmaster %d\n", Input.master, Input.gmaster);

    //print_param(Input);

    if(res == -1) // NO input master file
        return -1;
    if(!res)  // no errors
    {    //set_work_parameters();
        Work.master  = Input.master;
        Work.gmaster = Input.gmaster;
    }

    printf("\n Work: master %d, gmaster %d\n", Work.master, Work.gmaster);
    if(dout) fprintf(dout,"\nWork: master %d, gmaster %d\n", Work.master, Work.gmaster);
    //print_param(Work);

    return 0;
}

//  -------------------------------------------------------
/** -------------------------------------------------------
 *  \brief barometers initialization
 *  \return NumBar
 */
unsigned int bars_init(lcd &LCD, barometer Bar[])
{
    int   i   = 0;
    int   tmp = 0;

    LCD.bar_onoff(1);  // on barometer 0
    LCD.bar_onoff(0);  // on barometer

    // set BaseAddresses to barometers
    tmp = BaseAddrLCD + AddrBAROMETER;
    for(i = 0; i < NumBar; i++)
    {
        Bar[i].SetAddr(tmp++);
        if(Bar[i].read_coef())
        {
            printf("\nBar[%i] : Error in read_koef()!!", i);
            NumBar = i;
        }
        else
            printf("\nBar[%i] : OK in read_koef()!!", i);
    }

    // == end of barometers initialization ============
    return NumBar;
}

/** -------------------------------------------------------
 *  \brief read barometers
 *  \return 0
 */
unsigned int bars_read(lcd &LCD, barometer Bar[], char *message_out)
{
    unsigned short  i = 0;
    //char bar_out[1024] = {"\0"};
    char message[1024]= {"\0"};
    char tmp[1024]= {"\0"};
    //int pp[3] = {0};
    //float dP = 0.;

    strcpy(message_out, "");
    for(i = 0; i < NumBar; i++)
    {
        strcpy(message, "");
        strcpy(tmp, "");
        //printf("LCD.bar_onoff(1)\n");
        LCD.bar_onoff(1);
        //printf("done. read_bar_temp()\n");
        //pp[i] = 
        Bar[i].read_bar_temp(message);
        //printf("LCD.bar_onoff(0)\n");
        LCD.bar_onoff(0);

        sprintf(tmp, "%i %s", i, message);
        strcat(message_out, tmp);
    }

    //dP = (float) (pp[0] - pp[1]) / 100.;
    //sprintf(tmp,"Delta P:  %6.2f kPa (%6.2f mm w)\n",dP, dP/0.00981);
    //strcat(message_out,tmp);

    //if(stdout) fprintf(stdout, "\n%s", message_out);
    //if(  dout) fprintf(  dout, "\n%s", message_out);
    //if( ffmin) fprintf( ffmin, "\n%s", message_out);

    //return pp[0];
    return 0;
}

/** -------------------------------------------------------
 *  \brief make_calibration
 */
void make_calibration(lcd &LED)
{
    int begin = 130, end = 0; // !! begin > end !!!
    int num = 100;            // number of light spots
    int ii = 0;

    for(ii = begin; ii < end; ii -= 5)
    {
        if(LED.set_chan_amp(8, ii))  printf("\nError in set_chan_amp!!  ii = %d", ii);
        else                         printf("lamp amplitude %d set\n", ii);
        sleep(60);      // 60 sec
        LED.lcd_pulseN_sec(num);
    }
    if(LED.set_chan_amp(8, 255))  printf("\nError in set_chan_amp!!  ii = %d", ii);
}

/** -------------------------------------------------------
 *  \brief print EventNumber to file
 */
int save_eventnumber_to_file()
{
    FILE *fevent;
    char info[200] = {0};

    if ( (fevent = fopen(ENUM_FILE,"w")) == NULL)
    {
        sprintf(info, "Error! File Data file %s not open!", ENUM_FILE);
    }
    else
    {
        fprintf(fevent, "%d", EventNumber);
        sprintf(info, "EventNumber %d printed to file %s",  EventNumber, ENUM_FILE);
        fclose(fevent);
    }
    if(stdout) fprintf(stdout, "%s\n", info);
    if(dout)   fprintf(  dout, "%s\n", info);
    return 0;
}

/** -------------------------------------------------------
 *  \brief get file size
 */
long getFileSize(const char *fileName)
{
    struct stat file_stat;
    if( stat(fileName, &file_stat) == -1)  // file no exists
        return 0;
    return file_stat.st_size;
}

/** -------------------------------------------------------
 *  \brief search if file log.txt changes
 */
int search_competitor()
{
    long  fsize = -1, fsize1 = -1;
    const char filelog[100] = {LOG_FILE};

    // ----------- read log.txt file size
    fsize = getFileSize(filelog);
    fsize = getFileSize(filelog);
    printf("filesize = %ld\n", fsize);
    sleep(6);
    fsize1 = getFileSize(filelog);
    printf("filesize = %ld\n", fsize1);
    fsize = fsize1 - fsize;

    if(fsize > 0) 
    {
        if(stdout) fprintf(stdout, "filesize delta = %ld\n", fsize);
        if(  dout) fprintf(  dout, "filesize delta = %ld\n", fsize);
        if(stdout) fprintf(stdout, "File %s changes! There is a competitor! \n", LOG_FILE);
        if(  dout) fprintf(  dout, "File %s changes! There is a competitor! \n", LOG_FILE);
        return 1;
    }

    if(stdout) fprintf(stdout, "File %s doesn`t change! There is NOT a process-competitor! \n", LOG_FILE);
    if(  dout) fprintf(  dout, "File %s doesn`t change! There is NOT a process-competitor! \n", LOG_FILE);
    return 0;
}

/** -------------------------------------------------------
 *  \brief kill of another 'diag' process
 */
int kill_competitor(void)
{
    char syscom[100] = {0};
    if( search_competitor())
    {
        sprintf(syscom,"killall -9 %s; echo $?", PROC_TO_KILL);
        system(syscom);
        sleep(19);
    }
    return 0;
}

/** -------------------------------------------------------
 *  \brief read command file
 *  \return -1 - error in file reading 
 *          1 - command "exit" in file
 *          2 - command "levels" 
 *          4 - command "high_off" in file 
 *          5 - command "led_off" in file - NOT SUPPORTED
 */
int read_command_file(void)
{
    FILE *fcomin = NULL, *fcomout = NULL;
    time_t t;
    long fsize = -1;
    int  comnum = 0, il = 0, flag = 0;
    char filename[100] = {"command.in"};
    char fileout[100]  = {"command.out"};
    char line[80]     = {0};
    char commands[6][15] = {"exit","exit","levels","start","high_off","led_off"};

    // ----------- check command file sizesize
    fsize = getFileSize(filename);
    if(!fsize) return 0;

    // ----- open command file if its size > 3 byte
    if( (fcomin = fopen(filename, "r")) == NULL) 
    {
        printf("\n Error in command.in file reading! \n");
        return -1;
    }
    if( (fcomout = fopen(fileout, "a")) == NULL) 
        printf("\n Error in open command.out file to write! \n");
    else
    {
        time(&t);
        fprintf(fcomout, "\n\n%s\n", ctime(&t) );
    }
    if(dout)  fprintf(dout, "\nRead command file\n%s\n", ctime(&t) );

    // ------------ read command file
    while( fgets(line, sizeof(line), fcomin) != NULL )
    {
        if( (line[0] == '\n') || (line[0] == '/') )
            continue;

        // --- if line contains command - save command in commandin[]
        flag = 0;
        for(il = 1; il <= 5; il++)
        {
            if(strstr(line, commands[il]))
            {
                flag = 1;
                comnum ++;
                commandin[comnum] = il;
            }
        }
        commandin[0] = comnum;

        if(flag) if(fcomout) fprintf(fcomout, "%s", line);
        if(flag) if(stdout)  fprintf(stdout, "commandin: %s command = %d \"%s\"\n", line, commandin[comnum], commands[commandin[comnum]] );
        if(flag) if(  dout)  fprintf(  dout, "commandin: %s command = %d \"%s\"\n", line, commandin[comnum], commands[commandin[comnum]] );
    }
    fclose(fcomin);
    if(fcomout) fclose(fcomout);

    // ---------- clear command.in file
    if( (fcomin = fopen(filename, "w")) == NULL) 
    {
        if(  dout)  fprintf(  dout, "\n Error in command.in file re_writing! \n");
        if(stdout)  fprintf(stdout, "\n Error in command.in file re_writing! \n");
        return -2;
    }
    fclose(fcomin);

    return comnum;
}

/** -------------------------------------------------------
 *  \brief print log parameters
 */
int print_log_parameters(FILE *fileout)
{
        time_t t;

        time(&t);
        fprintf(fileout, "%s\n%s\n%s\n%s\n",  ctime(&t), gps_out, bar_out, incl_out);
        fprintf(fileout, "%s\n%s%s\n%s\n%s\n", comp_out, adc_out, pwr_out, msc_out, lcd_out);
        fprintf(fileout, "-----------------------\n");
        return 0;
}
// ----------------------------------------------
