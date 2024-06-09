#pragma once

#include "patch_table.hpp"

#include <stdint.h>

struct exe_patcher {
   ~exe_patcher();

   [[nodiscard]] bool load(const char* file_path);

   [[nodiscard]] bool save(const char* file_path);

   [[nodiscard]] bool compatible(uint32_t id_address, uint64_t expected_id);

   [[nodiscard]] bool apply(const patch& patch);

private:
   uint8_t* _data = nullptr;
   size_t _size = 0;
};
