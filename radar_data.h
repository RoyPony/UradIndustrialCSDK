#ifndef RADAR_DATA_H
#define RADAR_DATA_H

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include "data_structs.h"
#include "serial_data.h"
#include "config_radar.h"


//macros
#define CHECK_EMPTY(point) ((point)->point_x_y_z.x == 0 && (point)->point_x_y_z.y == 0 && (point)->point_x_y_z.z == 0)

//defines
#define SIZE_OF_DETECTED_POINT (sizeof(struct expanded_detected_point))
#define CONFIG_FILE_PATH "./chirp_config.cfg" 

/*
PARAMS: pointer to  record to dispose
dispose record and his fields
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int free_record(struct record *rec_2_free);

/*
PARAMS: int serial port, pointer to bool
check radar communication and set valid as well
RETURN VALUE: 
        param convert to radians from degrees 
*/
int check_radar_commuincation(int serial_port, bool *valid);

/*
PARAMS: int port number, pointer to master_header to store the received header
read header from port until the sync (MAGIC KEY) is correct and store the header received at header 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR
*/
int read_header_file(int serial_port,struct master_header *header);

/*
PARAMS: pointer to the detected point, pointer to bool to store true if valid or false if not
validate that the point is good 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int validate_point(struct detected_point *point, bool *valid);

/*
PARAMS: pointer to the detected point, pointer to lcoation to store the 3d location
calculate the 3d location of the detected point, store it in location 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int get_location_3d(struct detected_point *point, struct location_3d *location);

//get one record from data buffer the gets from the radar

/*
PARAMS: pointer to data, pointer to offset start from data, int number of objects, pointer to pointer to store the record at
get record from data buffer
~ inner allocated memory struct record ~
~ inner allocated memort expandedpoints arr in struct record ~ 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_ALLOCATE_MEMORY
*/
int get_record_from_data(char *data,int *offset,int num_of_objects_detected, struct record** rec);

/*
PARAMS: int data port, pointer to pointer to store the record at
get record from the radar
~ inner allocated memory struct record ~
~ inner allocated memory expandedpoints arr in struct record ~ 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_ALLOCATE_MEMORY, CANNOT_READ_FROM_RADAR, BAD_TLV, POINTS_TLV_NOT_FOUND
*/
int get_record_from_radar(int data_port, struct record** rec);

/*
PARAMS: data port, pointer to pointer to record to store the record 
get valid record from radar
~ inner allocated memory struct record ~
~ inner allocated memory expanded points arr in struct record ~ 
RETURN VALUE: 
    SUCCES: STATUS_OK
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR, CANNOT_ALLOCATE_MEMORY
*/ 
int get_good_record_from_radar(int data_port, struct record **record);

#endif