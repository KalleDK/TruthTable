/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* <github.com@k-moeller.dk> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return Kalle R. Møller
* ----------------------------------------------------------------------------
*/
#include "main.h"

bool run_again() {
	int key;
	printf("------------\n 1 Run again\n-1 Exit\n");
	scanf("%d", &key);
	return (key != -1);
}

int number_of_inputs() {
	int last_input = -1;
	while (!((0 <= last_input) && (last_input <= MAX_INPUT))) {
		printf("Output: 15\nInput: 0 - ");
		scanf("%d", &last_input);
	}
	return last_input;
}

double get_divider_lsb(double analog_hz) {
	double hz_divider = -1;
	while (!((0 <= hz_divider) && (hz_divider <= 1e8))) {
		printf("LSB clock (Hz): ");
		scanf("%lf", &hz_divider);
		hz_divider = hz_to_divider(analog_hz, hz_divider);
		if (DEBUG) {
			printf("Setting LSB clock to: ");
			print_hz(analog_hz, hz_divider);
			printf("\n");
		}
	}
	return hz_divider;
}

bool is_equal_recursive(WORD *analog_buffer, WORD start, WORD depth, WORD bitmask) {
	if (depth > 0) {
		//printf("%u == %u\n", analog_buffer[start], analog_buffer[start + depth]);
		return ((analog_buffer[start] & bitmask) == (analog_buffer[start + depth] & bitmask)) & is_equal_recursive(analog_buffer, start, depth - 1, bitmask);
	}
	else {
		return 1;
	}
}

void fill_table(HDWF *hdwf, WORD *truth_table, WORD combinations, int analog_buffer_size, double divider_lsb, WORD bitmask) {
	
	bool tableFull = false;
	WORD *analog_buffer = new WORD[analog_buffer_size];
	WORD missing_samples;

	while (!tableFull) {
		
		//Samples frequency 10* LSB frequency
		get_buffer(hdwf, analog_buffer, analog_buffer_size, divider_lsb/10);

		// Adding buffer data to the truth table
		// -3 to not exceed array in check
		for (int i = 0; i <= (analog_buffer_size - 3); i++) {
			if (is_equal_recursive(analog_buffer, i, 3, bitmask + (1 << 15))) {
				truth_table[(analog_buffer[i] & bitmask)] = (analog_buffer[i] & (1 << 15));
				//printf("%d", analog_buffer[i] & (1 << 15));
			}
		}


		// Looping through the truth table to see if there are any '2' left ie not full
		tableFull = true;
		missing_samples = 0;
		for (int i = 0; i < combinations; i++) {
			if (truth_table[i] == 2) {
				tableFull = false;
				missing_samples++;
			}
		}
		if (missing_samples) {
			printf("Missing %d samples\n", missing_samples);
		}

	}
}

void print_table(WORD *truth_table, WORD combinations, int last_input) {
	

	//+---+---+---+
	printf("\n+");
	for (int i = 0; i <= (last_input+1); i++) {
		printf("---+");
	}
	printf("\n");

	//| 1 | 0 | X |
	printf("|");
	for (int i = 0; i < (last_input+1); i++) {
		printf(" %c |", (last_input - i + 65));
	}
	printf(" X |\n");

	//+---+---+---+
	printf("|");
	for (int i = 0; i <= (last_input+1); i++) {
		printf("---+");
	}
	printf("\n");

	
	//| 0 | 1 | 0 |
	for (int j = 0; j < combinations; j++) {
		printf("|");
		//Printing the different combinations
		for (int i = last_input; i >= 0; i--) {
				printf(" %d |", (j & (1 << i)) == (1 << i));
		}
		//Printing the different result
		printf(" %d |\n", truth_table[j] == (1 << 15));
	}

	//+---+---+---+
	printf("+");
	for (int i = 0; i <= (last_input + 1); i++) {
		printf("---+");
	}
	printf("\n\n");

}

int main(int carg, char **szarg){
	HDWF hdwf;
	int last_input;


	WORD bitmask;
	WORD combinations;
	WORD *truth_table;

	double divider_lsb;
	double analog_hz;
	int analog_buffer_size;
	
	
	//Connecting to Analog Discovery
	if (!analog_connect(&hdwf)) {
		return -1;
	}

	do {

		//Getting its internal frequency (100 MHz)
		analog_hz = get_analog_hz(&hdwf);
		divider_lsb = hz_to_divider(analog_hz, 1000);
		if (DEBUG) {
			printf("System internal clock: ");
			print_hz(analog_hz, 1);
			printf("\n");
			printf("Default LSB clock: ");
			print_hz(analog_hz, divider_lsb);
			printf("\n");
		}

		//Getting Buffer Size
		analog_buffer_size = get_analog_buffer_size(&hdwf);
		if (DEBUG) { printf("Buffer Size: %d\n", analog_buffer_size); }

		//Getting the last input number thats in use
		last_input = number_of_inputs();

		//Getting divider_lsb
		divider_lsb = get_divider_lsb(analog_hz);
		
		//Preparing the bitmask (to remove unwanted inputs)
		bitmask = (1 << (last_input+1))-1;
		if (DEBUG) { printf("Bitmask: %d\n", bitmask); }

		//Calculating number of combinations in the TruthTable
		combinations = (1 << (last_input+1));
		if (DEBUG) { printf("Combinations: %d\n", combinations); }

		//Initializing the TruthTable
		truth_table = new WORD[combinations];
		for (WORD i = 0; i < combinations; i++) {
			truth_table[i] = 2;
		}

		//Starting the binary clock
		generate_binary_clock(&hdwf, last_input, divider_lsb);

		//Gathering input data from Analog Device
		fill_table(&hdwf, truth_table, combinations, analog_buffer_size, divider_lsb, bitmask);

		//Printing the table
		print_table(truth_table, combinations, last_input);
		

		


	}while (run_again());

	//Disconnecting Analog Discovery
	analog_disconnect(&hdwf);
	if (DEBUG) { printf("Disconnecting Analog Discovery\n"); }

}