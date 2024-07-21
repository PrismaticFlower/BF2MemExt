#include "exe_patcher.hpp"
#include "file_helpers.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool memeq(const void* left, size_t left_size, const void* right, size_t right_size)
{
   if (left_size != right_size) return false;

   return memcmp(left, right, left_size) == 0;
}

exe_patcher::~exe_patcher()
{
   if (_data) delete[] _data;
}

bool exe_patcher::load(const char* file_path)
{
   if (_data) {
      delete[] _data;

      _data = nullptr;
   }

   FILE* file = fopen(file_path, "rb");

   if (not file) return false;

   bool result = false;
   ptrdiff_t file_size = 0;

   if (fseek(file, 0, SEEK_END) != 0) goto cleanup;

   file_size = ftell(file);

   if (file_size < 0) goto cleanup;

   _size = (size_t)file_size;
   _data = new uint8_t[file_size];

   rewind(file);

   if (fread(_data, sizeof(uint8_t), _size, file) != _size) goto cleanup;

   result = true;

cleanup:
   fclose(file);

   return result;
}

bool exe_patcher::save(const char* file_path)
{
   if (not _data) return false;

   char* temp_file_name = aquire_temp_file(file_path, "BF2Patch");

   if (not temp_file_name) return false;

   FILE* file = fopen(temp_file_name, "wb");
   bool result = false;

   if (not file) goto cleanup;

   if (fwrite(_data, sizeof(uint8_t), _size, file) != _size) goto cleanup;

   fclose(file);

   file = nullptr;

   if (not move_file(temp_file_name, file_path)) goto cleanup;

   result = true;

cleanup:
   if (temp_file_name) {
      remove(temp_file_name);
      free(temp_file_name);
   }
   if (file) fclose(file);

   return result;
}

bool exe_patcher::compatible(uint32_t id_address, uint64_t expected_id)
{
   // Bounds and overflow Checks
   if (id_address + sizeof(uint64_t) >= _size) return false;
   if (id_address + sizeof(uint64_t) < id_address) return false;

   uint64_t exe_id = 0;

   memcpy(&exe_id, _data + id_address, sizeof(uint64_t));

   return exe_id == expected_id;
}

bool exe_patcher::apply(const patch& patch)
{
   if (not _data) return false;

   const uint32_t offset = patch.address;

   // Bounds Checks
   if (offset + sizeof(uint32_t) >= _size) return false;
   if (offset + sizeof(uint32_t) >= _size) return false;

   // Overflow Checks
   if (offset + sizeof(uint32_t) < offset) return false;
   if (offset + sizeof(uint32_t) < offset) return false;

   const bool already_patched = memeq(&_data[offset], sizeof(uint32_t), &patch.replacement_value,
                                      sizeof(patch.replacement_value));

   if (already_patched) return true;

   const bool expected_value =
      memeq(&_data[offset], sizeof(uint32_t), &patch.expected_value, sizeof(patch.expected_value));

   if (not expected_value) return false;

   memcpy(&_data[offset], &patch.replacement_value, sizeof(uint32_t));

   return true;
}