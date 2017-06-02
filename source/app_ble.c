/*
 * app_ble.c
 *
 *  Created on: 01/06/2017
 *      Author: root
 */
#include <stdio.h>
#include "k_list.h"
#include "BCDS_Basics.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "BLE_stateHandler_ih.h"
#include "BLE_serialDriver_ih.h"
#include "BleAlpwDataExchange_Server.h"
#include "app_ble.h"




/** task creation parameters */
#define BLE_APP_STK_SIZE		1024
#define BLE_APP_PRIO			(configMAX_PRIORITIES - 1)
#define BLE_QUEUE_SLOTS			(32)
#define BLE_QUEUE_IPC_SLOTS     (4)


/** static variables */
const static uint8_t ble_dev_name[] = "ulipe_xdk_ble";
static QueueHandle_t ble_tx_queue;
static QueueHandle_t ble_rx_queue;



/**
 * @brief sends a packet via BLE interface
 */
static void ble_send_packet(void)
{

		BleStatus sts;
		sts = BLE_sendData(transmit_buf, size);
		assert(sts != BLESTATUS_FAILED);
}

/**
 * @brief notification callback function
 */
static void ble_notify_cb(BLE_connectionDetails_t conn)
{
	/* process ccore stack global events */

	switch(conn.connectionStatus) {
		case BLE_CONNECTED_TO_DEVICE:
			printf( "Device connected: %02x:%02x:%02x:%02x:%02x:%02x \r\n",
				 conn.remoteAddress.addr[0],
				 conn.remoteAddress.addr[1],
				 conn.remoteAddress.addr[2],
				 conn.remoteAddress.addr[3],
				 conn.remoteAddress.addr[4],
				 conn.remoteAddress.addr[5]);
			break;

		case BLE_DISCONNECTED_FROM_DEVICE:
			printf("Device disconnected!\r\n");
			break;


		default:
			break;
	}

}

/**
 * @brief callback related to alpwise data exch svc
 */
static void ble_alpw_data_xch_cb(BleAlpwDataExchangeEvent ev, BleStatus sts,
			void *params)	{
	if((ev == BLEALPWDATAEXCHANGE_EVENT_RXDATA) &&
				(sts == BLESTATUS_SUCCESS) && (params != NULL)) {
		/* extract data arrived from client */
		BleAlpwDataExchangeServerRxData *rx =
				(BleAlpwDataExchangeServerRxData *) params;

		memcpy(&received_dat, rx->rxData, rx->rxDataLen);
		received_dat[rx->rxDataLen] = '\0';
		printf("Data sent by client: %s \n", received_dat);
	}

}

/**
 * @brief service registration callback
 */
static void ble_app_service_register_cb (void)
{
	BleStatus svc_status;
	svc_status = BLEALPWDATAEXCHANGE_SERVER_Register(ble_alpw_data_xch_cb);
	assert(svc_status == BLESTATUS_SUCCESS);
}


/**
 * @brief initializes the ble stack
 */
static int ble_app_init(void)
{
	int err = 0;

	BLE_notification_t cfg_params;
	BLE_returnStatus_t ble_ret;

	/* prepare ble parameters */
	cfg_params.callback = ble_notify_cb;
	cfg_params.enableNotification = BLE_ENABLE_NOTIFICATION;
	ble_ret = BLE_enablenotificationForConnect(cfg_params);
	assert(ble_ret != BLE_INVALID_PARAMETER);


	/* register data exchange service */
	BLE_status sts = BLE_customServiceRegistry(ble_app_service_register_cb);
	assert(sts != BLESTATUS_FAILED);


	/* sets the device name */
	ble_ret = BLE_setDeviceName(ble_dev_name, sizeof(ble_dev_name));
	assert(ble_ret != BLE_INVALID_PARAMETER);

	/* initializes ble core stack */
	sts = BLE_coreStackInit();
	assert(sts != BLESTATUS_FAILED);



	return(err);
}


/**
 * @brief main ble application task
 */
static void ble_app_task(void *args)
{

	BLE_return_t ble_ret;
	BLE_appStateReturn ble_state_ret;

	int err = ble_app_init();
	assert(err == 0);

	for(;;) {

		/* run the HCI and core processing stack */
		ble_ret = BLE_hciReceiveData();
		ble_state_ret = BLE_coreStateMachine();
	}
}


/*
 * Public functions
 */

void app_ble_init(void)
{
	portBASE_TYPE err;

	/* create queue/timers for communication channel */
	ble_tx_queue = xQueueCreate(BLE_QUEUE_SLOTS, sizeof(ble_data_t));
	assert(ble_tx_queue != NULL);

	ble_rx_queue = xQueueCreate(BLE_QUEUE_SLOTS, sizeof(ble_data_t));
	assert(ble_rx_queue != NULL);

	ble_timeout = xTimerCreate()



}

int app_ble_send(void *data, uint32_t size, uint8_t msg_id)
{

}

int app_ble_recv(void *data, uint32_t expected_size, uint8_t msg_id, uint32_t timeout)
{

}

