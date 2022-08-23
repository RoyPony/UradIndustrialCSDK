#ifndef CONFIG_RADAR_H
#define CONFIG_RADAR_H

/*
PARAMS: path to port
config radar -> send the settings located on CONFIGURATIONS to the radar
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_WRITE_TO_RADAR
*/
int config_radar_arr(char *DATA_PORT_LOCATION);

/*
PARAMS: path to port, char pointer to file name
config radar -> send the settings located on config_file_name to the radar
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_WRITE_TO_RADAR, CANNOT_OPEN_FILE,CANNOT_CLOSE_FILE, CANNOT_ALLOCATE_MEMORY
*/
int config_radar_file(char *DATA_PORT_LOCATION, char* config_file_name);

#endif
