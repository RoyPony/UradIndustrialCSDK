#ifndef SERIAL_DATA_H
#define SERIAL_DATA_H

/*
PARAMS: path to port, int pointer to store the port
open radar configuration port and store it in serial_port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int open_config_port(char *CONFIG_PORT_LOCATION, int *serial_port);

/*
PARAMS: path to port, int pointer to store the port
open radar data port and store it in serial_port (2 seconds sleep inside the func for flushing data buffer)
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int open_data_port(char *DATA_PORT_LOCATION, int *serial_port);

/*
PARAMS: int serial port number, void pointer to a data buffer, int size - bytes to write to port 
write size bytes from data buffer to serial port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_WRITE_TO_RADAR
*/
int write_serial_port(int serial_port, void *data_buffer, int size);

/*
PARAMS: int serial port number, void pointer to a data buffer, int size - bytes to write to port 
read size bytes from data buffer to serial port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR
*/
int read_serial_port(int serial_port, void *data_buffer, int size);

/*
PARAMS: int serial port number
close serial port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int dispose_port(int serial_port);

#endif