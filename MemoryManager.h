#pragma once

#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__

namespace MemoryManager
{
  // Initialize any data needed to manage the memory pool
  void initializeMemoryManager(void);

  // return a pointer inside the memory pool
  void* allocate(int aSize);

  // Free up a chunk previously allocated
  void deallocate(void* aPointer);

  // return the total free space remaining
  int freeRemaining(void);

  // return the largest free space remaining
  int largestFree(void);

  //return the smallest free space remaining
  int smallestFree(void);

  //no space is left for the allocation request
  void onOutOfMemory(void);

  void onIllegalOperation(const char* fmt,...);


};


#endif  // __MEMORY_MANAGER_H__
