#pragma once
#include "globals.h"

int serve(Globals *global);


// IPP ENUMS

/* Tags */
#define IPP_TAG_OPERATION 1
#define IPP_TAG_PRINTER 4

#define IPP_TAG_INTEGER 33
#define IPP_TAG_BOOLEAN 34
#define IPP_TAG_ENUM    35
#define IPP_TAG_TEXT    65
#define IPP_TAG_NAME    66
#define IPP_TAG_KEYWORD 68
#define IPP_TAG_URI     69
#define IPP_TAG_CHARSET 71
#define IPP_TAG_LANGUAGE 72
#define IPP_TAG_MIMETYPE 73

/* Printer State */
#define IPP_PSTATE_IDLE 3

/* Operations */
#define IPP_OP_PRINT_JOB 2
#define IPP_OP_VALIDATE_JOB 4
#define IPP_OP_CANCEL_JOB 8
#define IPP_OP_GET_JOB_ATTRIBUTES 9
#define IPP_OP_GET_PRINTER_ATTRIBUTES 11