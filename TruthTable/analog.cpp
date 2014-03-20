/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* <github.com@k-moeller.dk> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return Kalle R. Møller
* ----------------------------------------------------------------------------
*/
#include "analog.h"

void print_hz(double analog_hz, double divider) {
	if ((analog_hz / divider) > 1e6) {
		printf("%dMHz", (int)(analog_hz / divider / 1e6));
	}
	else if ((analog_hz / divider) > 1e3){
		printf("%dkHz", (int)(analog_hz / divider / 1e3));
	}
	else {
		printf("%dHz", (int)(analog_hz / divider));
	}
}

double hz_to_divider(double analog_hz, double hz) {
	return analog_hz / hz;
}

double divider_to_hz(double analog_hz, double divider){
	return analog_hz / divider;
}

bool analog_connect(HDWF *hdwf) {
	if (!FDwfDeviceOpen(-1, hdwf)){
		if (!OFFLINE_MODE) {
			printf("Error connecting to Analog Discovery\n");
			return 0;
		}
		else {
			printf("Analog discovery missing: OFFLINE MODE\n");
			return 1;
		}
	}
	if (DEBUG) {
		FDwfDigitalOutReset(hdwf[0]);
		FDwfDigitalInReset(hdwf[0]);
		printf("Connecting to Analog Discovery\n");
	}
	return 1;
}

bool analog_disconnect(HDWF *hdwf) {
	return (bool)FDwfDeviceClose(hdwf[0]);
}

double get_analog_hz(HDWF *hdwf) {
	double hzSys;
	FDwfDigitalOutInternalClockInfo(hdwf[0], &hzSys);
	return hzSys;
}

int get_analog_buffer_size(HDWF *hdwf) {
	int cSamples;
	FDwfDigitalInBufferSizeInfo(hdwf[0], &cSamples);
	return cSamples;
}

void generate_binary_clock(HDWF *hdwf, int last_input, int hz_divider) {
	
	if (DEBUG) {
		double pvMin;
		double pvMax;
		FDwfAnalogOutNodeAmplitudeInfo(hdwf[0], 0, AnalogOutNodeCarrier, &pvMin, &pvMax);
		printf("Analog Voltage Min: %.1lfV Max: %.1lfV\n", pvMin, pvMax);
	}
	
	WORD hz_multiplier;
	double analog_hz = get_analog_hz(hdwf);

	//Turning on 5.0V on W1
	FDwfAnalogOutNodeEnableSet(hdwf[0], 0, AnalogOutNodeCarrier, true);
	FDwfAnalogOutNodeFunctionSet(hdwf[0], 0, AnalogOutNodeCarrier, funcDC);
	FDwfAnalogOutNodeOffsetSet(hdwf[0], 0, AnalogOutNodeCarrier, 5.0);
	FDwfAnalogOutConfigure(hdwf[0], 0, true);
	Wait(1); //Wait 1 so the system have time to stabilize

	//Configuring Binary Clock
	for (int i = 0; i < (last_input+1); i++) {
		hz_multiplier = (1 << i);
		FDwfDigitalOutEnableSet(hdwf[0], i, 1);
		FDwfDigitalOutDividerSet(hdwf[0], i, hz_divider * hz_multiplier);
		FDwfDigitalOutCounterSet(hdwf[0], i, 1, 1);
		if (DEBUG) {
			printf("Starting input: %d at ", i);
			print_hz(analog_hz, hz_divider * hz_multiplier);
			printf("\n");
		}
	}
	
	//Starting Binary Clock
	FDwfDigitalOutConfigure(hdwf[0], 1);
}

void get_buffer(HDWF *hdwf, WORD *analog_buffer, int analog_buffer_size, int hz_divider) {
	
	STS sts;
	
	FDwfDigitalInReset(hdwf[0]);
	FDwfDigitalInDividerSet(hdwf[0], hz_divider);
	FDwfDigitalInSampleFormatSet(hdwf[0], 16);
	
	// Should be more dynamic
	FDwfDigitalInTriggerPositionSet(hdwf[0], 4096);
	FDwfDigitalInTriggerSourceSet(hdwf[0], trigsrcDetectorDigitalIn);
	FDwfDigitalInTriggerAutoTimeoutSet(hdwf[0], 10.0);
	FDwfDigitalInTriggerSet(hdwf[0], 0, 0, 0xFFFF, 0xFFFF);
	
	// Start
	FDwfDigitalInConfigure(hdwf[0], false, true);
	
	printf("Waiting for Analog Discovey\n");
	do{
		if (!FDwfDigitalInStatus(hdwf[0], true, &sts)) return;
	} while (sts != stsDone);
	
	//Fetching Buffer
	FDwfDigitalInStatusData(hdwf[0], analog_buffer, analog_buffer_size*sizeof(WORD));
	
}