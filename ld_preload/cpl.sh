#!/bin/bash
gcc -fPIC -shared -o microkit.so microkit.c -ldl
