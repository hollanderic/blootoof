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
#include <lib/ble.h>

static void _ble_print_gap_fields(ble_t * ble_p, uint32_t index);

static inline void _print_hw_addr(uint8_t * buff_p){
	printf("%02x:%02x:%02x:%02x:%02x:%02x",  buff_p[5], \
 											 buff_p[4], buff_p[3], \
 											 buff_p[2], buff_p[1], \
 											 buff_p[0]);
}

static inline void _hexdump(uint8_t * buff, uint8_t len) {
	for (int i=0; i < len ; i++) {
		printf("%02x",buff[i]);
	}
}
static inline void _chardump(uint8_t * buff, uint8_t len) {
	for (int i=0; i < len ; i++) {
		printf("%c",buff[i]);
	}
}

void ble_print_packet(ble_t * ble_p) {

 	switch (ble_p->payload[0] & 0x0F) {
 		case PDU_ADV_IND:
 			printf("Indirect Advertisement (ADV_IND)\n  ");
 			_print_hw_addr(&ble_p->payload[2]);
 			printf(" ----packet length = %u bytes\n",ble_p->payload_length);
 			_ble_print_gap_fields(ble_p,8); 
 			break;
  		case PDU_ADV_NONCONN_IND:
 			printf("Non-connectable Indirect Advertisement (ADV_NONCONN_IND)\n  ");
 			_print_hw_addr(&ble_p->payload[2]);
 			printf(" ----packet length = %u bytes\n",ble_p->payload[1]);
 			_ble_print_gap_fields(ble_p,8); 
 			break;
 		case PDU_ADV_SCAN_IND:
 			printf("Scannable Indirect Avertisement (ADV_SCAN_IND)\n  ");
 			_print_hw_addr(&ble_p->payload[2]);
 			printf(" ----packet length = %u bytes\n",ble_p->payload[1]);
 			break;
 		case PDU_SCAN_RSP:
 			printf("SCAN_RSP        - ");
 			_print_hw_addr(&ble_p->payload[2]);
 			printf(" ----packet length = %u bytes\n",ble_p->payload[1]);
 			break;



 		default:
 			printf("Unknown - 0x%02x  ",ble_p->payload[0]);
 			printf(" ----packet length = %u bytes\n",ble_p->payload[1]);
 			break;
 	}

 }

static inline uint32_t _ble_print_gadname(ble_t * ble_p, uint8_t index) {
	
	printf("\tGAP_ADTYPE_LOCAL_NAME_COMPLETE : ");
	_chardump( &ble_p->payload[index+2] , ble_p->payload[index]-1);
	printf("\n");
	return ble_p->payload[index] +1;

}

static inline uint32_t _ble_print_gadnameshort(ble_t * ble_p, uint8_t index) {

	printf("\tGAP_ADTYPE_LOCAL_NAME_SHORT : ");
	_chardump( &ble_p->payload[index+2] , ble_p->payload[index]-1);
	printf("\n");
	return ble_p->payload[index] +1;
}

static inline uint32_t _ble_print_gadtxpower(ble_t * ble_p, uint8_t index) {
	if ( ble_p->payload[index] != 2) 
		return 255;  //corrupt if len!=2
	printf("\tGAP_ADTYPE_POWER_LEVEL: %d dBm\n" , (int8_t)ble_p->payload[index+2] );
	return 3;

}
static inline uint32_t _ble_print_gadms(ble_t * ble_p, uint8_t index) {
		
	printf("\tGAP_ADTYPE_MANUFACTURER_SPECIFIC (%d bytes):" , ble_p->payload[index] );
	_hexdump(&(ble_p->payload[index + 2]),ble_p->payload[index]);
	printf("\n");
	return ble_p->payload[index] +1;  //return number of bytes handled
}

static inline uint32_t _ble_print_gadflags(ble_t * ble_p, uint8_t index) {
	if ( ble_p->payload[index] != 2) 
		return 255;  //corrupt flag type if len!=2
	printf("\tGAP_ADTYPE_FLAGS :");
	if ( ble_p->payload[index + 2 ] & 0x01 ) printf("LE Limited Disc : ");
	if ( ble_p->payload[index + 2 ] & 0x02 ) printf("LE General Disc : ");
	if ( ble_p->payload[index + 2 ] & 0x04 ) printf("No BR/EDR : ");
	if ( ble_p->payload[index + 2 ] & 0x08 ) printf("Simult LE BR/EDR(cntrlr) : ");
	if ( ble_p->payload[index + 2 ] & 0x10 ) printf("Simult LE BR/EDR(host) : ");
	printf("\n");
	return 3;
}

static void _ble_print_gap_fields(ble_t * ble_p, uint32_t index) {

	while (index < ble_p->payload_length) {
		if ((index + ble_p->payload[index]) > ble_p->payload_length + 2) {
			index=256;
		} else {
			switch (ble_p->payload[index + 1]) {
				case GAP_ADTYPE_MANUFACTURER_SPECIFIC:
					index = index + _ble_print_gadms(ble_p,index);
					break;
				case GAP_ADTYPE_FLAGS:
					index = index + _ble_print_gadflags(ble_p,index);
					break;
				case GAP_ADTYPE_LOCAL_NAME_SHORT:
					index = index + _ble_print_gadnameshort(ble_p,index);
					break;
				case GAP_ADTYPE_LOCAL_NAME_COMPLETE:
					index = index + _ble_print_gadname(ble_p,index);
					break;
				case GAP_ADTYPE_POWER_LEVEL:
					index = index + _ble_print_gadtxpower(ble_p,index);
					break;

				default:
					printf("\tunhandled type: ");
					for (int i=0; i<(ble_p->payload[index] +1 ) ; i++) {
						printf("%02x",ble_p->payload[index+i]);
					}
					printf("\n");
					index = index + ble_p->payload[index] + 1;
			}
		}
		if (index > 255)  // bit hacky error catch here, but ble buffers are always going to be much smaller
			printf("CORRUPT PACKET!\n");




	}


}
