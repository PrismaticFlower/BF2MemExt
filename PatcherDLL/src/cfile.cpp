#include "pch.h"

#include "cfile.hpp"

cfile::cfile(const char* filename, const char* mode)
{
   if (fopen_s(&file, filename, mode) != errno) file = nullptr;
}

cfile::~cfile()
{
   if (file) fclose(file);
}

void cfile::printf(char const* const format, ...) const
{
   if (not file) return;

   va_list args;
   va_start(args, format);

   vfprintf(file, format, args);

   va_end(args);
}

cfile::operator bool() const noexcept
{
   return file;
}
