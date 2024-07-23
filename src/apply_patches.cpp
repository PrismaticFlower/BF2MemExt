#include "exe_patcher.hpp"
#include "patch_table.hpp"

#include <stdio.h>
#include <string.h>

bool apply(const char* file_path, int (*print)(const char* format, ...)) noexcept
{
   if (not print) print = printf;

   exe_patcher editor;

   if (not editor.load(file_path)) {
      print("Failed to open %s for patching.\r\n", file_path);

      return false;
   }

   bool found_compatible_list = false;

   for (const exe_patch_list& exe_list : patch_lists) {
      if (not editor.compatible(exe_list.id_address, exe_list.expected_id)) {

         continue;
      }

      print("Identified executable as: %s. Applying patches.\r\n", exe_list.name);

      if (not editor.prepare(0x1000)) {
         print("Failed add new executable section for patch data. %s is unmodified.\r\n", file_path);

         return false;
      }

      found_compatible_list = true;

      for (const patch_set& set : exe_list.patches) {
         print("Applying patch set: %s\r\n", set.name);

         for (const patch& patch : set.patches) {
            if (not editor.apply(patch)) {
               print("Failed to apply patch. %s is unmodified.\r\n", file_path);

               return false;
            }
         }
      }

      break;
   }

   if (not found_compatible_list) {
      print("Couldn't identify executable. Unable to patch.\r\n");

      return false;
   }

   if (not editor.save(file_path)) {
      print("Failed to save %s after patching.\r\n", file_path);

      return false;
   }

   return true;
}