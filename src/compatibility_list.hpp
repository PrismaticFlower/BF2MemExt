#pragma once

#include <stdint.h>

#define EXE_COUNT 3

struct compatibile_exe {
   const char* name = "";
   uint32_t id_address = 0;
   uint64_t expected_id = 0;
};

extern const compatibile_exe compatibility_list[EXE_COUNT];
