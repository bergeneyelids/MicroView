/*
*/
#include <space03.h>
#include <space02.h>
#include <space01.h>
#include <fontlargenumber.h>
#include <font8x16.h>
#include <font5x7.h>
#include <7segment.h>
#include <MicroView.h>

#include <OneWire.h>

int DS18S20_Pin = 3; //DS18S20 Signal pin on digital 3 (uView 12)

char charbuf[10]; // buffer for string conversions


MicroViewWidget *vWidget1;
OneWire ds(DS18S20_Pin);

void setup() {
	uView.begin();
	uView.clear(PAGE);
	Serial.begin(9600);

	vWidget1 = new MicroViewSlider(20, 0, 10, 40, WIDGETSTYLE3);
}

void loop() {
	float temperature = getTemp();
	Serial.println(temperature);
	//String tString =  String(dtostrf(temperature, 1, 2, charbuf)) + "°C\n";
	//uView.print(tString);	// display temperature
	uView.display();
	vWidget1->setValue(temperature);
	delay(100); //just here to slow down the output so it is easier to read

	
	//for (int i = 0; i <= 255; i++) {
	//	vWidget1->setValue(i);
	//	uView.display();
	//}

}
//returns the temperature from one DS18S20 in DEG Celsius
float getTemp() {

	byte data[12];
	byte addr[8];

	if (!ds.search(addr)) {
		//no more sensors on chain, reset search
		ds.reset_search();
		return -1000;
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
		Serial.println("CRC is not valid!");
		return -1000;
	}

	if (addr[0] != 0x10 && addr[0] != 0x28) {
		Serial.print("Device is not recognized");
		return -1000;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1); // start conversion, with parasite power on at the end

	byte present = ds.reset();
	ds.select(addr);
	ds.write(0xBE); // Read Scratchpad


	for (int i = 0; i < 9; i++) { // we need 9 bytes
		data[i] = ds.read();
	}

	ds.reset_search();

	byte MSB = data[1];
	byte LSB = data[0];

	float tempRead = ((MSB << 8) | LSB); //using two's compliment
	float TemperatureSum = tempRead / 16;

	return TemperatureSum;

}