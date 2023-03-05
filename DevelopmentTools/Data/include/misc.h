/* *****************************************************************************
*  Vircon32 standard library: "misc.h"            File version: 2023/03/05     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file allows programs to use Vircon CPU's native functions for memory   *
*  operations and random number generation. This header also implements basic  *
*  dynamic memory management. It also includes other miscellaneous or general  *
*  use functions that don't match the scope of other headers.                  *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef MISC_H
    #define MISC_H
// *****************************************************************************


// =============================================================================
//   CPU MEMORY FUNCTIONS
// =============================================================================


void memset( void* destination, int value, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {destination}"
        "mov SR, {value}"
        "sets"
    }
}

// -----------------------------------------------------------------------------

void memcpy( void* destination, void* source, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {destination}"
        "mov SR, {source}"
        "movs"
    }
}

// -----------------------------------------------------------------------------

int memcmp( void* region1, void* region2, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {region1}"
        "mov SR, {region2}"
        "cmps R0"
    }
}


// =============================================================================
//   DYNAMIC MEMORY MANAGEMENT: DEFINITIONS AND AUXILIARY FUNCTIONS
// =============================================================================


// with this structure we implement a doubly linked list
// of blocks that mark each region within the memory range
// usable by dynamic memory; we need them to be a list so
// that adjacent blocks can be merged when freed, and
// large free blocks can be split into smaller ones; data
// for each block will begin in the next address after it,
// so: data adress = block address + sizeof( malloc_block )
struct malloc_block
{
    malloc_block* previous;
    malloc_block* next;
    int size;
    bool free;
};

// -----------------------------------------------------------------------------

// these variables can be modified before malloc is first
// used, to change the memory region usable for dynamic
// memory (changes after that first call have no effect);
// by default we leave the 1st MWord for globals, and the
// last MWord for stack, so we can use up to 50% of RAM
void* malloc_start_address = (void*)(1 * 1024 * 1024);
void* malloc_end_address   = (void*)(3 * 1024 * 1024 - 1);

// this first block marks the start of our block list; it
// will be allocated and initialized on first malloc use
malloc_block* malloc_first_block = NULL;

// -----------------------------------------------------------------------------

// auxiliary function used whenever a block is freed or
// a new free block is created; it merges that new free
// block with its 2 adjacent neighbors, when either of
// them are also free; we do this to have fewer, larger
// free blocks that are faster to treat and allow larger
// chunks to be allocated in the future
void merge_free_malloc_blocks( malloc_block* block )
{
    // obtain adjacent blocks
    malloc_block* previous_block = block->previous;
    malloc_block* next_block = block->next;
    
    // if we can, merge with next block
    if( next_block && next_block->free )
    {
        // remove next block from the list
        block->next = next_block->next;
        
        if( next_block->next )
          next_block->next->previous = block;
        
        // assign the combined size to current block
        block->size += next_block->size + sizeof( malloc_block );
        
        // careful! we must now update this pointer
        // for a possible backwards merge below!
        next_block = block->next;
    }
    
    // if we can, merge with previous block
    // (this must be done AFTER merging with next!)
    if( previous_block && previous_block->free )
    {
        // remove current block from the list
        previous_block->next = next_block;
        
        if( next_block )
          next_block->previous = previous_block;
        
        // assign the combined size to previous block
        previous_block->size += block->size + sizeof( malloc_block );
    }
}

// -----------------------------------------------------------------------------

// auxiliary function used by realloc when attempting to
// reduce a partially used block in place, by splitting
// its unused part into a free block when possible
void reduce_malloc_block( malloc_block* block, int new_size )
{
    // abort if block split is not possible
    int reduced_size = block->size - new_size;
    
    if( reduced_size <= sizeof( malloc_block ) )
      return;
    
    // create a new free block at the end of
    // the previous, having the excess size
    malloc_block* freed_block = (malloc_block*)( (int*)(block + 1) + new_size );
    freed_block->previous = block;
    freed_block->next = block->next;
    freed_block->free = true;
    freed_block->size = reduced_size - sizeof( malloc_block );
    
    // adjust its adjacent blocks
    block->size = new_size;
    block->next = freed_block;
    
    if( freed_block->next )
      freed_block->next->previous = freed_block;
    
    // if we can, merge new free block it with its next block
    merge_free_malloc_blocks( freed_block );
}

// -----------------------------------------------------------------------------

// auxiliary function used by realloc to expand a block by merging it with
// the following one when it's free, so that expansin is in place;
// returns true when the block could be expanded, false if not
bool expand_malloc_block( malloc_block* block, int new_size )
{
    int increased_size = new_size - block->size;
    if( increased_size <= 0 ) return true;
    
    // cannot expand in place if next block is not free
    malloc_block* next_block = block->next;
    if( !next_block || !next_block->free ) return false;
    
    // can only expand if next block is large enough
    int usable_size = next_block->size + sizeof( malloc_block );
    if( usable_size < increased_size ) return false;
    
    // merge this block with the next one by removing
    // the next from the list and adjusting size
    block->size += usable_size;
    block->next = next_block->next;
    
    if( next_block->next )
      next_block->next->previous = block;
    
    // after merging, try to re-split resulting block
    // to adapt to new size, in case it is too large
    reduce_malloc_block( block, new_size );
    return true;
}


// =============================================================================
//   DYNAMIC MEMORY MANAGEMENT: END-USER FUNCTIONS
// =============================================================================


// allocates the requested amount of memory and returns a
// pointer to its start address, or NULL if allocation failed
void* malloc( int size )
{
    // on first use, initialize the first block as the single
    // free block covering all of the usable memory region
    if( !malloc_first_block )
    {
        malloc_first_block = (malloc_block*)malloc_start_address;
        malloc_first_block->size = ((int*)malloc_end_address - (int*)malloc_start_address + 1);
        malloc_first_block->size -= sizeof( malloc_block );
        malloc_first_block->previous = NULL;
        malloc_first_block->next = NULL;
        malloc_first_block->free = true;
    }
    
    // for size 0, malloc does nothing
    if( size <= 0 ) return NULL;
    
    // look for a large enough free block in the list
    malloc_block* current_block = malloc_first_block;
    
    while( current_block )
    {
        if( current_block->free && current_block->size >= size )
          break;
        
        current_block = current_block->next;        
    }
    
    // abort if a suitable block was not found
    if( !current_block ) return NULL;
    
    // for a block large enough, split it and take the second
    // part (leave first part free for faster future allocations)
    int split_size = size + sizeof( malloc_block );
    
    if( current_block->size > split_size )
    {
        // create a new block at the end of the free block
        malloc_block* new_block = (malloc_block*)( (int*)(current_block + 1) + current_block->size - split_size );
        new_block->size = size;
        new_block->free = false;
        new_block->previous = current_block;
        new_block->next = current_block->next;
        
        // adjust current block to its new size and neighbor
        current_block->size -= split_size;
        current_block->next = new_block;
        
        if( new_block->next )
          new_block->next->previous = new_block;
        
        // provide data after the new block
        return (new_block + 1);
    }
    
    // otherwise just take the full block
    // and provide the data after it
    else
    {
        current_block->free = false;
        return (current_block + 1);
    }
}

// -----------------------------------------------------------------------------

// releases memory allocation of a pointer previously returned by
// malloc, calloc or realloc; note it does not set ptr to NULL;
// calling free with an invalid ptr will cause memory corrpution!
void free( void* ptr )
{
    // it is actually safe to free NULL in C
    if( !ptr ) return;
    
    // find the corresponding block and free it
    malloc_block* freed_block = (malloc_block*)ptr - 1;
    freed_block->free = true;
    
    // merge this block with its previous and next
    // neighbors when either of them are also free
    merge_free_malloc_blocks( freed_block );
}

// -----------------------------------------------------------------------------

// allocates memory for the requested number of elements, of
// the same size each; it also initializes memory to zeroes
void* calloc( int number, int size )
{
    // first allocate memory as usual
    int total_size = number * size;
    void* allocated_memory = malloc( total_size );
    
    // check that allocation succeeded
    if( !allocated_memory ) return NULL;
    
    // fill region with zeroes before providing it
    memset( allocated_memory, 0, total_size );
    return allocated_memory;
}

// -----------------------------------------------------------------------------

// reallocates a previously allocated pointer; when possible
// it will keep the same location, otherwise it will copy all
// data in the previous region to the new location; if new
// size is smaller, previous data will get truncated in place;
// calling realloc with an invalid ptr will cause memory corrpution!
void* realloc( void* ptr, int size )
{
    // for NULL ptr, realloc behaves like malloc
    if( !ptr ) return malloc( size );
    
    // for size 0, realloc behaves like free
    if( size <= 0 )
    {
        free( ptr );
        return NULL;
    }
    
    // obtain the corresponding block and
    // determine previous allocated size
    malloc_block* current_block = (malloc_block*)ptr - 1;
    int previous_size = current_block->size;
    
    // CASE 1: for the same size, do nothing
    if( size == previous_size )
      return ptr;
    
    // CASE 2: when reducing 
    if( size < previous_size )
    {
        // attempt to reduce the block by splitting it,
        // but do nothing if reduction was not possible
        reduce_malloc_block( current_block, size );
        return ptr;
    }
    
    // CASE 3: when increasing
    else
    {
        // 3-A: if we can, grow by merging with next block
        // (so that we can keep allocation in the same place)
        if( expand_malloc_block( current_block, size ) )
          return ptr;
        
        // 3-B: otherwise we just allocate a new space, and
        // all data needs to be copied to the new location
        else
        {
            // use malloc to find new allocation (if this fails
            // old allocation is NOT freed and remains valid!)
            void* new_ptr = malloc( size );
            if( !new_ptr ) return NULL;
            
            // copy all previous region to the new location
            memcpy( new_ptr, ptr, previous_size );
            
            // previous allocation is no longer needed now
            free( ptr );
            return new_ptr;
        }
    }
}


// =============================================================================
//   RANDOM NUMBER GENERATION
// =============================================================================


int rand()
{
    asm
    {
        "in R0, RNG_CurrentValue"
    }
}

// -----------------------------------------------------------------------------

// because of limitations of our specific generator, value
// 0 will never be actually set as seed (request is ignored)
void srand( int seed )
{
    asm
    {
        "mov R0, {seed}"
        "out RNG_CurrentValue, R0"
    }
}


// =============================================================================
//   PROGRAM FLOW
// =============================================================================


// since there are no exit codes from main, there
// is no need to also provide C's "abort" function
void exit()
{
    asm
    {
        "hlt"
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
