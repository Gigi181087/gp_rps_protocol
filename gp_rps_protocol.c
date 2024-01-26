#include <malloc.h>
#include <stdlib.h>
#include "gp_rps_protocol.h"

#define MESSAGE_TYPE__UNDEFINED   0
#define MESSAGE_TYPE__PING        1
#define MESSAGE_TYPE__LOCATION    2
#define MESSAGE_TYPE__RSSI_DATA   3
#define MESSAGE_TYPE__CONFIG      4

#define BRAODCAST_ID              65535


typedef struct {
  uint16_t id;
  uint8_t rssi;
  uint16_t location;
} rssi_data_t;

struct protocol {
  uint16_t own_id;
  uint16_t gateway_id;
  uint16_t message_id;

  gp__rps_protocol__module_type_t module_type;
  uint16_t cycle_time;
  uint64_t next_step;
  uint16_t max_message_length;

  uint8_t rssi_data_length;
  rssi_data_t* rssi_data;

  uint8_t message_buff_length;
  message_t** message_buff;

  // events
  uint8_t (*send_message)(uint8_t*, uint16_t);
};

typedef struct {
  uint16_t destination_id : 14;
  uint16_t source_id : 14;
  uint16_t message_id;
  uint8_t message_type : 4;
  uint8_t packet_number : 4;
  uint8_t total_packets : 4;
  uint16_t payload_length : 16;
  uint8_t* payload;
} message_t;

static uint8_t send_message(gp__rps_protocol__t* module, message_t* message);
static uint8_t process_message(gp__rps_protocol__t* module, message_t* message, uint8_t rssi_param);

uint8_t gp__rps_protocol__init(gp__rps_protocol__t** module_param, uint16_t own_id_param, gp__rps_protocol__module_type_t module_type_param) {

  return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}

uint8_t gp__rps_protocol__destroy(gp__rps_protocol__t** module_param) {
  free(*module_param);
  

  return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}

uint8_t gp__rps_protocol__handle(gp__rps_protocol__t* module_param, uint64_t system_time_param) {

  if (system_time_param > module_param->next_step) {
    message_t new_message;
    new_message.source_id = module_param->own_id;

    if (module_param->module_type == SENDER) {
      new_message.destination_id = BRAODCAST_ID;
      new_message.source_id = module_param->own_id;
      new_message.message_type = MESSAGE_TYPE__PING;
      new_message.packet_number = 1;
      new_message.total_packets = 1; 
      new_message.payload_length = 0;
      send_message(module_param, &new_message);

    } else if (module_param->module_type == SATELLITE) {
      uint8_t* payload_buffer = NULL;
      uint16_t payload_buffer_length = sizeof(rssi_data_t) * module_param->rssi_data_length;
      
      if(payload_buffer = (uint8_t*)malloc(sizeof(uint8_t) * payload_buffer_length) == NULL) {

        return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
      }
      
      for (uint16_t i = 0; i < module_param->rssi_data_length; i++) {
        payload_buffer[i * 5] = module_param->rssi_data[i].id >> 8;
        payload_buffer[i * 5 + 1] = module_param->rssi_data[i].id & 0x0f;
        payload_buffer[i * 5 + 2] = module_param->rssi_data[i].rssi;
        payload_buffer[i * 5 + 3] = module_param->rssi_data[i].location >> 8;
        payload_buffer[i * 5 + 1] = module_param->rssi_data[i].location & 0x0f;
      }
      new_message.destination_id = module_param->gateway_id;
      new_message.message_id = module_param->message_id;
      new_message.message_type = MESSAGE_TYPE__RSSI_DATA;
      new_message.total_packets = payload_buffer_length / (module_param->max_message_length - 13) + 1;
       
      for (uint8_t i = 0; i < new_message.total_packets; i++) {
        new_message.packet_number = i;

        if ( i + 1 < new_message.total_packets) {
          new_message.payload_length = module_param->max_message_length - 13;
        } else {
          
          new_message.payload_length = payload_buffer_length % (module_param->max_message_length - 13);
        }
        new_message.payload = &payload_buffer[i * (module_param->max_message_length - 13)];
        send_message(module_param, &new_message);
       }
       free(payload_buffer);
    }

    while (system_time_param > module_param->next_step) {
      module_param->next_step += module_param->cycle_time;
    }
  }

  return;
}

uint8_t gp__rps_protocol__receive_message(gp__rps_protocol__t* module_param, uint8_t* message_param, uint16_t message_length_param, uint8_t rssi_param) {

  // check if message has valid minimum size of 13 (headersize without payload)
  if (message_length_param < 13) {

    return GP__RPS_PROTOCOL__ERRORS__MESSAGE_NOT_RPS;
  }

  // check if preamble is correct
  if (message_param[0] != 'R' || message_param[1] != 'P' || message_param[2] != 'S' || message_param[3] != '\0') {

    return GP__RPS_PROTOCOL__ERRORS__MESSAGE_NOT_RPS;
  }
  message_t* message;
  if (message = (message_t*)malloc(sizeof(message_t)) == NULL) {

    return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
  }
  message->destination_id = (message_param[4] << 8 | message_param[5]);

  // check if receipient is correct
  if (message->destination_id != module_param->own_id || message->destination_id != BRAODCAST_ID) {
    free(message);

    return GP__RPS_PROTOCOL__ERRORS__NOT_RECEIPIENT;
  }
  message->source_id = (message_param[6] << 8 | message_param[7]);
  message->message_id = (message_param[8] << 8 | message_param[9]);
  message->packet_number = message_param[10] >> 4;
  message->total_packets = message_param[10] & 0x0f;
  message->payload_length = (message_param[11] << 8 | message_param[12]);

  // check if payload length is plausible
  if (message_length_param - 13 != message->payload_length) {
    free(message);

    return GP__RPS_PROTOCOL__ERRORS__PARSING_MESSAGE_FAILED;
  }
  uint8_t* payload_pointer = NULL;

  if (message->packet_number == 1) {

    if (message->payload = (uint8_t*)malloc(sizeof(uint8_t) * message->payload_length) == NULL) {
      free(message);

      return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
    }
    payload_pointer = message->payload;

    if (message->total_packets != 1) {
      
      if (module_param->message_buff = (message_t**)realloc(module_param->message_buff, sizeof(message_t*) * (module_param->message_buff_length + 1)) == NULL) {
        free(message->payload);
        free(message);

        return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
      }
      module_param->message_buff[module_param->message_buff_length++] = message;
    }

  } else {
    
    for (uint8_t i = 0; i < module_param->message_buff_length; i++) {

      if (module_param->message_buff[i]->message_id == message->message_id && module_param->message_buff[i]->source_id == message->source_id) {
        payload_pointer =  &module_param->message_buff[i]->payload[module_param->message_buff[i]->payload_length];
        module_param->message_buff[i]->packet_number = message->packet_number;
        module_param->message_buff[i]->payload_length += message->payload_length;
        free(message);
        message = module_param->message_buff[i];

        if ((module_param->message_buff[i]->payload = (uint8_t*)realloc(module_param->message_buff[i]->payload, sizeof(uint8_t) * (module_param->message_buff[i]->payload_length + message->payload_length))) == NULL) {

          return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
        }
      }
    }
  }

  for (uint16_t i = 0; i < message->payload_length; i++) {
    payload_pointer[i] = message_param[i + 13];
  }

  if (message->packet_number == message->total_packets) {

    if (message->message_type == MESSAGE_TYPE__PING && module_param->module_type == SATELLITE) {
      
      if ((module_param->rssi_data = (rssi_data_t*)realloc(module_param->rssi_data, sizeof(module_param->rssi_data_length + 1))) == NULL) {

        return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
      }
      module_param->rssi_data[module_param->rssi_data_length].id = message->source_id;
      module_param->rssi_data[module_param->rssi_data_length].location = 0;
      module_param->rssi_data[module_param->rssi_data_length].rssi = rssi_param;
    }
    
    return process_message(module_param, message, rssi_param);
  }

  return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}

static uint8_t send_message(gp__rps_protocol__t* module_param, message_t* message) {
  uint8_t* message_buff = NULL;
  uint16_t message_length;

  if ((message_buff = (uint8_t*)malloc(sizeof(uint8_t) * (message->payload_length + 13))) == NULL) {

    return GP__RPS_PROTOCOL__ERRORS__ALLOC_FAILED;
  }
  message_buff[0] = 'R';
  message_buff[1] = 'P';
  message_buff[2] = 'S';
  message_buff[3] = '\0';
  message_buff[4] = message->destination_id >> 8;
  message_buff[5] = message->destination_id & 0x0f;
  message_buff[6] = message->source_id >> 8;
  message_buff[7] = message->source_id & 0x0f;
  message_buff[8] = message->message_id >> 8;
  message_buff[9] = message->message_id & 0x00ff;
  message_buff[10] = (message->packet_number << 4) | message->total_packets;
  message_buff[11] = message->payload_length >> 8;
  message_buff[12] = message->payload_length & 0x0f;

  for (uint16_t i = 0; i < message->payload_length; i++) {
    message_buff[13 + i] = message->payload[i];
  }
  uint8_t result = module_param->send_message(message_buff, message_length);
  free(message_buff);

  return result;
}

static uint8_t process_message(gp__rps_protocol__t* module_param, message_t* message, uint8_t rssi_param) {

  return GP__RPS_PROTOCOL__ERRORS__NO_ERROR;
}