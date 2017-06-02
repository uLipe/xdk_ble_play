/*
 * app_ble.h
 *
 *  Created on: 01/06/2017
 *      Author: root
 */

#ifndef APP_BLE_H_
#define APP_BLE_H_


/** maximum payload in bytes */
#define PACKET_MAX_PAYLOAD 32


/** packet types */
typedef enum {
	k_command_packet = 0,
	k_data_packet,
}pack_type_t;

/** packet structure */
typedef struct {
	pack_type_t type;
	uint8_t id;
	uint8_t pack_nbr;
	uint8_t pack_data[PACKET_MAX_PAYLOAD];
}ble_data_t;


/** ipc strucutre */
typedef struct {
	QueueHandle_t msg_q;
	uint8_t msg_id;
	k_list_t link;
}ble_ipc;


/*
 * @brief initializes ble channel
 */
void app_ble_init(void);

/*
 * @brief send data to ble channel
 */
int app_ble_send(void *data, uint32_t size, uint8_t msg_id);

/*
 * @brief receive data from ble channel
 */
int app_ble_recv(void *data, uint32_t expected_size, uint8_t msg_id, uint32_t timeout);



#endif /* SOURCE_APP_BLE_H_ */
