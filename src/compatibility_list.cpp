#include "compatibility_list.hpp"

const compatibile_exe compatibility_list[EXE_COUNT] = {
   compatibile_exe{
      .name = "BF2_modtools",
      .id_address = 0x62b59c,
      .expected_id = 0x746163696c707041,
   },

   compatibile_exe{
      .name = "BattlefrontII.exe GoG",
      .id_address = 0x39f298,
      .expected_id = 0x746163696c707041,
   },

   compatibile_exe{
      .name = "BattlefrontII.exe Steam",
      .id_address = 0x39e234,
      .expected_id = 0x746163696c707041,
   },
};