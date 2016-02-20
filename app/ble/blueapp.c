/*
 * Copyright (c) 2016 Eric Holland
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <app.h>
#include <debug.h>
#include <compiler.h>


#include <compiler.h>
#include <kernel/thread.h>
#include <lib/ble.h>
#include <platform/nrf51.h>
#include <platform/nrf51_radio.h>
#include <platform.h>
#include <platform/timer.h>

#include <dev/ble_radio.h>

#include <target/gpioconfig.h>
#include <dev/gpio.h>


//   UUID  - 080223be-181a-4f59-b74a-ea5d04af35bc
 const uint8_t uuid1[16] =   {0xbc, 0x35, 0xaf, 0x04, \
                            0x5d, 0xea, 0x4a, 0xb7, \
                            0x59, 0x4f, 0x1a, 0x18, \
                            0xbe, 0x23, 0x02, 0x08 };


static ble_t ble1;
static thread_t *blethread;
static const char lkbeacon[] = "LK";


static void ble_init(const struct app_descriptor *app);
uint32_t ble_radio_scan_continuous(ble_t * ble_p, lk_time_t timeout);

void ble_stop(void) {
    ble_go_idle(&ble1);

}


void ble_scan(void) {
    printf("Starting BLE Scanning...\n");
    ble1.state = BLE_START_SCANNING;
}


#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("ble-scan", "Start BLE Scanning", (console_cmd)&ble_scan)
STATIC_COMMAND("ble-stop", "Stop BLE Activity", (console_cmd)&ble_stop)
STATIC_COMMAND_END(bletests);

#endif



static int ble_run(void * args)
{
    int32_t i =0;
    ble_initialize( &ble1 );
    while(1) {
        switch (ble1.state) {
            case BLE_START_SCANNING:
            case BLE_SCANNING:

                ble1.channel_index = 39;
                i =  ble_radio_scan_continuous(&ble1,3000);
                if ((i==0) && (ble1.payload)) {
                    printf("%llu: ",current_time_hires());
                    for (int x=0; x < ble1.payload_length; x++) {
                        printf("%02x",ble1.payload[x]);
                    }
                    printf("\n");
                 } else {
                    printf("timed out -%x\n",(uint32_t)ble1.payload);
                 }
                 break;
            case BLE_START_ADVERTISING:
            case BLE_ADVERTISING:

                ble_init_adv_nonconn_ind(&ble1);
                ble_gap_add_flags(&ble1);
                ble_gap_add_shortname(&ble1, lkbeacon, sizeof(lkbeacon)-1);
                //ble_gap_add_service_data_128(&ble1, uuid1, i++);
                ble1.scannable = true;
        //TODO - need a way to timeout the rx after tx when we don't get a scan request.
        //          if we accept connections or scans, we should do shortcut to enable rx after
        //          we disable on tx.
                ble1.channel_index = 37;
                ble_radio_tx(&ble1);
                ble1.channel_index = 38;
                ble_radio_tx(&ble1);
                ble1.channel_index = 39;
                ble_radio_tx(&ble1);
                thread_sleep(1000);
                break;
            default:
                thread_sleep(1000);
                break;
                //thread_yield();
        }
    }
	return 0;
}


static void ble_init(const struct app_descriptor *app) {
    
	blethread = thread_create("blethread", &ble_run, NULL, HIGH_PRIORITY, DEFAULT_STACK_SIZE);
	thread_resume(blethread);
}


APP_START(bletests)
	.init = ble_init,
	.flags = 0,
APP_END

