#if !defined(CHECKERIO_H)

#include <iostream>
#include <string>

#include "checkerio_platform.h"
#include "checkerio_intrinsics.h"
#include "checkerio_math.h"

#define internal static
#define global_variable static
#define local_persist static

#define Pi32 3.14159265359f

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

#if CHECKERIO_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
    
    i32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
#define PushSize(Arena, Size) PushSize_(Arena, Size)
inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    
    Result.Arena = Arena;
    Result.Used = Arena->Used;
    
    ++Arena->TempCount;
    
    return(Result);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void
ZeroSize(memory_index Size, void *Ptr)
{
    // TODO(casey): Check this guy for performance
    u8 *Byte = (u8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

struct player;
struct pawn
{
    v2i CurrentBoardPosition;
    b32 IsQueen;
    b32 IsAlive;
    b32 AtSafePosition;
    player *Owner;
};

#define PAWNS_COUNT_PER_PLAYER 12
struct player
{
    i32 PlayerNumber;
    i32 Direction;
    pawn Pawns[PAWNS_COUNT_PER_PLAYER];
    i32 PawnsAliveCount;
    i32 PlayerID;
};

struct field
{
    v2i BoardPosition;
    pawn *Pawn;
};

struct move
{
    v2i From;
    v2i To;
    pawn *KilledPawn;
    move *PreviousMove;
    player *PreviousPlayer;
};

struct transient_state
{
    b32 IsInitialized;
    memory_arena TranArena;
};

#define MAX_CHAIN_MOVES PAWNS_COUNT_PER_PLAYER
struct move_chain_link
{
    v2i From;
    v2i To;
    pawn *KilledPawn;
};

#define TOTAL_FIELDS_COUNT 64
struct game_state
{
    b32 IsOccupied;
    field BoardFields[TOTAL_FIELDS_COUNT];
    v2i BoardSize;
    
    move *LastMove;
    
    player Players[2];
    player *CurrentPlayer;
    
    move_chain_link MoveChain[MAX_CHAIN_MOVES];
    i32 CurrentChainMovesCount;
    
    i64 LastMoveTime;
};

typedef struct player_queue_entry
{
    i32 PlayerID;
} player_queue_entry;

struct request_move_data
{
    char From[2];
    char To[2];
    i32 PlayerID;
};

struct request_other_player_finished_data
{
    i32 PlayerID;
    i32 GameID;
};

enum move_result
{
    MOVE_OK, // NOTE(maciek): Returns nothing
    MOVE_INVALID, // NOTE(maciek): Returns nothing
    GAME_NO_LOGNER_EXISTS, // NOTE(maciek): Returns nothing
};

#define MAX_PLAYER_QUEUE_ENTRIES 1024
typedef struct players_queue
{
    player_queue_entry Entries[MAX_PLAYER_QUEUE_ENTRIES];
    i32 CurrentCount;
    i32 CurrentFirstInQueue;
} players_queue;

#define MAX_CLIENTS_COUNT 32
struct server_state
{
    memory_arena WorldArena;
    i32 UserPlayerNumber;
    
    std::string DEBUGLoadFileName;
    
    game_state GameStates[MAX_CLIENTS_COUNT];
    i32 CurrentClientsCount;
    
    players_queue PlayersQueue;
    
    i32 LastPlayerID;
};

#define CHECKERIO_PREFIX "CHECKERIO_"
#define STR_RESPONSE_MOVE_OK "MOVE_OK"
#define STR_RESPONSE_SERVER_ERROR "SERVER_ERROR"
#define STR_RESPONSE_MOVE_INVALID "MOVE_INVALID"
#define STR_RESPONSE_GAME_NO_LONGER_EXISTS "GAME_NO_LONGER_EXISTS"
#define STR_RESPONSE_FOUND_NEW_GAME "FOUND_NEW_GAME&&playerID="
#define STR_RESPONSE_GAME_NOT_YET_FOUND "GAME_NOT_YET_FOUND"
#define STR_RESPONSE_JOINED_QUEUE "JOINED_QUEUE"
#define STR_RESPONSE_GAME_FINISHED "GAME_FINISHED"
#define STR_RESPONSE_PLAYER_FINISHED "PLAYER_FINISHED"
#define STR_RESPONSE_PLAYER_NOT_FINISHED "PLAYER_NOT_FINISHED"

#define CHECKERIO_H
#endif