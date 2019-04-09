#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define DEBUG
#include "debug.h"

#define TYPE_INVAL 0
#define TYPE_FUNCT 1
#define TYPE_VALUE 2

void init();
int precheck();
bool compile();
bool calculate();
