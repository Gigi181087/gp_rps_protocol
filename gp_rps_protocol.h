#ifndef __GP__RPS__PROTOCOL_H_
#define __GP__RPS__PROTOCOL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct protocol_module gp__rps_protocol__t;


uint8_t gp__rps_protocol__init_module(gp__rps_protocol__t** module);

uint8_t gp__rps_protocol__destroy_module(gp__rps_protocol__t** module);

uint8_t gp__rps_protocol__handle(gp__rps_protocol__t* module, uint64_t system_time);


#ifdef __cplusplus
}
#endif

#endif // __GP__RPS__PROTOCOL_H_
