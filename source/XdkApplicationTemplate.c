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
#include "XdkApplicationTemplate.h"





/**
 * @brief prepare whole appplication
 */
void appInitSystem(xTimerHandle xTimer)
{
    (void) (xTimer);

    /* creates and starts the ble main application task */
    BaseType_t err = xTaskCreate(ble_app_task, "ble_app_task", BLE_APP_STK_SIZE,
    		BLE_APP_PRIO, NULL, NULL);
    assert(err == pdPASS);
}

/** ************************************************************************* */
