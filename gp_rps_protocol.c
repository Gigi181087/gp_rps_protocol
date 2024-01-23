#include <malloc.h>
#include <stdlib.h>
#include "gp_rps_protocol.h"

struct rps_protocol {
  uint16_t own_id;
  gp__rps_protocol__module_type_t module_type;
};

uint8_t gp__rps_protocol__init(gp__rps_protocol__t** module_param, gp__rps_protocol__module_type_t module_type_param) {

  return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}

uint8_t gp__rps_protocol__destroy(gp__rps_protocol__t** module_param) {
  free(*module_param);
  

  return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}

void gp__rps_protocol__handle(gp__rps_protocol__t* module_param, uint64_t system_time_param) {

  return;
}

uint8_t gp__rps_protocol__receive_message(gp__rps_protocol__t* module_param, uint8_t* message, uint16_t message_length_param) {

return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}
