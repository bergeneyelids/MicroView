#include "MicroView.h"
#include "myMicroView.h"

#include <OneWire.h>
#include <math.h>

//DS18S20 Signal pin on digital 3 (uView 12)
#define DS18S20_Pin 3

OneWire ds(DS18S20_Pin);

// Temp variables

// number of samples to use for running average and smoothing
#define avgreadings 12
#define smoothreadings 5

float tmin = 85.; // min temp
float tmax = -10.; // max temp
float tavg = 0.; // avg temp
float tavgprev = 0.; // previous avg temp
float avgsamples[avgreadings];  // samples for average
float smoothsamples[smoothreadings]; // samples for smoothing
float totavg = 0.; // running total for average
float totsmooth = 0.; // running total for smoothing
int si = 0; // smoothing index
int ai = 0; // average index

int displayType = 0; // type of temperature to display


void setup() {

#ifdef SERIALDEBUG
	Serial.begin(9600);
#endif

	uView.begin();

	for (int i = 0; i < avgreadings; i++) {
		avgsamples[i] = 0.;
	}

	for (int i = 0; i < smoothreadings; i++) {
		smoothsamples[i] = 0.;
}

	// throw away the first readings to initialize all smoothing values
	for (int i = 0; i < smoothreadings; i++) {
		float tbad = smoothTemp();
		displayTemp('.');
	}

	// initialize the average temperature and the sample set
	for (int i = 0; i < avgreadings; i++) {
		avgsamples[i] = smoothTemp();
		totavg += avgsamples[i];
		displayTemp('*');
	}

	// set average, min and max temp
	tavg = totavg / avgreadings;
	tavgprev = tavg;
	if (tavg > tmax) {
		tmax = tavg;
	}
	if (tavg < tmin) {
		tmin = tavg;
	}

}

void loop() {
	totavg -= avgsamples[ai];
	avgsamples[ai] = smoothTemp();
	totavg += avgsamples[ai];
	if (++ai >= avgreadings) {
		ai = 0;
	}
	// set average, min and max temp
	tavgprev = tavg;
	tavg = totavg / avgreadings;
	if (tavg > tmax) {
		tmax = tavg;
	}
	if (tavg < tmin) {
		tmin = tavg;
	}
	switch (displayType){
	case 0:
		displayTemp(tavg, displayType);
		break;
	case 1:
		if (tavg > tavgprev) {
			displayTemp(UP_ARROW);
		}
		if (tavg < tavgprev) {
			displayTemp(DOWN_ARROW);
		}
		break;
	case 2:
		displayTemp(tmin, displayType);
		break;
	case 3:
		displayTemp(tmax, displayType);
	}

	if (++displayType > 3) {
		displayType = 0;
	}

#ifdef SERIALDEBUG
	Serial.println(tavg);
#endif

}

// Compute Temperature Smoothed Value
float smoothTemp() {
	totsmooth -= smoothsamples[si];
	smoothsamples[si] = round(getTemp() * 100) / 100; // 1 decimal precision
	totsmooth += smoothsamples[si];
	if (++si >= smoothreadings) {
		si = 0;
	}
	return (totsmooth / smoothreadings);
}

// Display Temperature
void displayTemp(float temp, int type) {
	uView.clear(PAGE);
	uView.setCursor(0, 0);
	uView.setFontType(FONT_font8x16);

	switch (type) {
	// current temp
	case 0:
		uView.println("T. (C):");
		break;
	// tmin
	case 2:
		uView.println("T.MIN.:");
		break;
	// tmax
	case 3:
		uView.println("T.MAX.:");
	}

	uView.println(temp);
	uView.display();
	delay(1000);
}

void displayTemp(char temp) {
	uView.clear(PAGE);
	uView.setCursor(0, 0);
	uView.setFontType(FONT_font8x16);
	uView.println("T. (C):");
	uView.println(temp);
	uView.display();
	delay(1000);
}

void displayTemp(char *temp) {
	uView.clear(PAGE);
	uView.setCursor(27, 10);
	uView.setFontType(FONT_Arrows);
	uView.println(*temp);
	uView.display();
	delay(1000);
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

