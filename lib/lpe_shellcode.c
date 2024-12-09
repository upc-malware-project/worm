#include <stdio.h>

void __attribute__((constructor)) setup() { printf("Hello"); }