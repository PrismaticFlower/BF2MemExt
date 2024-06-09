// BF2MemExt.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "exe_patcher.hpp"
#include "patch_table.hpp"

#include <stdio.h>

int main(int arg_count, const char** args)
{
   if (arg_count != 2) {
      printf("Usage: <file>");

      return 1;
   }

   const char* file_path = args[1];

   exe_patcher editor;

   if (not editor.load(file_path)) {
      printf("Failed to open %s for patching.\n", file_path);

      return 1;
   }

   bool found_compatible_list = false;

   for (const exe_patch_list& exe_list : patch_lists) {
      if (not editor.compatible(exe_list.id_address, exe_list.expected_id)) {

         continue;
      }

      printf("Identified executable as: %s. Applying patches.\n", exe_list.name);

      found_compatible_list = true;

      for (const patch_set& set : exe_list.patches) {
         printf("Applying patch set: %s\n", set.name);

         for (const patch& patch : set.patches) {
            if (not editor.apply(patch)) {
               printf("Failed to apply patch. %s is unmodified.\n", file_path);

               return 1;
            }
         }
      }

      break;
   }

   if (not found_compatible_list) {
      printf("Couldn't identify executable. Unable to patch.\n");

      return 1;
   }

   if (not editor.save(file_path)) {
      printf("Failed to save %s after patching.\n", file_path);

      return 1;
   }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
