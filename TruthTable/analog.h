/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* <github.com@k-moeller.dk> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return Kalle R. Møller
* ----------------------------------------------------------------------------
*/
#include <stdio.h>
#include "dwf.h"

#ifdef WIN32
#include <windows.h>
#define Wait(ts) Sleep(1000*ts)
#else
#include <unistd.h>
#include <sys/time.h>
#define Wait(ts) usleep(1000000*ts)
#endif

using namespace std;

#define OFFLINE_MODE true
#define DEBUG true
#define MAX_INPUT 14

void print_hz(double analog_hz, double divider);
double hz_to_divider(double analog_hz, double hz);
double divider_to_hz(double analog_hz, double divider);
bool analog_connect(HDWF *hdwf);
bool analog_disconnect(HDWF *hdwf);
double get_analog_hz(HDWF *hdwf);
int get_analog_buffer_size(HDWF *hdwf);
void generate_binary_clock(HDWF *hdwf, int last_input, int hz_divider);
void get_buffer(HDWF *hdwf, WORD *analog_buffer, int analog_buffer_size, int hz_divider);