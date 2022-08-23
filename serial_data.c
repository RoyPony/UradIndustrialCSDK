#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <termios.h>
#include "serial_data.h"
#include "errors.h"


/*
PARAMS: int fd, int speed(bytes per second formatted)
set port attributes 
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int set_port_attributes(int fd, int speed)
{
    int status = STATUS_NOT_OK;
    struct termios tty;

    //check for valid parameters
    if(fd < 0 || speed < 0)
    {
        return PARAMETERS_NOT_VALID;
    }

    status = tcgetattr (fd, &tty); 
    if (status != STATUS_OK)
    {
        return CANNOT_GET_PORT_ATTRIBUTES;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= 0;               // pairity is none
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;


    status = tcsetattr (fd, TCSANOW, &tty);
    if (status != STATUS_OK)
    {
        return CANNOT_SET_PORT_ATTRIBUTES;
    }

    return STATUS_OK;
}

/*
PARAMS: path to port, int speed(bytes per second formatted), int pointer to store the port
open radar port, config port attributes and store it in serial_port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int open_port(char *PORT_PATH, int speed, int *serial_port)
{
    int port_fd = -1;
    int status = STATUS_NOT_OK;

    //check for valid parameters
    if(PORT_PATH == NULL || serial_port == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    //open port
    port_fd = open (PORT_PATH, O_RDWR | O_NOCTTY | O_SYNC);
    if (port_fd < 0)
    {
        return CANNOT_OPEN_PORT;
    }

    //set port attributes
    status = set_port_attributes(port_fd, speed); 
    if(status != STATUS_OK)
    {
        close(port_fd);
        return status;
    }

    *serial_port = port_fd;

    return STATUS_OK;
}

/*
PARAMS: path to port, int pointer to store the port
open radar configuration port and store it in serial_port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int open_config_port(char *config_port_path, int *serial_port)
{
    int status = STATUS_NOT_OK;

    //check for valid parameters
    if(config_port_path == NULL || serial_port == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    status = open_port(config_port_path, B115200, serial_port);
    if(status != STATUS_OK)
    {
        return status;
    }

    return STATUS_OK;
}

/*
PARAMS: path to port, int pointer to store the port
open radar data port and store it in serial_port (2 seconds sleep inside the func for flushing data buffer)
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int open_data_port(char *data_port_path, int *serial_port)
{
    int status = STATUS_NOT_OK;

    //check for valid parameters
    if(data_port_path == NULL || serial_port == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    status = open_port(data_port_path, B921600, serial_port);
    if(status != STATUS_OK)
    {
        return status;
    }

    return STATUS_OK;
}

/*
PARAMS: int serial port number, void pointer to a data buffer, int size - bytes to write to port 
write size bytes from data buffer to serial port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_WRITE_TO_RADAR
*/
int write_serial_port(int serial_port, void *data_buffer, int size)
{
    u_int number_of_bytes = 0;
    
    //check for valid parameters
    if(serial_port < 0 || size <= 0 || data_buffer == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    number_of_bytes = write(serial_port, data_buffer,size); //write the data
    if(number_of_bytes != size)
    {
        return CANNOT_WRITE_TO_RADAR;
    }

    return STATUS_OK;
}

/*
PARAMS: int serial port number, void pointer to a data buffer, int size - bytes to write to port 
read size bytes from data buffer to serial port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_READ_FROM_RADAR, RADAR_NOT_SEND_DATA
*/
int read_serial_port(int serial_port, void *data_buffer, int size)
{
    int bytes_read = -1;
    size_t u_size = (size_t)size;
    char *error = NULL;

    //check for valid parameters
    if(serial_port < 0 || size <= 0 || data_buffer == NULL)
    {
        return PARAMETERS_NOT_VALID;
    }

    //read data
    bytes_read = read(serial_port, data_buffer, u_size); 
    if(bytes_read != size) 
    {
        if(bytes_read == -1)
        {
            return CANNOT_READ_FROM_RADAR;
        }
        else if(bytes_read == 0)
        {
            return RADAR_NOT_SEND_DATA;
        }
        else
        {
            // if(size != 40)
            // {
            //     printf("---expected: %d, recieved: %d---\n", size, bytes_read);
            // }
            return RADAR_MISSING_DATA;
        }
    }
    
    return STATUS_OK;
}

/*
PARAMS: int serial port number
close serial port
RETURN VALUE: 
    SUCCES: STATUS_OK 
    FAILURE: PARAMETER_NOT_VALID, CANNOT_OPEN_PORT
*/
int dispose_port(int serial_port)
{
    int status = STATUS_NOT_OK;

    //check for valid parameters
    if(serial_port < 0)
    {
        return PARAMETERS_NOT_VALID;
    }

    status = close(serial_port);
    if(status != STATUS_OK)
    {
        return CANNOT_CLOSE_PORT;
    }

    return STATUS_OK;
}