/**
 * @file ble_app.c
 * @brief ble play main application file
 *
 */


#include <stdio.h>
#include "BCDS_Basics.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "BLE_stateHandler_ih.h"
#include "BLE_serialDriver_ih.h"
#include "BleAlpwDataExchange_Server.h"
#include "BleBattery_Service.h"

#include "XdkApplicationTemplate.h"

#include "BCDS_Power.h"
#include "BCDS_BatteryMonitor.h"



/** task creation parameters */
#define BLE_APP_STK_SIZE		1024
#define BLE_APP_PRIO			(configMAX_PRIORITIES - 16)
#define BLE_SEND_DATA_PERIOD	1000


/* ble device name */
const static uint8_t ble_dev_name[] = "ulipe_xdk_ble";
static TickType_t elapsed_ticks = 0;
static uint8_t received_dat[128] = {0};
static uint8_t transmit_buf[128] = {0};


/**
 * @brief batt status callback
 */
static Battery_callback batt_cb(Battery_State_T batt_state)
{
	(void)batt_state;
}

/**
 * @brief sends a packet via BLE interface
 */
static void ble_send_packet(void)
{
	bool send = false;

	/* check if is time to send data */
	if((xTaskGetTickCount() - elapsed_ticks) >= BLE_SEND_DATA_PERIOD) {
		elapsed_ticks = xTaskGetTickCount();
		send = true;
	}

	if(send) {
		BleStatus sts;
		memset(transmit_buf, 0, sizeof(transmit_buf));
		int size = sprintf(transmit_buf, "xdk bosch ble test, actual timestamp: %d ", elapsed_ticks);
		sts = BLE_sendData(transmit_buf, size);
		assert(sts != BLESTATUS_FAILED);


		//BLEBATTERYSERVICE_SetBatteryLevel((uint8_t)Battery_readBatteryLevel());
		//assert(sts != BLESTATUS_FAILED);


	}

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

	//sts = BLEBATTERYSERVICE_Register();
	//assert(sts != BLESTATUS_FAILED);


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
		ble_send_packet();
	}
}



/**
 * @brief prepare whole appplication
 */
void appInitSystem(xTimerHandle xTimer)
{
    (void) (xTimer);

    Battery_init(batt_cb);

    /* creates and starts the ble main application task */
    BaseType_t err = xTaskCreate(ble_app_task, "ble_app_task", BLE_APP_STK_SIZE,
    		BLE_APP_PRIO, NULL, NULL);
    assert(err == pdPASS);
}

/** ************************************************************************* */
