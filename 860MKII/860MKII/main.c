/*
 * 860MKII.c
 *
 * Created: 2020-05-30 6:11:37 PM
 * Author : jeff
 */ 

//python pyupdi.py -d tiny202 -c COM3 -b 9600 -f ../860MKII.hex

//set BOD fuse
//pymcuprog -t uart -u COM3 -c 9600 -d attiny202 write -m fuses -o 1 -l 0xe4

//-o --fuse offset: 1 is BODCFG
//-l --literal fuse value 0xe4
//0xe4 --> 0b11100100 - BOD LVL 4.2 V, BOD active




#include <avr/io.h>
#define F_CPU 2000000UL
#include <util/delay.h>
#include <avr/eeprom.h>

#define A0 6
#define A1 7
#define Rb 1
#define Yb 2
#define SW_IN 3
#define OUT_CONFIG (1<<A0) | (1<<A1) | (1<<Rb) | (1<<Yb)
#define LP_24 (1<<Rb)
#define LP_12 (1<<Yb) | (1<<A0)
#define BP (1<<A1) | (1<<Rb)
#define HP (1<<A1) | (1<<A0)
#define NUM_MODES 4

//memory management
//#define MEM_LOC 32 //memory slot where mode recall value is stored
#define MEM_END 60 //leave last 3 bytes for memory address storage
#define ADDR_1 61
#define ADDR_2 62
#define ADDR_3 63

int main(void)
{
	
	
	PORTA.DIR = OUT_CONFIG;
	uint8_t port_output[NUM_MODES] = {LP_24, LP_12, BP, HP};
	uint8_t current_sw_state = 0;
	uint8_t previous_sw_state = 0;
	uint8_t counter = NUM_MODES -1; //read previously stored state
	
	//uint8_t EEMEM saved_state_eeprom;
	uint8_t recall_address = 0;
	uint8_t address_01 = eeprom_read_byte(ADDR_1);
	uint8_t address_02 = eeprom_read_byte(ADDR_2);
	uint8_t address_03 = eeprom_read_byte(ADDR_3);
	
	if ((address_01 == address_02) || (address_01 == address_03)) {
			recall_address = address_01;	
		} else if (address_02 == address_03) {
			recall_address = address_02;
		}
	
	
	if (recall_address > MEM_END) recall_address = 0;
	
	
	//uint8_t mem_location = eeprom_read_byte(MEM_LOC); //read last memory location
	counter = eeprom_read_byte(recall_address); //read state value from that location
	if (counter >= NUM_MODES) counter = 0;
	if (++recall_address > MEM_END) recall_address = 0; //increment the location, reset if > MEM_END
	eeprom_update_byte(ADDR_1,recall_address); //update new memory location
	eeprom_update_byte(ADDR_2, recall_address);
	eeprom_update_byte(ADDR_3, recall_address);
	eeprom_update_byte(recall_address,counter); //store counter in new memory location so mode is recalled even if no changes to state are made during use
	
	PORTA.OUT = (port_output[counter]);
	current_sw_state = (PORTA.IN & (1<<SW_IN));
	current_sw_state ^= previous_sw_state;
	previous_sw_state ^= current_sw_state;
	current_sw_state &= previous_sw_state;
	
	//eeprom_read_byte()
    while (1) 
    {
		current_sw_state = (PORTA.IN & (1<<SW_IN));
		current_sw_state ^= previous_sw_state;
		previous_sw_state ^= current_sw_state;
		current_sw_state &= previous_sw_state;
		
		if (current_sw_state) {
			
			if (++counter >= NUM_MODES) {
				counter = 0;
				
			}
			//write new state to eeprom:
			eeprom_update_byte(recall_address,counter);
			
			PORTA.OUT = (port_output[counter]);
			
		}
		_delay_ms(10);
		
		//for (int i = 0; i < NUM_MODES; i++) {
			//
			//PORTA.OUT = port_output[i];
			//_delay_ms(2000);
		//}
		
    }
}

