#pragma once

#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

int charToInt(char* msg, int size);