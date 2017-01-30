#include "checkerio_platform.h"

#include <windows.h>

#include "win_checkerio.h"

global_variable b32 GlobalRunning;
global_variable i64 GlobalPerfCountFrequency;

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {                    
                    // TODO(casey): Logging
                    DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    b32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE(casey): File read successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // TODO(casey): Logging
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(Result);
}

internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(casey): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if(GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }
    
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    }

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
    win32_game_code Result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
        
        CopyFile(SourceDLLName, TempDLLName, FALSE);
        
        Result.GameCodeDLL = LoadLibraryA(TempDLLName);
        if(Result.GameCodeDLL)
        {
            Result.UpdateAndRender = (game_update_and_render *)
                GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
            
            Result.IsValid = Result.UpdateAndRender ? 1 : 0;
        }
    }
    
    if(!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        }
    
    return(Result);
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline r32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    r32 Result = ((r32)(End.QuadPart - Start.QuadPart) /
                     (r32)GlobalPerfCountFrequency);
    return(Result);
}

#if 0
int
main(int argc, int **argv)
{
    win32_state Win32State = {};
    
    Win32GetEXEFileName(&Win32State);
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    UINT DesiredSchedulerMS = 1;
    b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "checkerio.dll",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
    
    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "checkerio_temp.dll",
                              sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);
    
    char GameCodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.tmp",
                              sizeof(GameCodeLockFullPath), GameCodeLockFullPath);
    
#if CHECKERIO_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(16);
    GameMemory.TransientStorageSize = Megabytes(16);//Gigabytes(1);
    GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
    GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
    GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
    
    Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize,
                                              MEM_RESERVE|MEM_COMMIT,
                                              PAGE_READWRITE);
    GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
    GameMemory.TransientStorage = ((u8 *)GameMemory.PermanentStorage +
                                   GameMemory.PermanentStorageSize);
    
    r32 GameUpdateHz = (60.0f / 2.0f);
    r32 TargetSecondsPerFrame = 1.0f / (r32)GameUpdateHz;
    
    LARGE_INTEGER LastCounter = Win32GetWallClock();
    LARGE_INTEGER FlipWallClock = Win32GetWallClock();
    
    GlobalRunning = true;
    
    win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                             TempGameCodeDLLFullPath,
                                             GameCodeLockFullPath);
    
    u64 LastCycleCount = __rdtsc();
    while(GlobalRunning)
    {
        FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
        if(CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0)
        {
            Win32UnloadGameCode(&Game);
            Game = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                     TempGameCodeDLLFullPath,
                                     GameCodeLockFullPath);
                    }
        
        thread_context Thread = {};
        
        if(Game.UpdateAndRender)
        {
            Game.UpdateAndRender(&Thread, &GameMemory);
        }
        
        LARGE_INTEGER WorkCounter = Win32GetWallClock();
        r32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
        
        // TODO(casey): NOT TESTED YET!  PROBABLY BUGGY!!!!!
        r32 SecondsElapsedForFrame = WorkSecondsElapsed;
        if(SecondsElapsedForFrame < TargetSecondsPerFrame)
        {                        
            if(SleepIsGranular)
            {
                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
                                                   SecondsElapsedForFrame));
                if(SleepMS > 0)
                {
                    Sleep(SleepMS);
                }
            }
            
            r32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                       Win32GetWallClock());
            if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
            {
                // TODO(casey): LOG MISSED SLEEP HERE
            }
            
            while(SecondsElapsedForFrame < TargetSecondsPerFrame)
            {                            
                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                Win32GetWallClock());
            }
        }
        else
        {
            // TODO(casey): MISSED FRAME RATE!
            // TODO(casey): Logging
        }
        
        LARGE_INTEGER EndCounter = Win32GetWallClock();
        r32 MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);                    
        LastCounter = EndCounter;
    }
    
    return(0);
}

#endif