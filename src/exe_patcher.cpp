#include "exe_patcher.hpp"
#include "file_helpers.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <detours.h>

exe_patcher::~exe_patcher()
{
   if (_data) delete[] _data;
   if (_detour_binary) DetourBinaryClose(_detour_binary);
}

bool exe_patcher::load(const char* file_path)
{
   HANDLE file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);

   if (file == INVALID_HANDLE_VALUE) return false;

   bool result = false;
   LARGE_INTEGER file_size = {};
   DWORD read_bytes = 0;

   _detour_binary = DetourBinaryOpen(file);

   if (_detour_binary == nullptr) goto cleanup;

   if (SetFilePointer(file, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
      goto cleanup;
   }

   if (not GetFileSizeEx(file, &file_size)) {
      goto cleanup;
   }

   if (file_size.QuadPart > UINT32_MAX) goto cleanup;

   _size = (size_t)file_size.QuadPart;
   _data = new uint8_t[_size];

   if (not ReadFile(file, _data, (DWORD)file_size.QuadPart, &read_bytes, nullptr)) {
      goto cleanup;
   }

   result = true;

cleanup:
   if (file != INVALID_HANDLE_VALUE) CloseHandle(file);

   return result;
}

bool exe_patcher::add_dll(const char* dll_name)
{
   DetourBinaryResetImports(_detour_binary);

   struct context {
      const char* dll_name = nullptr;
      bool added_dll = false;
   };

   context edit_context{.dll_name = dll_name};

   if (not DetourBinaryEditImports(
          _detour_binary, &edit_context,
          [](PVOID pContext, LPCSTR pszFile, LPCSTR* ppszOutFile) -> BOOL {
             context* edit_context = (context*)pContext;

             if (not pszFile && not edit_context->added_dll) { // Add new byway.
                edit_context->added_dll = true;
                *ppszOutFile = edit_context->dll_name;
             }

             return TRUE;
          },
          nullptr, nullptr, nullptr)) {
      return false;
   }

   return true;
}

bool exe_patcher::reset_dlls()
{
   return DetourBinaryResetImports(_detour_binary) != FALSE;
}

bool exe_patcher::save(const char* file_path)
{
   if (not _detour_binary) return false;

   char* temp_file_name = aquire_temp_file(file_path, "BF2Patch");

   if (not temp_file_name) return false;

   bool result = false;

   HANDLE file = CreateFileA(temp_file_name, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

   if (file == INVALID_HANDLE_VALUE) goto cleanup;

   if (not DetourBinaryWrite(_detour_binary, file)) goto cleanup;

   CloseHandle(file);
   file = INVALID_HANDLE_VALUE;

   DetourBinaryClose(_detour_binary);
   _detour_binary = nullptr;

   if (not move_file(temp_file_name, file_path)) goto cleanup;

   result = true;

cleanup:
   if (temp_file_name) {
      remove(temp_file_name);
      free(temp_file_name);
   }
   if (file != INVALID_HANDLE_VALUE) CloseHandle(file);

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
