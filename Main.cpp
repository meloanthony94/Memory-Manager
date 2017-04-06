#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>

#include "MemoryManager.h"

int main(void)
{
  using namespace MemoryManager;

  initializeMemoryManager();

  long* int_pointer;
  char* string_pointer;

  std::cout << "Free memory = " << freeRemaining() << std::endl;

  int_pointer = (long *) allocate(sizeof(long));
  string_pointer = (char*) allocate(255);

  *int_pointer = 0xDEADBEEF;
  strcpy(string_pointer,"If your happy and you know it, clap your hands");

  std::cout << "Free memory = " << freeRemaining() << std::endl;

    deallocate(int_pointer);
    deallocate(string_pointer);
}

namespace MemoryManager
{
  void onOutOfMemory(void)
  {
    std::cerr << "Memory pool out of memory" << std::endl;
    exit( 1 );
  }

  void onIllegalOperation(const char* fmt,...)
  {
    if ( fmt == NULL )
    {
      std::cerr << "Unknown illegal operation" << std::endl;
      exit( 1 );
    }
    else
    {
      char	buf[8192];

      va_list argptr;
      va_start (argptr,fmt);
      vsprintf (buf,fmt,argptr);
      va_end (argptr);

      std::cerr << "Illegal operation: \"" << buf << "\"" << std::endl;
      exit( 1 );
    }
  }
}

