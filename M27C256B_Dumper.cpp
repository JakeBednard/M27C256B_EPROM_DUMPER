// Jacob Bednard
// Wayne State Computer and Systems Security Lab
// 2017-09-09
//
// M27C256B EPROM Extractor for Raspberry Pi. This code will raid all the memory
// address on the EPROM chip, then dump them to binary file called "EPROM_DUMP.bin"
// located within the execution directory.
// 
// DEPENDENCY: wiringPi v2.44
//
// The user parameter section below will let you set your GPIO pins accordingly for your
// setup. Depending on endianess of address space and data-out, you may have to 
// adjust accordingly. Also note that this is designed with the Broadcom chipset 
// in mind. You may have to edit the "wiringPiSetupGpio()" method in order to call
// the right init. Consult the wiringPi documentation for this... simple enough.
//
// This should do the trick for compiling with GCC on the Raspberry Pi
// Compile: gcc M27C256B_Dumper.cpp -lwiringPi -std=c++11
// Run with: ./M27C256B_Dumper
//
// Use at your own risk. This is a quick and dirty script for dumping chips; not production ready
// code... meant for Public Use. 
//
// You can probabally quickly modify this code to work
// with any parallel EPROM chip. Consult the chips documentation for further guidenance.
//
// NOTE: This program is not written for speed. Yes, I can dump a 32KB EPROM in 400ms, but thats
// actually pretty slow. For most reverse engineering, this shouldn't matter.

/* ------------------------------------ USER PARAMETERS ------------------------------------ */

// Raspberry Pi GPIO Control Pins																		
const int CHIP_ENABLE = 24;                                                             // E
const int OUTPUT_ENABLE = 23;                                                           // G
const int ADDRESS_PINS[] = { 5, 11, 9, 10, 22, 27, 17, 4, 15, 18, 8, 25, 3, 14, 2 };    // A0 -> A14
const int READ_PINS[] = { 13, 19, 26, 21, 20, 16, 12, 6 };                              // Q0 -> Q7
const int DELAY = 1;                                                                    // Microseconds - Delay Length (If you're getting read errors, increase this).

const int NUMBER_OF_ADDRESSES = 32768;                                                  // Total number of addresses (Fit accordingly for your needs.)

/* --------------------------------------- EXECUTION --------------------------------------- */

#include <wiringPi.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

// Chip Info ( M27C256B EPROM ) 
const int ADDRESS_WIDTH = 15;                                                           // Bits
const int DATA_WIDTH = 8;                                                               // Bits (DONT CHANGE, IT'LL BREAK)

vector<char> byteArray(NUMBER_OF_ADDRESSES);                                            // Array to hold bytes prior to file write

// Method Prototypes
void setup();
void setAddress(int address);
void read(int address);
void writeFile();
void teardown();													

int main() {

	// Init GPIO, then for each memory address, read the chip, 
	// then write write the dump to file. Finally, close-up shop.

	setup();

	for (int address = 0; address < NUMBER_OF_ADDRESSES; ++address) {
		setAddress(address);
		read(address);
	}

	writeFile();
	teardown();

}

void setAddress(int address) {
	
	// Set the GPIO pins to access the correct address of the EPROM.

	for (int offset = 0; offset < ADDRESS_WIDTH; offset++) {
		digitalWrite(ADDRESS_PINS[offset], (address & (1 << offset)) >> offset);
	}

	delayMicroseconds(DELAY);

}

void read(int address) {

	// Read the current data-output of the EPROM and place it in the memory dump array.
	
	unsigned char byte = 0;

	for (int i = 0; i < DATA_WIDTH; ++i) {
		if (digitalRead(READ_PINS[i])) {
			byte |= 1 << i;
		}
	}
	
	byteArray[address] = byte;

	delayMicroseconds(DELAY);
	
	return;

}

void writeFile() {

	// Make the dump binary file with the read data.

	ofstream outfile("EPROM_DUMP.bin", ios::out | ios::trunc | ios::binary);

	for (int address = 0; address < NUMBER_OF_ADDRESSES; ++address) {
		outfile.write(&byteArray[address], sizeof(char));
	}

	outfile.close();

}

void setup() {

	// Init GPIO Pin mode, Flicker chip/write enable pins to get the EEPROM into read mode.

	wiringPiSetupGpio();

	for (int pin : ADDRESS_PINS) {
		pinMode(pin, OUTPUT);
	}

	for (int pin : READ_PINS) {
		pinMode(pin, INPUT);
	}

	pinMode(CHIP_ENABLE, OUTPUT);
	digitalWrite(CHIP_ENABLE, HIGH);
	delayMicroseconds(DELAY);

	pinMode(OUTPUT_ENABLE, OUTPUT);
	digitalWrite(OUTPUT_ENABLE, HIGH);
	delayMicroseconds(DELAY);

	digitalWrite(CHIP_ENABLE, LOW);
	delayMicroseconds(DELAY);
	digitalWrite(OUTPUT_ENABLE, LOW);
	delayMicroseconds(DELAY);

}

void teardown() {

	// Gracefully place the chip out of read mode.

	digitalWrite(OUTPUT_ENABLE, HIGH);
	delayMicroseconds(DELAY);

	digitalWrite(CHIP_ENABLE, HIGH);
	delayMicroseconds(DELAY);

}
