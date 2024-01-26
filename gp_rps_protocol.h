#ifndef __GP__RPS__PROTOCOL_H_
#define __GP__RPS__PROTOCOL_H_

#include <stdint.h>

#define GP__RPS_PROTOCOL__ERRORS__NO_ERROR                      0
#define GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED                  1
#define GP__RPS_PROTOCOL__ERRORS__MESSAGE_NOT_RPS               2
#define GP__RPS_PROTOCOL__ERRORS__NOT_RECEIPIENT                3
#define GP__RPS_PROTOCOL__ERRORS__PARSING_MESSAGE_FAILED        4 

#ifdef __cplusplus
extern "C" {
#endif


typedef struct protocol gp__rps_protocol__t;

typedef enum {
    GATEWAY = 0,
    SATELLITE = 1,
    SENDER = 2
} gp__rps_protocol__module_type_t;


uint8_t gp__rps_protocol__init_module(gp__rps_protocol__t** module, uint16_t own_id, gp__rps_protocol__module_type_t module_type);

uint8_t gp__rps_protocol__destroy_module(gp__rps_protocol__t** module);

uint8_t gp__rps_protocol__handle(gp__rps_protocol__t* module, uint64_t system_time);

uint8_t gp__rps_protocol__receive_message(gp__rps_protocol__t* module, uint8_t* message, uint16_t message_length, uint8_t rssi);


#ifdef __cplusplus
}
#endif

#endif // __GP__RPS__PROTOCOL_H_
