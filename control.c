#include "errors.h"
#include "radar_data.h"

/*
PARAMS: pointer to struct expanded_detected_point  
print point
RETURN VALUE: 
    SUCCES: STATUS_OK
    FAILURE: PARAMETER_NOT_VALID  
*/ 
int print_point(struct expanded_detected_point *point)
{
    struct detected_point *dec_point = NULL;

    //check for valid parameters
    if(point == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    dec_point = &(point->detec_point);
    
    printf("x: %1.3f m, y: %1.3f m, z: %1.3f m, v: %1.3f, distance: %1.3f",dec_point->point_x_y_z.x,dec_point->point_x_y_z.y,dec_point->point_x_y_z.z,dec_point->doopler,point->location.distance);
    printf(", azimuth angle: %f,  elevation angle: %f\n",point->location.azimuth,point->location.elevation);
    
    return STATUS_OK;
}

/*
PARAMS: pointer to record 
print all points in the record
RETURN VALUE: 
    SUCCES: STATUS_OK
    FAILURE: PARAMETER_NOT_VALID  
*/ 
int print_record(struct record *rec)
{
    int status = STATUS_NOT_OK;
    size_t i = 0;
    struct expanded_detected_point *curr_point = NULL;

    //check for valid parameters
    if(rec == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    curr_point = rec->points_arr;
    for (i = 0; i < rec->num_of_detected_points; i++)
    {
        // status = print_point(curr_point);
        printf("%3.5f,", curr_point->location.azimuth);
        curr_point++;
    }
    // printf("--------------\n");

    return STATUS_OK;
}


/*
PARAMS: None
start communicate
RETURN VALUE: 
    SUCCES: STATUS_OK
    FAILURE: PARAMETER_NOT_VALID  
*/ 
int start_communicate()
{
    int status = STATUS_NOT_OK;
    int data_port = 0;
    char *CONFIG_PORT_LOCATION = "/dev/ttyUSB0";
    char *DATA_PORT_LOCATION = "/dev/ttyUSB1";
    bool comm_valid = false;
    struct record *curr_record = NULL;

    //"open" data port
    status = open_data_port(DATA_PORT_LOCATION, &data_port);
    if(status != STATUS_OK)
    {
        printf("cannot open data port\n");
        printf("ERROR: %d\n", status);
        return status;     
    }
    printf("data port: %d\n", data_port);

    // "open" radar port,config radar - send configuration to radar
    status = config_radar_arr(CONFIG_PORT_LOCATION);
    if(status != STATUS_OK)
    {
        printf("ERROR: %d\n", status);
        close(data_port);
        return status;     
    }
    printf("config port\n");

    //check radar communication is valid
    status = check_radar_commuincation(data_port, &comm_valid);
    if(status != STATUS_OK)
    {
        close(data_port);
        return status;
    }
    printf("CHECK COMMUNICATION: %s\n", comm_valid ? "SUCCESSED" : "FAILED");
    if(!comm_valid)
    {
        printf("--------------\n");
        return COMM_NOT_VALID;
    }

    int count = 0;
    printf("[");
    while(count < 30)
    {
        count++;
        status = get_good_record_from_radar(data_port, &curr_record);
        if(status != STATUS_OK)
        {
            printf("ERROR: %d\n", status);
            break;
        }
        print_record(curr_record);
        free_record(curr_record);
    }
    printf("]");

    return STATUS_OK;
}

/*
PARAMS: None
Start system
RETURN VALUE: 
    SUCCES: STATUS_OK
    FAILURE: PARAMETER_NOT_VALID  
*/ 
int start()
{
   int status = STATUS_NOT_OK;
    size_t i = 0;

    for (i = 0; i < 10; i++)
    {
        status = start_communicate();
        if(status != COMM_NOT_VALID)
        {
            return status;
        }
    }

    return status;
}

int main()
{
    return start();
}