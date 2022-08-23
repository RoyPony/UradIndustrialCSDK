#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define DETECTED_POINTS_TYPE  1

struct tlv
{
    uint32_t type; //tlv type 1 - 9
    uint32_t length;  //tlv length
};

struct point
{
    float x;
    float y; 
    float z;
};

struct detected_point
{
    struct point point_x_y_z;
    float doopler;
};

struct location_3d
{
    float azimuth;
    float elevation;
    float distance;
};

struct expanded_detected_point
{
    struct detected_point detec_point;
    struct location_3d location;
};

struct master_header
{
    uint64_t magic_word; //Output buffer magic word (sync word). It is initialized to 0x0102 0x0304 0x0506 0x0708
    uint32_t version; //SDK Version represented as (MajorNum x 2^24 + MinorNum x 2^16 + BugfixNum x 2^8 + BuildNum)
    uint32_t total_package_length; //Total packet length including frame header length in Bytes
    uint32_t platform; //Device type (ex 0xA6843 for IWR6843 devices)
    uint32_t frame_number; //Frame number (resets to 0 when device is power cycled or reset. Not when sensor stop/start is issued.)
    uint32_t time; //Time in CPU cycles when the message was created.
    uint32_t num_of_objects_detected; //Number of detected objects (points) for the frame
    uint32_t num_of_tlv_items; //Number of TLV items for the frame.
    uint32_t sub_frame_number; //0 if advanced subframe mode not enabled, otherwise the sub-frame number in the range 0 to (number of subframes - 1)
};

struct record
{
    int num_of_detected_points;
    struct  expanded_detected_point *points_arr;
    time_t record_time;
};

#endif