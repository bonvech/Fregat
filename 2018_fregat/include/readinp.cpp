/**
 * \file readinp.cpp 
 * \brief Чтение входных файлов
 * 
 * Функции для чтения входных конфигурационных файлов.
 */
/// name of file with start and stop time and date
#define DATE_FILE  "./config/sdate.inp"
/// name of file with channel information
#define CHAN_FILE  "./config/channel.inp"
#define PARAM_FILE "./config/sparam.inp"
#define MASTER_FILE "./config/smaster.inp"
#define ENUM_FILE   "./config/enumber.config"

int  read_chan_from_file(unsigned short *trig, unsigned short *hvtrig);
struct time_onoff read_date_from_file();
int  read_param_from_file(input_parameters &Param);
int  print_param(input_parameters Param);
int set_default_parameters(input_parameters &Param);
int init_param();
int set_work_parameters();

/// Копирует size первых элементов массива dim1 в другой массив dim2
int mcopy(unsigned short dim1[], unsigned short dim2[], int size)
{
    int i = 0;

    for(i = 0; i < size; i++)
    {
        dim2[i] = dim1[i];
    }
    return 0;
}

/** --------------------------------------
 * \brief Read all input files
 * 
 * \return -1 - если неверно прочитана дата начала измерений\par
 *          0 - в случае успеха
 * 
*/
int read_input_files() 
{
    unsigned short kk = 0;
    int res = 0;
    char nname [100] = {0};
    FILE *fe;
    
    /// -- read Date from file --
    Work.timeOnOff = read_date_from_file();
    if(Work.timeOnOff.time_on < 10)
    {
        printf("DATA PANIC!!!!!\nError in reading DATE information!!!\n Check file \"config/sdate.inp\"!");
        return -1;
    }
    
    /// -- read EventNumber from file --
    strcpy(nname, ENUM_FILE);
    if ( (fe = fopen(nname,"r")) == NULL)
    {
        printf("File Data file %s not found!\n", nname);
        EventNumber = 0; 
    }
    else
    {
        if(fscanf(fe,"%d", &EventNumber) != 1) 
        {
            printf("File \"trans.config\" cannot be read! \nEvent numeration start from 0! ");
            EventNumber = 0;                
        }
        fclose (fe);
    }
    printf("\nEvent numeration starts from %d", EventNumber);
    if(dout) fprintf(dout, "\nEvent numeration starts from %d", EventNumber);
    
    /// -- read input parameters --
    init_param();
    set_default_parameters(Default);
    printf("Defaults:\n");
    print_param(Default);
    res = read_param_from_file(Input);
    printf("Errors in read %s file: %i\n", PARAM_FILE, res);
    if(dout) fprintf(dout, "Errors in read %s file: %i\n", PARAM_FILE, res);
    printf("Input:\n");
    if(dout) fprintf(dout, "Input:\n");
    print_param(Input);

    if(res == -1)
        set_default_parameters(Work);
    else set_work_parameters();
    printf("\nWork:\n");
    if(dout) fprintf(dout,"\nWork:\n");
    print_param(Work);

    /// -- read trigger labels from file --
    kk = read_chan_from_file(Input.trigger_onoff, Input.hvtrig);
    if( !kk)  mcopy(Input.trigger_onoff,    Work.trigger_onoff, Work.hvchan);
    else      mcopy(Default.trigger_onoff,  Work.trigger_onoff, Work.hvchan);
    if( !kk)
    {
        //printf("HV-trig:\n");
        if(dout) fprintf(dout, "Hv-trig:\n");
        for(kk = 0; kk < Work.hvchan; kk++)
        {
            //printf("hv = %i, chan = %i\n",kk, Input.hvtrig[kk]);
            if(dout) fprintf(dout, "hv = %i, chan = %i\n",kk, Input.hvtrig[kk]);
        }
    }
    
    for(kk = 0; kk < Work.hvchan; kk++)
    {
        if(dout) fprintf(dout, "i = %2i, work = %2i  inp = %2i  def = %2i\n",
               kk, Work.trigger_onoff[kk], Input.trigger_onoff[kk], Default.trigger_onoff[kk]);
    }
    
    return 0;
}

/** --------------------------------------
 * \brief Read trigger labels
 * 
 * Read trigger labels from file channels.inp. Пишет копию данных файла в ./log/channel.bak.
 * 
 * Заполняет массив триггера trig[] и массив hvtrig[hvps].
 * 
 * \return -1 - если неверно прочитана дата начала измерений;\par
 *          0 - в случае успеха 
*/
int read_chan_from_file(unsigned short *trig, unsigned short *hvtrig)
{
    FILE *fchan, *fbak;
    int kk = 0, num = 0, trigon = 0, hvps = 0, ii = 0;
    char line[200];

    for(ii = 0; ii < 112; ii ++)
    {
        hvtrig[ii]  = 200;
    }

    if((fchan = fopen( CHAN_FILE, "r")) == NULL)
    {
        printf("\n  Error: \"%s\" file is not open!\n", CHAN_FILE);
        if(dout) fprintf(dout, "\n  Error: file \"%s\" is not open!\n", CHAN_FILE);
        return -1;
    }
    printf(" file \"%s\" is open!\n", CHAN_FILE);
    if(dout) fprintf(dout, "\nfile \"%s\" is open!\n", CHAN_FILE);

    if(( fbak = fopen("./log/channel.bak","w")) == NULL)
    {
        printf("\n  Error: file \"./log/channel.bak\"  is not open!\n");
        if(dout) fprintf(dout, "\n  Error: file \"channel.bak\" is not open!\n");
    }

    while( fgets(line, sizeof(line), fchan) != NULL )
    {
        if( (line[0] == '\n') || (line[0] == '/') )
        {
            continue;
        }
        if(fbak) fprintf(fbak, "%s", line);
        hvps = 0;
        kk = sscanf(line, "%i %i %i", &num, &trigon, &hvps);
        if(kk < 2) //
        {
            //printf("break;");
            continue;
        }
        if((num < 0) && (num >= 112)) // no correct chan number
        {
            continue;
        }

        trig[num - 1] = trigon;

        if( hvps )
        {
            hvtrig[hvps] = num - 1;   //trig_hv[num - 1] = hvps;
        }

        //printf("trig[%2i - 1] = %i, hv= %i \n", num, trigon, hvps);
        if(dout) fprintf(dout, "trig[%2i - 1] = %i, hv= %i \n", num, trigon, hvps);
        //printf("trig[%2i - 1] = %i, hv= %i kk = %i\n", num, trigon, hvps, kk);

    }
    if(fbak) fclose (fbak);
    fclose(fchan);

    return 0; // O'k
}

/** --------------------------------------
 * \brief Read OnOff times
 * \date 2012_02_04 redaction
 * 
 * Read OnOff times from file DATE_FILE=sdate.inp, search nearest time. 
 * 
 * \return return tmin - структуру tm со временем начала измерения
 */
struct time_onoff read_date_from_file()
{
    FILE  *fdate;
    char  buf[180]  = {"\0"};
    char  line[200] = {"\0"};
    char  *cc = line;
    struct tm* ptm0;
    struct tm  tm1 = {0}, tm2 = {0};
    struct time_onoff delta, tmin;
    time_t time0 = 0, time1 = 0, time2 = 0, deltamin = 0;

    ptm0 = &tm1;  //
    // ----------------------------------
    if((fdate = fopen( DATE_FILE, "r")) == NULL)
    {
        printf("\n  Error: file \"%s\" is not open!\n", DATE_FILE);
        if(dout) fprintf(dout, "\n  Error: file \"%s\" is not open!\n", DATE_FILE);
        tmin.time_on = 1;
        return tmin;
    }
    printf("\n\n\n\nfile \"%s\" is open!\n", DATE_FILE);
    if(dout) fprintf(dout, "===> read dates from \"%s\"  <=====\n", DATE_FILE);

//    cc = strptime("2008-02-24 12:30", "%Y-%m-%d %H:%M", &tm1);
//    time1 = mktime( &tm1);
//    cc = strptime("2008-02-25 12:30", "%Y-%m-%d %H:%M", &tm2);
//    time2 = mktime( &tm2);

    //  -- get actial time -- 
    time(&time0);
    printf("time now = %ld\n\n", time0);
    tmin.time_on = time0;
    tmin.time_of = time0;
    delta.time_on = 0;
    delta.time_of = 0;
    deltamin = time0;

    // ----------------------------------
    while( fgets(line, sizeof(line), fdate) != NULL )
    {
        if(dout) fprintf(dout, "%s", line);
        if( (line[0] == '\n') || (line[0] == '/') )
        {
            continue;
        }
        // -- read begin_date --
        printf("\n==>%s", line);
        cc = strptime(line, "%Y-%m-%d %H:%M", &tm1);
        if(cc == NULL)
        {
            continue;
        }
        time1 = mktime( &tm1);
        //strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm1);
        //printf("%s", buf);
        printf(" time1 = %ld ", time1);

        // -- read end_date --
        strptime(cc, "%Y-%m-%d %H:%M", &tm2);
        time2 = mktime( &tm2);
        printf(" time2 = %ld ", time2);
        if(cc == NULL)
        {
            //printf("NULL-2\n");
            continue;
        }
        //strftime(buf, sizeof(buf), "   %d %b %Y %H:%M", &tm2);
        //puts(buf);

        // -- search proxima (min) date --
        if(time2-time1 < 0) continue;

        delta.time_on = time1-time0;
        delta.time_of = time2-time0;
        //printf( "delta: %ld %ld\n", delta.time_on, delta.time_of);
        if( (delta.time_of > 600) && ( delta.time_on < deltamin) )
        {
            tmin.time_on = time1;
            tmin.time_of = time2;
            deltamin = delta.time_on;
        }
    }
    //
    if(dout) fprintf(dout, "\n===> end of file \"%s\" <=====\n", DATE_FILE);

    if(tmin.time_on == time0)
    {
        printf("Error: No time to start!!! \n");
        tmin.time_on = 0;
        return tmin;
    }

    // -- print results --
    ptm0 = localtime(&time0);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ptm0);
    printf( " NOW: %s\n", buf);
    if(dout)  fprintf(dout, "\n time NOW: %s \n", buf);

    printf(" time  ON: = %ld :", tmin.time_on);
    ptm0 = localtime(&tmin.time_on);
    //strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm0);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ptm0);
    printf( ": %s\n", buf);
    if(dout)  fprintf(dout, " ON-OFF: %s ", buf);

    printf(" time OFF: = %ld :", tmin.time_of);
    ptm0 = localtime(&tmin.time_of);
    //strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm0);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ptm0);
    printf( ": %s\n", buf);
    if(dout)  fprintf(dout, " %s\n", buf);
    if(dout)  fprintf(dout, "=================================================\n");

    return tmin; // O'k
}


int read_param_from_file(input_parameters &Param)
{
    char  buf[30];
    char line[250];
    //char *cc;
    int  kk = 0, err = 0;
    unsigned char  ucpar = 0;
    unsigned short uspar = 0;
    float  ffpar = 0.0;
    time_t tpar = 0;

    FILE *fpar;

    // ----------------------------------
    // ----------------------------------
    if((fpar = fopen( PARAM_FILE, "r")) == NULL)
    {
        printf("\n  Error: file \"%s\" is not open!\n", PARAM_FILE);
        if(dout) fprintf(dout, "\n  Error: file \"%s\" is not open!\n", PARAM_FILE);
        return -1;
    }
    printf("\n\n\n\nfile \"%s\" is open!\n", PARAM_FILE);
    if(dout) fprintf(dout, "===> parameters dates from \"%s\"  <=====\n", PARAM_FILE);

    Work.master = 1;
    Work.gmaster = 0;

    // ----------------------------------
    while( fgets(line, sizeof(line), fpar) != NULL )
    {
        //printf("%s", line);
        if( (line[0] == '\n') || (line[0] == '/') )
        {
            continue;
        }
        if(dout) fprintf(dout, "%s", line);

        kk = sscanf(line, "%s", buf);
        //printf( "\n==>%s<==", buf);
        if(!kk) continue;

        uspar = 0;
        ucpar = 0;
        ffpar = 0;
        if( !strcmp(buf,"PERIOD") )
        {
             kk = sscanf(line, "%s %li", buf, &tpar);
             if(kk > 1)    Param.period = tpar;
             else      err++;
             //printf("kk = %i Period= %ld",kk, Param.period);
             continue;
        }
        if( !strcmp(buf,"MASTER") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.master = uspar;
            else      err++;
            //printf("kk = %i Period= %i",kk, Param.master);
            continue;
        }
        if( !strcmp(buf,"GMASTER") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.gmaster = uspar;
            else      err++;
            //printf("kk = %i Period= %i",kk, Param.gmaster);
            continue;
        }
        if( !strcmp(buf,"MAXCUR") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.maxcur = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"WORKCUR") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.workcur = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"LMIN") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.lmin = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"LMAX") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.lmax = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"UMAX") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.umax = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"UMIN") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.umin = uspar;
            else      err++;
            continue;
        }
        // 2012: params for Hamamatsu
        if( !strcmp(buf,"H_UMAX") ) // for Hamamatsu PMT
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.h_umax = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"H_UMIN") ) // for Hamamatsu PMT
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.h_umin = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"H_MAXCUR") ) // for Hamamatsu PMT
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.h_maxcur = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"H_WORKCUR") ) // for Hamamatsu PMT
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.h_workcur = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"H_VIP") ) // for Hamamatsu PMT
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.h_vip = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"H_CHAN") ) // for Hamamatsu PMT
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.h_chan = uspar;
            else      err++;
            continue;
        }
        // end of 2012: params for Hamamatsu
        if( !strcmp(buf,"BUF2") )
        {
            kk = sscanf(line, "%s %hu", buf,&uspar);
            if(kk > 1)    Param.buf2 = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"HVCHAN") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.hvchan = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"ONSCREEN") )
        {
            kk = sscanf(line, "%s %cu", buf,  &ucpar);
            if(kk > 1)    Param.onscreen = ucpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"WAIT") )
        {
            kk = sscanf(line, "%s %cu", buf, &ucpar);
            if(kk > 1)    Param.wait = ucpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"OFF") )
        {
            kk = sscanf(line, "%s %cu", buf, &ucpar);
            if(kk > 1)    Param.off = ucpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"RATE") )
        {
            kk = sscanf(line, "%s %f", buf, &ffpar);
            if(kk > 1)    Param.rate = ffpar;
            else      err++;
            continue;
        }

    }
    fclose(fpar);
    if(dout) fprintf(dout, "\n===> end of file \"%s\" <=====\n", PARAM_FILE);
    if(dout) fprintf(dout, "=============================================\n");
    return err; // O'k
}

//======================================
int read_master_from_file(input_parameters &Param)
{
    char  buf[30];
    char line[250];
    int  kk = 0, err = 0;
    unsigned short uspar = 0;

    FILE *fpar;

    // ----------------------------------
    if((fpar = fopen( MASTER_FILE, "r")) == NULL)
    {
        if(stdout) fprintf(stdout, "\n  Error: file \"%s\" is not open!\n", MASTER_FILE);
        if(  dout) fprintf(  dout, "\n  Error: file \"%s\" is not open!\n", MASTER_FILE);
        return -1;
    }
    printf("\n\n\n\nfile \"%s\" is open!\n", MASTER_FILE);
    if(dout) fprintf(dout, "\n===> MASTER from \"%s\"  <=====\n", MASTER_FILE);

    // ----------------------------------
    while( fgets(line, sizeof(line), fpar) != NULL )
    {
        //printf("%s", line);
        if( (line[0] == '\n') || (line[0] == '/') )
        {
            continue;
        }
        if(dout) fprintf(dout, "%s", line);

        kk = sscanf(line, "%s", buf);
        if(!kk) continue;

        uspar = 0;
        if( !strcmp(buf,"MASTER") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.master = uspar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"GMASTER") )
        {
            kk = sscanf(line, "%s %hu", buf, &uspar);
            if(kk > 1)    Param.gmaster = uspar;
            else      err++;
            continue;
        }
    }
    fclose(fpar);
    if(dout) fprintf(dout, "\n===> end of file \"%s\" <=====\n", MASTER_FILE);
    if(dout) fprintf(dout, "=============================================\n");
    return err; // number of errors
}
//======================================
int print_param(input_parameters Param)
{
    int kk = 0;

    kk = printf( "PERIOD  %4ld\n", Param.period);
    if(dout) fprintf(dout, "PERIOD  %4ld\n", Param.period);
    kk = printf( "MASTER  %4u\n", Param.master);
    if(dout) fprintf(dout, "MASTER  %4u\n", Param.master);
    kk = printf( "GMASTER  %4u\n", Param.gmaster);
    if(dout) fprintf(dout, "GMASTER  %4u\n", Param.gmaster);
    kk = printf( "MAXCUR  %4u\n", Param.maxcur);
    if(dout) fprintf(dout, "MAXCUR  %4u\n", Param.maxcur);
    kk = printf( "WORKCUR %4u\n", Param.workcur);
    if(dout) fprintf(dout, "WORKCUR %4u\n", Param.workcur);
    kk = printf( "RATE    %.2f\n", Param.rate);
    if(dout) fprintf(dout, "RATE    %.2f\n", Param.rate);
    kk = printf( "Lmin    %4u\n", Param.lmin);
    if(dout) fprintf(dout, "Lmin    %4u\n", Param.lmin);
    kk = printf( "Lmax    %4u\n", Param.lmax);
    if(dout) fprintf(dout, "Lmax    %4u\n", Param.lmax);
    kk = printf( "Umax    %4u\n", Param.umax);
    if(dout) fprintf(dout, "Umax    %4u\n", Param.umax);
    kk = printf( "UMIN    %4u\n", Param.umin);
    if(dout) fprintf(dout, "Umin    %4u\n", Param.umin);
    
    // 2012: params for Hamamatsu
    kk = printf( "H_Umax    %4u\n", Param.h_umax);
    if(dout) fprintf(dout, "H_Umax    %4u\n", Param.h_umax);
    kk = printf( "H_Umin    %4u\n", Param.h_umin);
    if(dout) fprintf(dout, "H_Umin    %4u\n", Param.h_umin);
    kk = printf( "H_MAXCUR    %4u\n", Param.h_maxcur);
    if(dout) fprintf(dout, "H_MAXCUR  %4u\n", Param.h_maxcur);
    kk = printf( "H_WORKCUR   %4u\n", Param.h_workcur);
    if(dout) fprintf(dout, "H_WORKCUR %4u\n", Param.h_workcur);
    kk = printf( "H_VIP    %4u\n", Param.h_vip);
    if(dout) fprintf(dout, "H_VIP    %4u\n", Param.h_vip);
    kk = printf( "H_CHAN   %4u\n", Param.h_chan);
    if(dout) fprintf(dout, "H_CHAN   %4u\n", Param.h_chan);

    kk = printf( "BUF2    %4u\n", Param.buf2 );
    if(dout) fprintf(dout, "BUF2    %4u\n", Param.buf2 );
    kk = printf( "HVCHAN  %4u\n", Param.hvchan);
    if(dout) fprintf(dout, "HVCHAN  %4u\n", Param.hvchan);
    kk = printf( "WAIT    %4u\n", Param.wait);
    if(dout) fprintf(dout,"WAIT    %4u\n", Param.wait);
    kk = printf( "ONSCREEN %3u\n", Param.onscreen);
    if(dout) fprintf(dout, "ONSCREEN %3u\n", Param.onscreen);
    kk = printf( "OFF     %3u\n", Param.off);
    if(dout) fprintf(dout, "OFF     %3u\n", Param.off);
    
    if(kk==0) ;
    return 0;
}

int init_param()
{
    Work.maxcur  = 0;  // mA
    Work.workcur = 0;  // mA
    Work.period  = 0;  //1200; // sec
    Work.buf2    = 510;// fadc buffer size
    Work.lmin    = 0;  // min level in fadc channel
    Work.lmax    = 8190;  // max level in fadc channel
    Work.umax    = 255;  // max U_high in hv channel
    Work.umin    = 0;  // min U_high in hv channel
    // 2012: for Hamamatsu
    Work.h_umax  = 155;  // max U_high in hv channel
    Work.h_umin  = 100;  // min U_high in hv channel
    Work.h_maxcur  = 10;  // mA
    Work.h_workcur = 6;  // mA
    Work.h_vip  = 53;  // max U_high in hv channel
    Work.h_chan = 1;   // min U_high in hv channel
    // end of Hamamatsu
    Work.rate    = 0.0; // rate in one channel
    Work.hvchan  = 0;  // number of vip channels
    Work.onscreen = 100; // printf to screen(1) or no (0)
    Work.master  = 0;  // printf to screen(1) or no (0)
    Work.wait    = 0;  // printf to screen(1) or no (0)
    Work.wait    = 1;  // printf to screen(1) or no (0)
    // --- init input parameters ---
    Input.maxcur  = 0;  // mA
    Input.workcur = 0;  // mA
    Input.period  = 0;  //1200; // sec
    Input.buf2    = 0;
    Input.lmin    = 0;  // min level in fadc channel
    Input.lmax    = 0;  // max level in fadc channel
    Input.umax    = 0;  // max U_high in hv channel
    Input.umin    = 0;  // max U_high in hv channel
    Input.rate    = 0.001; // rate in one channel
    Input.hvchan  = 0;     // number of vip channels
    Input.onscreen = 100;  // printf to screen(1) or no (0)
    Input.master  = 0;    // printf to screen(1) or no (0)
    Input.wait    = 100;  // printf to screen(1) or no (0)
    Input.off     = 100;  // printf to screen(1) or no (0)
    // 2012: for Hamamatsu
    Input.h_umax  = 0;  // max U_high in hv channel
    Input.h_umin  = 0;  // min U_high in hv channel
    Input.h_maxcur  = 0;  // mA
    Input.h_workcur = 0;  // mA
    Input.h_vip  = 0;  // max U_high in hv channel
    Input.h_chan = 0;  // min U_high in hv channel
    return 0;
}

int set_work_parameters()
{
    if(Input.maxcur)  Work.maxcur = Input.maxcur; // mA
    else              Work.maxcur = Default.maxcur;

    if(Input.workcur) Work.workcur = Input.workcur; // mA
    else              Work.workcur = Default.workcur;

    if(Input.period)  Work.period = Input.period;
    else              Work.period = Default.period;

    if(Input.period)  Work.period = Input.period;
    else              Work.period = Default.period;

    if(Input.buf2)    Work.buf2 = Input.buf2;
    else              Work.buf2 = Default.buf2;

    if(Input.lmin)    Work.lmin = Input.lmin;
    else              Work.lmin = Default.lmin;

    if(Input.lmax)    Work.lmax = Input.lmax;
    else              Work.lmax = Default.lmax;

    if(Input.umax)    Work.umax = Input.umax;
    else              Work.umax = Default.umax;

    if(Input.umin)    Work.umin = Input.umin;
    else              Work.umin = Default.umin;

    if(Input.rate > 0.005)  Work.rate = Input.rate;
    else                    Work.rate = Default.rate;

    if(Input.hvchan)  Work.hvchan = Input.hvchan;
    else              Work.hvchan = Default.hvchan;

    if(Input.onscreen != 100) Work.onscreen = Input.onscreen;
    else                      Work.onscreen = Default.onscreen;

    if(Input.wait != 100) Work.wait = Input.wait;
    else                  Work.wait = Default.wait;

    //if(Input.master)
    Work.master = Input.master;
    //else              Work.master = Default.master;

    //if(Input.gmaster)
    Work.gmaster = Input.gmaster;
    //else               Work.gmaster = Default.gmaster;

    if(Input.off != 100) Work.off = Input.off;
    else                 Work.off = Default.off;

    // ----------- 2012: for Hamamatsu ------
    if(Input.h_umax) Work.h_umax = Input.h_umax;
    else             Work.h_umax = Default.h_umax;
    if(Input.h_umin) Work.h_umin  = Input.h_umin ;
    else             Work.h_umin  = Default.h_umin ;
    if(Input.h_maxcur) Work.h_maxcur  = Input.h_maxcur ;
    else               Work.h_maxcur  = Default.h_maxcur ;
    if(Input.h_workcur) Work.h_workcur  = Input.h_workcur;
    else                Work.h_workcur  = Default.h_workcur ;
    if(Input.h_vip)  Work.h_vip  = Input.h_vip ;
    else             Work.h_vip  = Default.h_vip ;
    if(Input.h_chan) Work.h_chan  = Input.h_chan ;
    else             Work.h_chan  = Default.h_chan ;

    return 0;
}


int set_default_parameters(input_parameters &Param)
{
    int  i = 0;
    unsigned short trigger_onoff[112] = { 1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,
                                          1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,
                                          1,1,1,1,1,0,0,1,  1,0,0,1,1,0,0,1,
                                          1,0,0,1,1,0,0,1,  1,0,0,1,1,0,0,0,
                                          0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
                                          0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
                                          0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0 };

    for(i = 0; i < 112; i++)
    {
        Param.trigger_onoff[i] = trigger_onoff[i];
    }

    Param.maxcur  = MAXCURR;  // mA
    Param.workcur = WORKCURR;  // mA
    Param.period  = 300;   //1200; // sec
    Param.buf2    = BUFF2; //512;
    Param.lmin    = 1;    // min level in fadc channel
    Param.lmax    = 8192; // max level in fadc channel
    Param.umax    = 250;  // max U_high (hv) on PMT
    Param.umin    = 169;  // max U_high (hv) on PMT
    Param.rate    = 1.0;  // rate in one channel
    Param.hvchan  = HVCHANN;  // number of vip channels
    Param.onscreen = 0;  // printf to screen(1) or no (0)
    Param.master  =  0;  // master local
    Param.gmaster  =  0; // master global
    Param.wait    =  1;
    Param.off     =  1;
    // 2012: for Hamamatsu
    Param.h_umax  = 151;  // max U_high in hv channel
    Param.h_umin  = 51;  // min U_high in hv channel
    Param.h_maxcur  = 51;  // mA
    Param.h_workcur = 4;  // mA
    Param.h_vip  = 53;  // max U_high in hv channel
    Param.h_chan = 1;  // min U_high in hv channel

    return 0;
}
