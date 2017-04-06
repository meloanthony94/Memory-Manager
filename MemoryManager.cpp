#include "MemoryManager.h"


namespace MemoryManager
{
  const int POOL_SIZE = 65536;
  char Memory_Pool[POOL_SIZE];

  struct Header
  {
	  //How much memory is available in this block
	  unsigned int size;

	  // Pointer to the previous free header
	  Header * prev;

	  // Pointer to the next free header
	  Header * next;
  };

  struct Footer
  {
	  //How much memory is available in this block
	  unsigned int size;
  };

  Header* firstHeader;
  Header* freeListEntry;
  Footer* lastFooter;

  Header * FindBlock(unsigned int allocSize);


  // Initialize set up any data needed to manage the memory pool
  void initializeMemoryManager(void)
  {
	//Size of memory block minus header and footer
	reinterpret_cast<Header*>(Memory_Pool)->size = POOL_SIZE - sizeof(Header) - sizeof(Footer);

	//Only one block so far, so we point at ourselves
	reinterpret_cast<Header*>(Memory_Pool)->next = reinterpret_cast<Header*>(Memory_Pool);
	reinterpret_cast<Header*>(Memory_Pool)->prev = reinterpret_cast<Header*>(Memory_Pool);

	//Same goes for the first header and free data list entry point
	firstHeader = reinterpret_cast<Header*>(Memory_Pool);
	firstHeader->size = POOL_SIZE - sizeof(Header) - sizeof(Footer);
	firstHeader->next = firstHeader;
	firstHeader->prev = firstHeader;
	firstHeader->size &= ~(1 << 31); //free memory block

	//first entry in free list is the head of the list
	freeListEntry = firstHeader;

	//Footer
	//some_array[3] == *(some_array + 3)
	//*(Memory_Pool + POOL_SIZE - sizeof(Footer));

	//Set the footer of the pool
	reinterpret_cast<Footer*>((Memory_Pool + (POOL_SIZE - sizeof(Footer))))->size = POOL_SIZE - sizeof(Header) - sizeof(Footer);

	lastFooter = reinterpret_cast<Footer*>((Memory_Pool + (POOL_SIZE - sizeof(Footer))));
	lastFooter->size = POOL_SIZE - sizeof(Header) - sizeof(Footer);
  }

  // return a pointer inside the memory pool
  void* allocate(int aSize)
  {
	  //Find a memory block
	  Header* free_head = FindBlock(aSize);
	  Footer* used_foot = ((Footer*)(((char*)free_head) + free_head->size + sizeof(Header)));

	  Header* used_head = nullptr;
	  Footer* free_foot = nullptr;
	  void* data = nullptr;

	  //if a suitable memory block exists
	  if (free_head != nullptr)
	  {
		  //split logic
		  if (free_head->size - aSize >= (sizeof(Header) + sizeof(Footer) + 1))
		  {
			  //still free 
			  free_head->size = free_head->size - aSize - sizeof(Header) - sizeof(Footer);
			  free_foot = (Footer*)(((char*)free_head) + sizeof(Header) + free_head->size);
			  free_foot->size = free_head->size;
			  //used
			  used_head = ((Header*)(((char*)free_foot) + sizeof(Footer)));
			  used_head->size = aSize;
			  used_head->size |= (1 << 31); //set to used
			  used_foot->size = used_head->size;

			  data = (char*)(((char*)used_head) + sizeof(Header));
		  }
		  else //don't split
		  {
			  free_head->next->prev = free_head->prev;
			  free_head->prev->next = free_head->next;

			  free_head->size |= (1 << 31);

			  data = (char*)(((char*)free_head) + sizeof(Header));
		  }
	  }

	  return data;
  }

  // Free up a chunk previously allocated
  void deallocate(void* aPointer)
  {
	  if (aPointer != nullptr)
	  {
		  bool merged_left = false;
		  bool merged_right = false;

		  Header* myHead = (Header*)((char*)aPointer - sizeof(Header));

		  if ((myHead->size & (1 << 31))) //if the memory block in use
		  {
			  myHead->size &= ~(1 << 31); // free the memory block
			  Footer* myFoot = (Footer*)((char*)myHead + sizeof(Header) + myHead->size);
			  myFoot->size &= ~(1 << 31);

			  //right merge
			  //remove blockto the right so it can be merged with the block I'm freeing
			  if (myFoot != lastFooter)
			  {
				  Header* righthead = (Header*)((char*)myFoot + sizeof(Footer));

				  if (!(righthead->size & (1 << 31))) //free
				  {
					  Footer* rightfoot = (Footer*)((char*)righthead + righthead->size + sizeof(Header));
					  rightfoot->size += myFoot->size + sizeof(Header) + sizeof(Footer);
					  myFoot = rightfoot;
					  myHead->size = myFoot->size;

					  merged_right = true;

					  righthead->next->prev = righthead->prev;
					  righthead->prev->next = righthead->next;
				  }
			  }

			  //left merge
			  //expand memory block to the left because block on the left is already in the free list
			  if (myHead != firstHeader)
			  {
				  Footer* leftfoot = (Footer*)((char*)myHead - sizeof(Footer));

				  if (!(leftfoot->size & (1 << 31))) //free
				  {
					  Header* lefthead = ((Header*)(((char*)leftfoot) - leftfoot->size - sizeof(Header)));
					  lefthead->size += myHead->size + sizeof(Header) + sizeof(Footer);;
					  myHead = lefthead;
					  myFoot->size = myHead->size;

					  merged_left = true;
				  }
			  }

			  //add block to the free list
			  if (merged_left == false)
			  {
				  if (freeListEntry == nullptr)
				  {
					  myHead->next = myHead;
					  myHead->prev = myHead;

					  freeListEntry = myHead;
				  }
				  else
				  {
					  myHead->next = freeListEntry->next;
					  myHead->prev = freeListEntry;

					  freeListEntry->next->prev = myHead;
					  freeListEntry->next = myHead;
				  }
			  }
		  }
		  else
		  {
			  onIllegalOperation("data has already been deallocated");
		  }
	  }
  }

  Header * FindBlock(unsigned int allocSize)
  {
	  //Start at the front of the free list
	  Header * CurrentBlock = freeListEntry;

	  //Loop until a block with enough space is found
	  while (true)
	  {
		  if (CurrentBlock->size >= allocSize)
			  return CurrentBlock;
		  else
			  CurrentBlock = CurrentBlock->next;

		  //if only one spot exist
		  if (CurrentBlock == freeListEntry)
		  {
			  onOutOfMemory();
			  break;
		  }
	  }

	  //if no spaces are available that can accomidate that size
	  return nullptr;
  }

  // Will scan the memory pool and return the total free space remaining
  int freeRemaining(void)
  {
      return freeListEntry->size;
  }

  // Will scan the memory pool and return the largest free space remaining
  int largestFree(void)
  {

    //Loops through the free list, updating LargestSpace when a larger size is found
	Header* TempEntry = freeListEntry;
	unsigned int LargestSpace = TempEntry->size;

	do
	{
	  if(LargestSpace < TempEntry->size)
			LargestSpace = TempEntry->size;

	  TempEntry = TempEntry->next;
	} while (TempEntry != freeListEntry);

	return LargestSpace;
  }

  // will scan the memory pool and return the smallest free space remaining
  int smallestFree(void)
  {
	//Loops through the free list, updating LargestSpace when a larger size is found
	  Header* TempEntry = freeListEntry;
	  unsigned int SmallestSpace = TempEntry->size;

	  do
	  {
		  if (SmallestSpace > TempEntry->size)
			  SmallestSpace = TempEntry->size;

		  TempEntry = TempEntry->next;
	  } while (TempEntry != freeListEntry);

	  return SmallestSpace;
  }

 }