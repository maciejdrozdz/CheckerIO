#if !defined(CHECKERIO_PLATFORM_H)

/*
  NOTE(maciek):
  
  CHECKERIO_INTERNAL:
    0 - Build for public release
    1 - Build for developer only
    
  CHECKERIO_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#ifdef __cplusplus
extern "C" {
#endif
    
    //
    // NOTE(maciek): Compilers
    //
    
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
    
#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif
    
#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
    // TODO(maciek): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif
    
#if COMPILER_MSVC
#include <intrin.h>
#endif
    
    //
    // NOTE(maciek): Types
    //
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
    
    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;
    
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    
    typedef float r32;
    typedef double r64;
    
    typedef int32_t b32;
    
    typedef size_t memory_index;
    
#define Real32Maximum FLT_MAX
    
#define internal static
#define local_persist static
#define global_variable static
    
#define Pi32 3.14159265359f
    
#if CHECKERIO_SLOW
    // TODO(maciek): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
    
#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break
    
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)
    
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
    // TODO(maciek): swap, min, max ... macros???
    
    inline u32
        SafeTruncateUInt64(u64 Value)
    {
        // TODO(maciek): Defines for maximum values
        Assert(Value <= 0xFFFFFFFF);
        u32 Result = (u32)Value;
        return(Result);
    }
    
    typedef struct thread_context
    {
        int Placeholder;
    } thread_context;
    
    /*
      NOTE(maciek): Services that the platform layer provides to the game
    */
#if CHECKERIO_INTERNAL
    /* IMPORTANT(maciek):
    
       These are NOT for doing anything in the shipping game - they are
       blocking and the write doesn't protect against lost data!
    */
    typedef struct debug_read_file_result
    {
        u32 ContentsSize;
        void *Contents;
    } debug_read_file_result;
    
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);
    
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
    
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(thread_context *Thread, char *Filename, u32 MemorySize, void *Memory)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
    
#endif
    
    typedef struct game_memory
    {
        b32 IsInitialized;
        b32 Reload;
        i64 CurretTime;
        
        u64 PermanentStorageSize;
        void *PermanentStorage; // NOTE(maciek): REQUIRED to be cleared to zero at startup
        
        u64 TransientStorageSize;
        void *TransientStorage; // NOTE(maciek): REQUIRED to be cleared to zero at startup
        
        debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
        debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
        debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    } game_memory;
    
    typedef enum request_type
    {
        REQUEST_INVALID = 0,
        
        REQUEST_MOVE,
        
        REQUEST_FIND_NEW_GAME,
        REQUEST_FOUND_NEW_GAME,
        
        REQUEST_OTHER_PLAYER_FINISHED,
        
        REQUEST_TYPES_COUNT
    } request_type;
    /*
    typedef struct request
    {
        request_type Type;
        void *Data;
    } request;
    */
    
    typedef struct request
    {
        char *Data;
        i32 Length;
    } request;
    
    typedef struct new_game_info
    {
        i32 PlayerID;
    } new_game_info;
    
    typedef enum response_data_type
    {
        RESPONSE_INVALID = 0,
        
        RESPONSE_JOINED_QUEUE, // NOTE(maciek): Returns number in queue
        RESPONSE_FOUND_NEW_GAME, // NOTE(maciek): Returns new_game_info
        
        RESPONSE_SERVER_ERROR, // TODO(maciek): Make server error numbers
        
        RESPONSE_COUNT
    } response_data_type;
    
    typedef struct response
    {
        response_data_type Type;
        char *Data;
        i32 Size;
    } response;
    
#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread, game_memory *Memory, request Request, response *Response)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
    
    // NOTE(maciek): At the moment, this has to be a very fast function, it cannot be
    // more than a millisecond or so.
    // TODO(maciek): Reduce the pressure on this function's performance by measuring it
    // or asking about it, etc.
    
#ifdef __cplusplus
}
#endif

#define CHECKERIO_PLATFORM_H
#endif
