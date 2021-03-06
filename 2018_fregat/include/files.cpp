/**
\file
\brief Функции для работы с файлами

Данный файл содержит в себе определения основных функций, работающих с файлами
*/
/// first part of filename to write binary data
#define FILE_NAME "/home/data18/s"

void time_to_file();
extern int FileNum;


/** -----------------------------------
 *  \brief print debug message to stdout and debug file 
*/
void print_debug(char* message)
{
    if(stdout) printf(      "%s", message);
    if(dout)   fprintf(dout,"%s", message);    
}

/** -----------------------------------
/// print to datafile begining some information about current detector configuration:
/// BUF2, CHANMAX, 
/// AddrOn - number of fadc plates
*/
unsigned int init_data_file(FILE* file, fadc_board &Fadc)
{
    if( !file ) return 1; // no data file is open

    time_to_file();

    // print BUF2 and CHANMAX to file
    Conv.tInt = Fadc.Buf2; //BUF2;
    fprintf(file, "b%c%c",Conv.tChar[1], Conv.tChar[0]);
    Conv.tInt = CHANMAX;
    fprintf(file, "c%c%c",Conv.tChar[1], Conv.tChar[0]);
    
    // print fadc boards number to file
    Conv.tInt = Fadc.AddrOn[0];
    fprintf(file, "a%c%c",Conv.tChar[1], Conv.tChar[0]);
    fflush(file);

    return 0;
}

/** --------------------------------------
    \brief Generation of new outdata filename
     Generation of new outdata filename
    \param *filename  строка для хранения нового имени файла
    \param filenum    номер файла данных 
     example of using:
        \code
        char filename[100] = {"\0"};
        char f = *filename;
        f = new_filename(filename, num);
        printf("main: %s\n", filename);
        \endcode
*/
char new_filename(char *filename, unsigned int filenum)
{
    //char num[4] = {" "};
    //int  nnn = 0;
    char time_string[40];
    struct timeval tv;
    struct tm* ptm;

    // make file name
    strcpy(filename, FILE_NAME);
    gettimeofday(&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d_%H_%M", ptm);
    strcat(filename, time_string);

    sprintf(filename, "%s.%i", filename, filenum);
    printf("nf: %s\n", filename);

    return *filename;
}

/** --------------------------------------
    Prints current time to current binary data file *fout.
*/
void time_to_file()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    Conv.tInt = tv.tv_sec;
    if(fout)   fprintf(fout, "t%c%c%c%c", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
    if(stdout) fprintf(stdout, "t-%3i-%3i-%3i-%3i\n", Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
}

/** --------------------------------------
    Prints current time to text file *ff
    \param *ff  file to print timestamp
*/
void timestamp_to_file(FILE *ff)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    //Conv.tInt = tv.tv_sec;
    if(ff)  fprintf(ff, "<t>%ld ", tv.tv_sec);  // Conv.tChar[3], Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
}

/** --------------------------------------
    Open debug file with current time in filename
*/
int open_debug_file()
{
    char filename[100] = {"\0"};
    char f; // = *filename;

    f = new_filename(filename, 0);

    strcat(filename,".dbg");
    //if((dout = fopen("/home/data/debug.out", "w")) == NULL)
    if((dout = fopen(filename, "w")) == NULL)
    {
        fprintf(stderr, "Debug file is not open!");
        if(dout) fprintf(dout, "Debug file is not open!");
        return 1;
    }

    printf( "Debug file %s open!\n", filename);
    if(dout) fprintf(dout, "Debug file %s is open!\n", filename);
    return 0;
}

/** --------------------------------------
    Open and init binary data file with current time in filename
*/
int open_data_file(fadc_board &Fadc)
{
    char filename[100] = {"\0"};
    char f = *filename;

    // generate new file name
    f = new_filename(filename, FileNum++);

    // open out data file
    //if((fout = fopen("/home/data/graph.out", "w+b")) != NULL)
    if((fout = fopen(filename, "w+b")) == NULL)
    {
        if(dout) fprintf(dout, "Data file %s is not open!", filename);
        return 1;  // error
    }
    
    printf( "File %s open!\n", filename);
    if(dout) fprintf(dout, "File %s is open!\n", filename);
    init_data_file(fout, Fadc);
    return 0;
}

/** --------------------------------------
    Open debug files for online telemetry monitoring
*/
int open_telemetry_files()
{
    int n = 0;
    
    if((fkadr = fopen("event", "wt")) == NULL)
    {
        if(dout) fprintf(dout, "Data file \"kadr\" is not open!");
        n--;  // error
    }
    if((ffmin = fopen("1m.data", "wt")) == NULL)
    {
        if(dout) fprintf(dout, "Data file \"1m.data\" is not open!");
        n--;  // error
    }
    if((f5sec = fopen("5s.data", "wt")) == NULL)
    {
        if(dout) fprintf(dout, "Data file \"5s.data\" is not open!");
        n--;  // error
    }
    return n;
}
// ----------- files for online telemetry monitoring ------

