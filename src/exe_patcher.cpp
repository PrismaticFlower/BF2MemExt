#include "exe_patcher.hpp"
#include "file_helpers.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
   if (temp_file_name) free(temp_file_name);
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
   if (offset + patch.expected_bytes.size() >= _size) return false;
   if (offset + patch.replacement_bytes.size() >= _size) return false;

   // Overflow Checks
   if (offset + patch.expected_bytes.size() < offset) return false;
   if (offset + patch.replacement_bytes.size() < offset) return false;

   bool already_patched = true;

   for (size_t i = 0; i < patch.replacement_bytes.size(); ++i) {
      if (_data[offset + i] != patch.replacement_bytes[i]) {
         already_patched = false;

         break;
      }
   }

   if (already_patched) return true;

   for (size_t i = 0; i < patch.expected_bytes.size(); ++i) {
      if (_data[offset + i] != patch.expected_bytes[i]) return false;
   }

   for (size_t i = 0; i < patch.replacement_bytes.size(); ++i) {
      _data[offset + i] = patch.replacement_bytes[i];
   }

   return true;
}