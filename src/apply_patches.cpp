#include "compatibility_list.hpp"
#include "exe_patcher.hpp"
#include "file_helpers.hpp"

#include <stdio.h>
#include <string.h>

const char* PATCH_DLL_NAME = "BF2GameExt.dll";

bool apply(const char* file_path, int (*print)(const char* format, ...)) noexcept
{
   if (not print) print = printf;

   if (not file_exists(PATCH_DLL_NAME)) {
      print("%s is missing. Patching depends on this file and can't not work without.\r\n", PATCH_DLL_NAME);

      return false;
   }

   exe_patcher editor;

   if (not editor.load(file_path)) {
      print("Failed to open %s for patching.\r\n", file_path);

      return false;
   }

   bool is_compatible = false;

   for (const compatibile_exe& exe : compatibility_list) {
      if (not editor.compatible(exe.id_address, exe.expected_id)) {

         continue;
      }

      print("Identified executable as: %s. Applying patches.\r\n", exe.name);

      is_compatible = true;

      break;
   }

   if (not is_compatible) {
      print("Couldn't identify executable. Unable to patch.\r\n");

      return false;
   }

   if (not editor.add_dll(PATCH_DLL_NAME)) {
      print("Failed to add DLL import to executable.\r\n");

      return false;
   }

   print("Copying %s to game directory.\r\n", PATCH_DLL_NAME);

   if (not copy_next_to(PATCH_DLL_NAME, file_path)) {
      print("Failed to copy patch DLL (%s) to game directory.\r\n", PATCH_DLL_NAME);

      return false;
   }

   print("Saving patched game executable.\r\n");

   if (not editor.save(file_path)) {
      print("Failed to save %s after patching.\r\n", file_path);

      return false;
   }

   print("Patching succeeded.\r\n");

   return true;
}