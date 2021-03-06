#ifndef ROUTEASM_H
#define ROUTEASM_H

#include "util.h"

#define POINT 0x01
#define PRINT 0x02
#define WHILE 0x03
#define WHILE_VAR 0x04
#define ENDWHILE 0x05
#define FOR 0x06
#define ENDFOR 0x07
#define INTEGER 0x08
#define INCREMENT 0x09
#define DECREMENT 0x0A
#define ADD 0x0B
#define ADD_ASSIGN 0x0C
#define ASSIGN 0x0D
#define SUB 0x0E
#define SUB_ASSIGN 0x0F
#define MUL 0x10
#define MUL_ASSIGN 0x11
#define DIV 0x12
#define DIV_ASSIGN 0x13
#define FOR_VAR 0x14
#define IF_Z 0x15
#define IF_NZ 0x16
#define IF_POS 0x17
#define IF_NEG 0x18
#define ENDIF 0x19
#define BREAK_WHILE 0x20
#define END 0x21
#define POINT_LLA 0x22
#define LAUNCH 0x23
#define LAND 0x24
#define RTL 0x25

#ifdef AUTOPILOT_INTERFACE
bool routeasm(std::string inputfile, std::string filestring, uint8_t*& writeback, int& size);
void routeasm_get_log(std::string& routeLog);
#endif

#endif