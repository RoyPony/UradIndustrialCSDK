#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include "data_structs.h"
#include "radar_data.h"
#include "errors.h"

//defines
#define SYNC_PATTERN 0x708050603040102 //sync number for header
#define TLV_HEADER_LEN 8
#define MASTER_HEADER_SIZE sizeof(struct master_header)
#define MAX_AXIS_VALUE_XYZ 100
#define MIN_AXIS_VALUE_XYZ -100 
#define MAX_LENGTH 1000

/*
PARAMS: int degrees in radians
convert the given radian parameters to degrees, return the value 
RETURN VALUE: 
        param convert to radians from degrees 
*/
float to_degree(float rad)
{
    return rad * (180.0 / M_PI);
}


/*
PARAMS: pointer to  record to dispose
dispose record and his fields
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int free_record(struct record *rec_2_free)
{
    if(rec_2_free == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    free(rec_2_free->points_arr);
    free(rec_2_free);

    return STATUS_OK;
}

/*
PARAMS: int serial port, pointer to bool
check radar communication and set valid as well
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR
*/
int check_radar_commuincation(int serial_port, bool *valid)
{
    int status = STATUS_NOT_OK;
    struct master_header header;
    char buffer[1000];

    //check for valid parameters
    if(serial_port < 0 || valid == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }
    
    for (int i = 0; i < 10; i++)
    {
        status = read_header_file(serial_port, &header);
        if(status != STATUS_OK)
        {
            *valid = false;
            return STATUS_OK;
        }

        status = read_serial_port(serial_port, buffer, header.total_package_length - MASTER_HEADER_SIZE);
        if(status != STATUS_OK)
        {
            *valid = false;
            return STATUS_OK;
        }
    }

    *valid = true;

    return STATUS_OK;
}

/*
PARAMS: int port number, pointer to master_header to store the received header
read header from port until the sync (MAGIC KEY) is correct and store the header received at header 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR
*/
int read_header_file(int serial_port,struct master_header *header)
{   
    int status = STATUS_NOT_OK;
    int not_send_data_count = 0;
    int missing_data_count = 0;
    int count = 0;

    //check for valid parameters
    if(serial_port < 0 || header == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    // read header until the sync is ok
    status = read_serial_port(serial_port, header, MASTER_HEADER_SIZE); //read header
    if(status != STATUS_OK && status != RADAR_NOT_SEND_DATA) 
    {
        return status;
    }

    while(header->magic_word != SYNC_PATTERN)
    {
        if(status == RADAR_NOT_SEND_DATA)
        {
            not_send_data_count++;
        }
        else if(status == RADAR_MISSING_DATA)
        {
            missing_data_count++;
        }
        count++;

        status = read_serial_port(serial_port, header, MASTER_HEADER_SIZE); //read header
        if(status != STATUS_OK && status != RADAR_NOT_SEND_DATA) 
        {
            return status;
        }
    }
    count++;

    return STATUS_OK;
}

/*
PARAMS: pointer to the detected point, pointer to bool to store true if valid or false if not
validate that the point is good 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int validate_point(struct detected_point *point, bool *valid)
{
    //check for valid parameters
    if(point == NULL || valid == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    //check empty point
    if(CHECK_EMPTY(point))
    {
        *valid = false;
        return STATUS_OK; 
    }

    //check x,y,z values
    if(point->point_x_y_z.x > MAX_AXIS_VALUE_XYZ || point->point_x_y_z.y > MAX_AXIS_VALUE_XYZ || point->point_x_y_z.z > MAX_AXIS_VALUE_XYZ || point->point_x_y_z.x < MIN_AXIS_VALUE_XYZ || point->point_x_y_z.y < MIN_AXIS_VALUE_XYZ || point->point_x_y_z.z < MIN_AXIS_VALUE_XYZ )
    {
        *valid = false;
        return STATUS_OK; 
    }

    //check doopler is heigher, 100 is not based on something
    if(point->doopler > 100 || point->doopler < -100)
    {
        *valid = false;
        return STATUS_OK; 
    }

    *valid = true;
    return STATUS_OK;
}

/*
PARAMS: pointer to the detected point, pointer to lcoation to store the 3d location
calculate the 3d location of the detected point, store it in location 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID
*/
int get_location_3d(struct detected_point *point, struct location_3d *location)
{
    //check for valid parameters
    if(point == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    float x = point->point_x_y_z.x;
    float y = point->point_x_y_z.y;
    float z = point->point_x_y_z.z;
    float xp = powf(x,2);
    float yp = powf(y,2);
    float zp = powf(z,2);
    float distance_from_radar = 0;
    float azimuth = 0;
    float elevation = 0;

    //check for valid parameters
    if(location == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    //distance from radar
    distance_from_radar = sqrtf(xp + yp + zp);
    location->distance = distance_from_radar;

    //azimuth
    azimuth = atanf(y/x);
    azimuth = to_degree(azimuth);
    location->azimuth = azimuth;

    //elevation
    elevation = atanf(y/z);
    elevation = to_degree(elevation); 
    location->elevation = elevation;

    return STATUS_OK;
}

/*
PARAMS: pointer to data, pointer to offset start from data, int number of objects, pointer to pointer to store the record at
get record from data buffer
~ inner allocated memory struct record ~
~ inner allocated memort expandedpoints arr in struct record ~ 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_ALLOCATE_MEMORY
*/
int get_record_from_data(char *data,int *offset,int num_of_objects_detected, struct record** rec)
{
    int status = STATUS_NOT_OK;
    int removed_points = 0;
    int curr_point_index = 0;
    size_t i = 0;
    time_t time_in_seconds = 0;
    bool valid_point = false;
    struct expanded_detected_point point;
    
    // check for valid parameters
    if(data == NULL || offset == NULL || rec == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }
    else if(num_of_objects_detected <= 0)
    {
        return RECORD_WITHOUT_POINTS;
    }

    // allocate record and his fields
    *rec = malloc(sizeof(struct record));
    if(*rec == NULL)
    {
        return CANNOT_ALLOCATE_MEMORY;
    }
    (*rec)->points_arr = malloc(sizeof(struct expanded_detected_point) * num_of_objects_detected);
    if((*rec)->points_arr == NULL)
    {
        free(*rec);
        return CANNOT_ALLOCATE_MEMORY;
    }

    //intialize time
    time_in_seconds = time(NULL);
    (*rec)->record_time = time_in_seconds;

    //check detected points
    removed_points = 0;
    curr_point_index = 0;
    for (i = 0; i < num_of_objects_detected; i++)
    {
        // zero point 
        memset(&point, 0, sizeof(point));

        point.detec_point = *((struct detected_point*)(data + (*offset)));
        status = get_location_3d(&point.detec_point,&point.location);
        if(status != STATUS_OK)
        {
            return status;
        }
        (*offset) += sizeof(struct detected_point);
        
        // check point
        status = validate_point(&point.detec_point, &valid_point);
        if(status != STATUS_OK)
        {
            return status;
        }

        if(valid_point)
        {
            (*rec)->points_arr[curr_point_index] = point;
            curr_point_index++;
        }
        else
        {
            removed_points++;
            memset(((*rec)->points_arr + i),0,SIZE_OF_DETECTED_POINT);
        }
    }


    //intialize num of detected points
    (*rec)->num_of_detected_points = curr_point_index;

    return STATUS_OK;
}

/*
PARAMS: int data port, pointer to pointer to store the record at
get record from the radar and store his address in rec
~ inner allocated memory struct record ~
~ inner allocated memory expanded points arr in struct record ~ 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_ALLOCATE_MEMORY, CANNOT_READ_FROM_RADAR, BAD_TLV, POINTS_TLV_NOT_FOUND
*/
int get_record_from_radar(int data_port, struct record** rec)
{
    int status = STATUS_NOT_OK;
    int offset = 0;
    u_int data_length = 0;
    size_t i = 0;
    struct tlv *curr_tlv = NULL;
    struct master_header header;

    //check for valid parameters
    if(data_port < 0 || rec == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    // printf("----||----\n");

    // read header file until magic word is ok
    memset(&header, 0, sizeof(header));
    status = read_header_file(data_port, &header);
    if(status != STATUS_OK)
    {
        return status;
    }

    //calculate data_length
    data_length = header.total_package_length - MASTER_HEADER_SIZE; 

    //read data from radar
    char data[(int)data_length];
    status = read_serial_port(data_port, data, (int)data_length);
    if(status != STATUS_OK && status != RADAR_MISSING_DATA) // ! RADAR_NOT_SEND_DATA
    {
        return status;
    }

    //move on tlv`s
    for (i = 0; i < header.num_of_tlv_items; i++)
    {
        //get current tlv from data
        curr_tlv = (struct tlv*)(data+offset);
        offset += sizeof(struct tlv);

        //check for tlv errors
        if(curr_tlv->type > 20 || curr_tlv->length > 10000)
        {
            return BAD_TLV;
        }

        if(curr_tlv->type == DETECTED_POINTS_TYPE)
        {
            status = get_record_from_data(data, &offset, (int)header.num_of_objects_detected, rec);
            if(status != STATUS_OK)
            {
                return status;
            }

            return STATUS_OK;
        }
        else
        {
            offset += curr_tlv->length;
        }
    }

    return POINTS_TLV_NOT_FOUND;
}

/*
PARAMS: data port, pointer to pointer to record to store the record 
get valid record from radar
~ inner allocated memory struct record ~
~ inner allocated memory expanded points arr in struct record ~ 
RETURN VALUE: 
    SUCCES: STATUS_OK
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR, CANNOT_ALLOCATE_MEMORY
*/ 
int get_good_record_from_radar(int data_port, struct record **record)
{
    int status = STATUS_NOT_OK;
    struct record *rec = NULL;
    
    //check for valid parameters
    if(data_port < 0 || record == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    status = get_record_from_radar(data_port, record);

    while(status != STATUS_OK)
    {
        if(status != RECORD_WITHOUT_POINTS && status != RADAR_MISSING_DATA && status != RADAR_NOT_SEND_DATA)
        {
            return status;
        }
        status = get_record_from_radar(data_port, record);
    }

    return STATUS_OK;
}
