#pragma warning(disable : 4530)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fcntl.h>
#include <io.h>

[[nodiscard]] char* aquire_temp_file(const char* base_file_path, const char* prefix)
{
   const char* backslash = strrchr(base_file_path, '\\');
   const char* forwardslash = strrchr(base_file_path, '/');

   if (not backslash and not forwardslash) {
      char temp_file_name[MAX_PATH] = {};

      if (GetTempFileNameA(".", prefix, 0, &temp_file_name[0]) == 0) {
         return nullptr;
      }

      return _strdup(&temp_file_name[0]);
   }

   const ptrdiff_t backslash_distance = backslash ? backslash - base_file_path : PTRDIFF_MIN;
   const ptrdiff_t forwardslash_distance = forwardslash ? forwardslash - base_file_path : PTRDIFF_MIN;

   const char* slash = backslash_distance > forwardslash_distance ? backslash : forwardslash;

   const size_t directory_size = slash - base_file_path;
   char* directory = (char*)malloc(directory_size + 1);

   if (not directory) return nullptr;

   memset(directory, '\0', directory_size + 1);
   memcpy(directory, base_file_path, directory_size);

   char temp_file_name[MAX_PATH] = {};

   const UINT result = GetTempFileNameA(directory, prefix, 0, &temp_file_name[0]);

   free(directory);

   if (result == 0) return nullptr;

   return _strdup(temp_file_name);
}

[[nodiscard]] bool move_file(const char* from, const char* to)
{
   return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0;
}

void init_cstdio()
{
   if (not AttachConsole(ATTACH_PARENT_PROCESS)) return;

   FILE* file = nullptr;
   freopen_s(&file, "CONIN$", "r", stdin);
   freopen_s(&file, "CONOUT$", "w", stdout);
   freopen_s(&file, "CONOUT$", "w", stderr);
}