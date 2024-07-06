#include "patch_table.hpp"

const exe_patch_list patch_lists[EXE_COUNT] = {
   exe_patch_list{
      .name = "BF2_modtools",
      .id_address = 0x62b59c,
      .expected_id = 0x746163696c707041,
      .patches =
         {
            patch_set{
               .name = "RedMemory Heap Extensions",
               .patches =
                  {
                     patch{0x337921, {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x10}}, // malloc call arg
                     patch{0x33792c, {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x10}}, // malloc'd block end pointer
                  },
            },
         },
   },

   exe_patch_list{
      .name = "BattlefrontII.exe GoG",
      .id_address = 0x39f298,
      .expected_id = 0x746163696c707041,
      .patches =
         {
            patch_set{
               .name = "RedMemory Heap Extensions",
               .patches =
                  {
                     patch{0x217651, {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x10}}, // malloc call arg
                     patch{0x217667, {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x10}}, // malloc'd block end pointer
                  },
            },
         },

   },

   exe_patch_list{
      .name = "BattlefrontII.exe Steam",
      .id_address = 0x39e234,
      .expected_id = 0x746163696c707041,
      .patches =
         {
            patch_set{
               .name = "RedMemory Heap Extensions",
               .patches =
                  {
                     patch{0x2165b1, {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x10}}, // malloc call arg
                     patch{0x2165c7, {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x10}}, // malloc'd block end pointer
                  },
            },
         },

   },

};