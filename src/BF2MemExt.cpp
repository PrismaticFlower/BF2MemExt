// BF2MemExt.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "apply_patches.hpp"
#include "file_helpers.hpp"
#include "gui.hpp"

#include <stdio.h>
#include <string.h>

int main(int arg_count, const char** args)
{
   if (arg_count == 1) {
      return show_gui();
   }

   init_cstdio();

   if (arg_count != 2 or strcmp(args[1], "/?") == 0) {
      printf("Usage: <file>\r\n");

      return 1;
   }

   const char* file_path = args[1];

   return apply(file_path, printf) ? 0 : 1;
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
