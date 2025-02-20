#pragma once

#include <stdint.h>

struct exe_patcher {
   ~exe_patcher();

   [[nodiscard]] bool load(const char* file_path);

   [[nodiscard]] bool add_dll(const char* dll_name);

   [[nodiscard]] bool reset_dlls();

   [[nodiscard]] bool save(const char* file_path);

   [[nodiscard]] bool compatible(uint32_t id_address, uint64_t expected_id);

private:
   void* _detour_binary = nullptr;

   uint8_t* _data = nullptr;
   size_t _size = 0;
};
