/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* <github.com@k-moeller.dk> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return Kalle R. Møller
* ----------------------------------------------------------------------------
*/
#include "analog.h"

bool run_again();
int number_of_inputs();
double get_divider_lsb(double analog_hz);
bool is_equal_recursive(WORD *analog_buffer, WORD start, WORD depth);
void fill_table(HDWF *hdwf, WORD *truth_table, WORD combinations, int analog_buffer_size, double divider_lsb, WORD bitmask);