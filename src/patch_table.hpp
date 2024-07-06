#pragma once

#include <stdint.h>

#include "slim_vector.hpp"

#define PATCH_COUNT 2
#define EXE_COUNT 3

struct patch {
   uint32_t address = 0;
   slim_vector<uint8_t> expected_bytes;
   slim_vector<uint8_t> replacement_bytes;
};

struct patch_set {
   const char* name = "";
   slim_vector<patch> patches;
};

struct exe_patch_list {
   const char* name = "";
   uint32_t id_address = 0;
   uint64_t expected_id = 0;

   const patch_set patches[PATCH_COUNT];
};

extern const exe_patch_list patch_lists[EXE_COUNT];
