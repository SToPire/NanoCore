#pragma once

#include "common/x86.h"

int init_serial();
char read_serial();
void write_serial(char c);