#pragma once

#include <stdio.h>

struct cfile {
   cfile(const char* filename, const char* mode);

   ~cfile();

   void printf(char const* const format, ...) const;

   explicit operator bool() const noexcept;

private:
   FILE* file = nullptr;
};