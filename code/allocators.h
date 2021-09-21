/* date = August 27th 2021 11:14 am */

#ifndef ALLOCATORS_H
#define ALLOCATORS_H

#include "utils.h"
//~ CUSTOM ALLOCATORS

//- Base Memory VTable

typedef void* alloc_reserve(memoryIndex size);
typedef void alloc_changeMemoryFunction(void* ptr,
                                        u64 size);

typedef struct alloc_BaseMemory {
    alloc_reserve* reserve;
    alloc_changeMemoryFunction* commit;
    alloc_changeMemoryFunction* decommit;
    alloc_changeMemoryFunction* release;
} alloc_BaseMemory;

//- Base Memory Helper Functions
internal void
alloc_changeMemoryNOOP(void* ptr,
                       u64 size);

//- Malloc Wrappers
// NOTE(Jai): With the CRT, reserve & commit, and release & decommit are coupled, but we still want
// the general template of the them seperated as if we choose to use the OS APIs to allocate
// memory, we can reserve without committing, and decommit without releasing.
internal void*
alloc_malloc_reserve(u64 size);
internal void
alloc_malloc_commit(void* ptr,
                    u64 size);
internal void
alloc_malloc_decommit(void* ptr,
                      u64 size);
internal void
alloc_malloc_release(void* ptr,
                     u64 size);
internal alloc_BaseMemory
alloc_baseMemory_use_malloc(void);

//- Arena:
// used http://www.gingerbill.org/series/memory-allocation-strategies/ for ref.
// NOTE(Jai): A general purpose arena allocator.
// Use by initializing with alloc_arena_initialize(Arena* arena, void* backingBuffer,
//                                                 memoryIndex backingBufferLength);
// use the Macros defined below to interface with the arena :
// ARENA_PUSH_STRUCT(arena, type)
// ARENA_PUSH_STRUCTALIGNED(arena, type, align)
// ARENA_PUSH_ARRAY(arena, count, type)
// ARENA_PUSH_ARRAYALIGNED(arena, count, type, align)
// Finally, free the arena by using alloc_arena_free(Arena* arena)

// NOTE(Jai): Arena as defined in "utils.h"
//typedef struct Arena {
//        u8* buffer; // Pointer to the buffer, must point to an allocated block of
//                       memory on initiliazation
//        memoryIndex bufferSize; // Size of the buffer in bytes
//        memoryIndex currentOffset; // The current offset to the end of the used memory,
//                                      initialize to zero
//        memoryIndex memoryBlockStart; // The previous offset, set using function,
//                                         initialize to zero
//} Arena;
//

internal uintptr
alloc_arena_alignForward(uintptr ptr,
                         memoryIndex align);
internal void*
alloc_arena_allocAlign(Arena* arena,
                       memoryIndex allocation,
                       memoryIndex align);
internal void*
alloc_arena_alloc(Arena* arena,
                  memoryIndex size);
internal void*
alloc_arena_resizeAlign(Arena* arena,
                        void* oldMemory,
                        memoryIndex oldSize,
                        memoryIndex newSize,
                        memoryIndex align);
internal void*
alloc_arena_resize(Arena* arena,
                   void* oldMemory,
                   memoryIndex oldSize,
                   memoryIndex newSize);
internal void
alloc_arena_initialize(Arena* arena,
                       void* backingBuffer,
                       memoryIndex backingBufferLength);
internal void
alloc_arena_free(Arena* arena);
#endif //ALLOCATORS_H

#if defined (JAI_ALLOCATORS_IMPLEMENTATION)
//~ ___ IMPLEMENTATION ___

//- Base Memory

internal void*
alloc_malloc_reserve(u64 size) {
    return malloc(size);
}

internal void
alloc_malloc_commit(void* ptr,
                    u64 size) {}

internal void
alloc_malloc_decommit(void* ptr,
                      u64 size) {}

internal void
alloc_malloc_release(void* ptr,
                     u64 size) {
    free(ptr);
}

internal alloc_BaseMemory
alloc_baseMemory_use_malloc(void) {
    localPersist alloc_BaseMemory memory = {0};
    if (memory.reserve == 0) {
        memory.reserve = alloc_malloc_reserve;
        memory.commit = alloc_malloc_commit;
        memory.decommit = alloc_malloc_decommit;
        memory.release = alloc_malloc_release;
    }
    
    return memory;
}

//- Arena:

internal uintptr
alloc_arena_alignForward(uintptr ptr,
                         memoryIndex align) {
	ASSERT(IS_POWER_OF_2(align));
	uintptr result, a, modulo;
	result = ptr;
	//modulo = p & 7;
	a = (uintptr)align;
	modulo = result & (a - 1);
	if (modulo != 0) { result += a - modulo; }
    
	return result;
}


internal void*
alloc_arena_allocAlign(Arena* arena,
                       memoryIndex allocation,
                       memoryIndex align) {
	void* result = 0;
    uintptr currentPtr = (uintptr)arena->buffer + (uintptr)arena->currentOffset;
	// Calculate the offset
	uintptr offset = alloc_arena_alignForward(currentPtr,
                                              align);
	// Change to relative offset
	offset -= (uintptr)arena->buffer;
	// Check to see if the backing memory has space left
	if (offset + allocation < arena->bufferSize) {
		result = &arena->buffer[offset];
		arena->memoryBlockStart = arena->currentOffset;
		arena->currentOffset = allocation + offset;
        
        // Zero new memory by default
		memset(result, 0, allocation);
	}
    
	return result;
}

#ifndef DEFAULT_ARENA_ALIGNMENT
#define DEFAULT_ARENA_ALIGNMENT (2 * sizeof(void*))
#endif

// NOTE(Jai): Push a Struct into the arena
#define ARENA_PUSH_STRUCT(arena, type) (type*)alloc_arena_alloc(arena, sizeof(type))
#define ARENA_PUSH_STRUCTALIGNED(arena, type, align) (type*)alloc_arena_allocAlign(arena, sizeof(type), align)
// NOTE(Jai): Push an Array into the arena
#define ARENA_PUSH_ARRAY(arena, count, type) (type*)alloc_arena_alloc(arena, (count * sizeof(type)))
#define ARENA_PUSH_ARRAYALIGNED(arena, count, type, align)\
(type*)alloc_arena_allocAlign(arena, (count * sizeof(type)), align)

// Because C doesn't allow default parameters
internal void*
alloc_arena_alloc(Arena* arena,
                  memoryIndex size) {
	return alloc_arena_allocAlign(arena,
                                  size,
                                  DEFAULT_ARENA_ALIGNMENT);
}

// NOTE(Jai): Resize an arena and align the address to a custom alignment
internal void*
alloc_arena_resizeAlign(Arena* arena,
                        void* oldMemory,
                        memoryIndex oldSize,
                        memoryIndex newSize,
                        memoryIndex align) {
	ASSERT(IS_POWER_OF_2(align));
	if (oldMemory == NULL || oldSize == 0) {
		return alloc_arena_allocAlign(arena,
                                      newSize,
                                      align);
	} else if (arena->buffer <= (u8*)oldMemory
			   && (u8*)oldMemory < (arena->buffer + arena->bufferSize)) {
		if (arena->buffer + arena->memoryBlockStart == oldMemory) {
			arena->currentOffset = arena->memoryBlockStart + newSize;
			if (newSize > oldSize) {
				memset(&arena->buffer[arena->currentOffset],
					   0,
					   (newSize - oldSize));
			}
            
			return oldMemory;
		} else {
			void* newMemory = alloc_arena_allocAlign(arena,
                                                     newSize,
                                                     align);
			memoryIndex copySize = oldSize < newSize ? oldSize : newSize;
			memmove(newMemory,
					oldMemory,
					copySize);
            
			return newMemory;
		}
	} else {
		ASSERT(0 && "Memory is out of bounds of the buffer in this arena");
		return NULL;
	}
}

// Because C doesn't allow default parameters
internal void*
alloc_arena_resize(Arena* arena,
                   void* oldMemory,
                   memoryIndex oldSize,
                   memoryIndex newSize) {
	return alloc_arena_resizeAlign(arena,
                                   oldMemory,
                                   oldSize,
                                   newSize,
                                   DEFAULT_ARENA_ALIGNMENT);
}

// NOTE(Jai): Initialize the arena
internal void
alloc_arena_initialize(Arena* arena,
                       void* backingBuffer,
                       memoryIndex backingBufferLength) {
    arena->buffer = (u8*)backingBuffer;
    arena->bufferSize = backingBufferLength;
    arena->currentOffset = 0;
    arena->memoryBlockStart = 0;
}

internal void
alloc_zeroSize(memoryIndex size,
               void* ptr) {
    u8* byte = (u8*)ptr;
    while(size--) {
        *byte++ = 0;
    }
}
#define ZERO_STRUCT(instance) alloc_zeroSize(sizeof(instance), &(instance))

// NOTE(Jai): Completely frees the arena
internal void
alloc_arena_free(Arena* arena) {
	arena->currentOffset = 0;
	arena->memoryBlockStart = 0;
}

#undef JAI_ALLOCATORS_IMPLEMENTATION
#endif // IMPLEMENTATION

