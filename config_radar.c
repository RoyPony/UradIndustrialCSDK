//c libraries
#include <stdio.h>
#include <errno.h> // Error integer and strerror() function
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>
#include "serial_data.h"
#include "errors.h"

//defines
#define NUM_OF_LINES_CONFIGURATION 30

//consts
const int MAX_LINE_LENGTH = 100;
char *CONFIGURATIONS[NUM_OF_LINES_CONFIGURATION] = {
"sensorStop\n",
"flushCfg\n",
"dfeDataOutputMode 1\n",
"channelCfg 15 7 0\n",
"adcCfg 2 1\n",
"adcbufCfg -1 0 1 1 1\n",
"profileCfg 0 60 359 7 57.14 0 0 70 1 256 5209 0 0 158\n",
"chirpCfg 0 0 0 0 0 0 0 1\n",
"chirpCfg 1 1 0 0 0 0 0 2\n",
"chirpCfg 2 2 0 0 0 0 0 4\n",
"frameCfg 0 2 16 0 100 1 0\n",
"lowPower 0 0\n",
"guiMonitor -1 1 0 0 0 0 0\n",
"cfarCfg -1 0 2 8 4 3 0 15 1\n",
"cfarCfg -1 1 0 4 2 3 1 15 1\n",
"multiObjBeamForming -1 1 0.5\n",
"clutterRemoval -1 1\n",
"calibDcRangeSig -1 0 -5 8 256\n",
"extendedMaxVelocity -1 0\n",
"lvdsStreamCfg -1 0 0 0\n",
"compRangeBiasAndRxChanPhase 0.0 1 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 -1 0\n",
"measureRangeBiasAndRxChanPhase 0 1.5 0.2\n",
"CQRxSatMonitor 0 3 5 121 0\n",
"CQSigImgMonitor 0 127 4\n",
"analogMonitor 0 0\n",
"aoaFovCfg -1 -90 90 -90 90\n",
"cfarFovCfg -1 0 0 49.99\n",
"cfarFovCfg -1 1 -1 1.00\n",
"calibData 0 0 0\n",
"sensorStart\n"
};

/*
PARAMS: char pointer to line buffer,file pointer, bool pointer to store if fp reached end of file
read  a line from fp into line buffer, when reach EOF end_of_file wiil set to true
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int readline(char *line, FILE *fp, bool *end_of_file)
{
    int ch = 0;
    int index = 0;

    //check for valid parameters
    if(line == NULL || fp == NULL || end_of_file == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    while ((ch = getc(fp)) != '\n')
    {
        if(ch == EOF ) // check enf of file
        {
            if(index == 0) // no data to read
            {
                *end_of_file = true;
                return STATUS_OK;
            }
            else //end of last line
            {
                break;
            }
        }
        line[index] = ch;
        index++;
    }

    // end of line
    line[index] = '\n'; 
    line[index + 1] = '\0';

    *end_of_file = false;
    
    return STATUS_OK;
}


/*
PARAMS: char pointer to line buffer,file pointer, bool pointer to store if fp reached end of file
read config file line by line and store the lines pointers array at lines_pointer
~ inner malloc lines array ~
~ inner malloc every line ~
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_FILE, CANNOT_CLOSE_FILE, CANNOT_ALLOCATE_MEMORY
*/
int get_config_file_by_lines(char *config_file_path,char *** lines_pointer)
{
    int status = STATUS_NOT_OK;
    size_t i = 0;
    FILE *config_fp = NULL;
    char **lines = NULL;
    bool eof = false;

    //check for valid parameters
    if(config_file_path == NULL || lines_pointer == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    // open configuration file
    config_fp = fopen(config_file_path, "r");
    if (config_fp == NULL)
    {
        return CANNOT_OPEN_FILE;
    }

    // aloccate array of pointers (each cell is pointer for line)
    lines = malloc(sizeof(char *) * NUM_OF_LINES_CONFIGURATION); 

    for (i = 0; i < NUM_OF_LINES_CONFIGURATION; i++)
    // read and save all lines
    {
        lines[i] = malloc(MAX_LINE_LENGTH); //allocate line
        if(lines[i] == NULL)
        {
            return CANNOT_ALLOCATE_MEMORY;
        }

        status = readline(lines[i], config_fp, &eof);
        if(status != STATUS_OK)
        {
            return status;
        }
    }

    //close the file
    status = fclose(config_fp); 
    if(status != STATUS_OK)
    {
        return CANNOT_CLOSE_FILE;
    }

    *lines_pointer = lines;

    return STATUS_OK;
}

/*
PARAMS: lines pointer 
dispose the memory located for config radar settings file
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int dispose_config_file(char **lines)
{
    size_t i = 0;

    //check for valid parameters
    if(lines == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    //dispose lines
    for (i = 0; i < NUM_OF_LINES_CONFIGURATION; i++)
    {
        free(lines[i]);
    }
    free(lines);

    return STATUS_OK;
}

/*
PARAMS: path to port, char pointer to file name
config radar -> send the settings located on CONFIGURATIONS to the radar
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_WRITE_TO_RADAR
*/
int config_radar_arr(char *DATA_PORT_LOCATION)
{
    int status = STATUS_NOT_OK;
    size_t i = 0;
    int serial_port = -1;
    int len = 0;

    status = open_config_port(DATA_PORT_LOCATION, &serial_port);
    if(status != STATUS_OK)
    {
        return status;
    }    

    //send configuration to the radar
    for (i = 0; i < NUM_OF_LINES_CONFIGURATION; i++)
    {
        len = strlen(CONFIGURATIONS[i]);
        status = write_serial_port(serial_port,CONFIGURATIONS[i],len);
        if(status != STATUS_OK)
        {
            return status;
        }

        usleep(10000); //sleep for at least 0.02 seconds (part of the protocol i think)
    }

    close(serial_port);
    return STATUS_OK;
}


/*
PARAMS: path to port, char pointer to file name
config radar -> send the settings located on config_file_name to the radar
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_WRITE_TO_RADAR, CANNOT_OPEN_FILE,CANNOT_CLOSE_FILE, CANNOT_ALLOCATE_MEMORY
*/
int config_radar_file(char *DATA_PORT_LOCATION, char* config_file_name)
{
    int status = STATUS_NOT_OK;
    size_t i = 0;
    char **config_file = NULL;
    int serial_port = -1;
    int len = 0;
    //load configuration from the file
    status = get_config_file_by_lines(config_file_name, &config_file);
    if(status != STATUS_OK)
    {
        return status;
    }

    status = open_config_port(DATA_PORT_LOCATION, &serial_port);
    if(status != STATUS_OK)
    {
        return status;
    }    

    //send configuration to the radar
    for (i = 0; i < NUM_OF_LINES_CONFIGURATION; i++)
    {
        len = strlen(config_file[i]);
        status = write_serial_port(serial_port,config_file[i],len);
        if(status != STATUS_OK)
        {
            dispose_config_file(config_file);
            return status;
        }

        usleep(10000); //sleep for at least 0.02 seconds (part of the protocol i think)
    }

    status = dispose_config_file(config_file);
    if(status != STATUS_OK)
    {
        return status;
    }    

    close(serial_port);
    return STATUS_OK;
}