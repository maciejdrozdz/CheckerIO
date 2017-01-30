#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH

struct win32_state
{
    u64 TotalSize;
    void *GameMemoryBlock;
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
    };
    
    struct win32_game_code
    {
        HMODULE GameCodeDLL;
        FILETIME DLLLastWriteTime;
        
        // IMPORTANT(casey): Either of the callbacks can be 0!  You must
        // check before calling.
        game_update_and_render *UpdateAndRender;
        
        b32 IsValid;
};