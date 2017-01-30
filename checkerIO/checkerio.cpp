#include "checkerio.h"
#include "checkerio_string.h"

#if CHECKERIO_INTERNAL
inline void
SaveCurrentMemoryState(game_memory *GameMemory, char *FileName)
{
    GameMemory->DEBUGPlatformWriteEntireFile(0, FileName, (u32)GameMemory->PermanentStorageSize, GameMemory->PermanentStorage);
    std::cout << "Saved: " << FileName << std::endl;
}

inline debug_read_file_result
LoadMemoryState(game_memory *GameMemory, char *FileName)
{
    debug_read_file_result Result = GameMemory->DEBUGPlatformReadEntireFile(0, FileName);
    return(Result);
}
#endif

inline b32 
IsPositionValid(game_state *GameState, v2i Position)
{
    b32 Result = false;
    if(Position.x >= 0 && Position.x < GameState->BoardSize.x &&
       Position.y >= 0 && Position.y < GameState->BoardSize.y)
    {
        Result = true;
    }
    return(Result);
}

inline b32
IsPositionSafe(game_state *GameState, v2i Position)
{
    b32 Result = false;
    if(Position.x == 0 || Position.x == GameState->BoardSize.x || 
       Position.y == 0 || Position.y == GameState->BoardSize.y)
    {
        Result = true;
    }
    return(Result);
}

inline field *
GetBoardFieldAtPosition(game_state *GameState, v2i Position)
{
    field *Result = 0;
    if(IsPositionValid(GameState, Position))
    {
        Result = GameState->BoardFields + GameState->BoardSize.x*Position.y + Position.x;
    }
    return(Result);
}

inline pawn *
GetPawnAtPosition(game_state *GameState, v2i Position, b32 GetDead = false)
{
    pawn *Result = 0;
    field *FieldAt = GetBoardFieldAtPosition(GameState, Position);
    if(FieldAt)
    {
        Result = FieldAt->Pawn;
    }
    if(Result && !Result->IsAlive && !GetDead)
    {
        Result = 0;
    }
    return(Result);
}

inline void
KillPawn(game_state *GameState, pawn *Pawn)
{
    field *Field = GetBoardFieldAtPosition(GameState, Pawn->CurrentBoardPosition);
    Field->Pawn->IsAlive = false;
    Field->Pawn->Owner->PawnsAliveCount--;
    Field->Pawn = 0;
}

inline pawn *
GetPlayersPawnForIndex(game_state *GameState, player *Player, i32 PawnIndex, b32 GetDead = false)
{
    pawn *Result = Player->Pawns + PawnIndex;
    if(Result && !Result->IsAlive && !GetDead)
    {
        Result = 0;
    }
    return(Result);
}

internal b32
CanPawnMove(game_state *GameState, pawn *Pawn)
{
    b32 Result = false;
    
    v2i PossibleMoves[4] = { V2i(Pawn->CurrentBoardPosition.x - 1, Pawn->CurrentBoardPosition.y + 1),
        V2i(Pawn->CurrentBoardPosition.x + 1, Pawn->CurrentBoardPosition.y + 1),
        V2i(Pawn->CurrentBoardPosition.x - 1, Pawn->CurrentBoardPosition.y - 1),
        V2i(Pawn->CurrentBoardPosition.x + 1, Pawn->CurrentBoardPosition.y - 1) };
    
    for(i32 i = 0;
        i < ArrayCount(PossibleMoves);
        i++)
    {
        if(IsPositionValid(GameState, PossibleMoves[i]) && !GetPawnAtPosition(GameState, PossibleMoves[i]))
        {
            Result = true;
            break;
        }
    }
    
    return(Result);
}

struct can_pawn_kill_result
{
    b32 Result;
    move KillMoves[4];
};

internal can_pawn_kill_result
CanPawnKill(game_state *GameState, pawn *Pawn, b32 FindAllTargets = false)
{
    can_pawn_kill_result Result = {};
    i32 KillCounter = 0;
    if(Pawn->IsQueen)
    {
        v2i DirectionVectors[4] = { V2i(-1, 1), V2i(1, 1), V2i(-1, -1), V2i(1, -1) };
        
        for(i32 i = 0;
            i < ArrayCount(DirectionVectors);
            i++)
        {
            b32 CouldKillPrevious = false;
            b32 BreakLoop = false;
            pawn *PawnToKill = 0;
            for(i32 j = 0;
                j < GameState->BoardSize.x;
                j++)
            {
                v2i CheckPosition = V2i(Pawn->CurrentBoardPosition.x + DirectionVectors[i].x*(j + 1),
                                        Pawn->CurrentBoardPosition.y + DirectionVectors[i].y*(j + 1));
                if(!IsPositionValid(GameState, CheckPosition))
                {
                    break;
                }
                
                pawn *PawnAtCheckPosition = GetPawnAtPosition(GameState, CheckPosition);
                if(CouldKillPrevious)
                {
                    if(!PawnAtCheckPosition)
                    {
                        Result.Result = true;
                        Result.KillMoves[KillCounter++].KilledPawn = PawnToKill;
                        Result.KillMoves[KillCounter++].From = Pawn->CurrentBoardPosition;
                        Result.KillMoves[KillCounter++].To = CheckPosition;
                        
                        if(!FindAllTargets)
                        {
                            BreakLoop = true;
                        }
                        break;
                    }
                    else
                    {
                        CouldKillPrevious = false;
                    }
                }
                else
                {
                    if(PawnAtCheckPosition)
                    {
                        if(PawnAtCheckPosition->Owner != Pawn->Owner)
                        {
                            CouldKillPrevious = true;
                            PawnToKill = PawnAtCheckPosition;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                
            }
            if(BreakLoop)
            {
                break;
            }
        }
    }
    else
    {
        v2i PossibleMoves[4] = { V2i(Pawn->CurrentBoardPosition.x - 2, Pawn->CurrentBoardPosition.y + 2),
            V2i(Pawn->CurrentBoardPosition.x + 2, Pawn->CurrentBoardPosition.y + 2),
            V2i(Pawn->CurrentBoardPosition.x - 2, Pawn->CurrentBoardPosition.y - 2),
            V2i(Pawn->CurrentBoardPosition.x + 2, Pawn->CurrentBoardPosition.y - 2) };
        
        for(i32 i = 0;
            i < ArrayCount(PossibleMoves);
            i++)
        {
            if(IsPositionValid(GameState, PossibleMoves[i]) && !GetPawnAtPosition(GameState, PossibleMoves[i]))
            {
                v2i MiddlePoint = V2i(Lerp(Pawn->CurrentBoardPosition.x, 1, PossibleMoves[i].x),
                                      Lerp(Pawn->CurrentBoardPosition.y, 1, PossibleMoves[i].y));
                
                pawn *PawnToKill = GetPawnAtPosition(GameState, MiddlePoint);
                if(PawnToKill && PawnToKill->Owner != Pawn->Owner)
                {
                    Result.Result = true;
                    Result.KillMoves[KillCounter++].KilledPawn = PawnToKill;
                    Result.KillMoves[KillCounter++].From = Pawn->CurrentBoardPosition;
                    Result.KillMoves[KillCounter++].To = MiddlePoint;
                    
                    if(!FindAllTargets)
                    {
                        break;
                    }
                }
            }
        }
    }
    
    return(Result);
}

internal b32
CanPawnDie(game_state *GameState, pawn *Pawn)
{
    b32 Result = false;
    if(!IsPositionSafe(GameState, Pawn->CurrentBoardPosition))
    {
        v2i DirectionVectors[4] = { V2i(-1, 1), V2i(1, 1), V2i(-1, -1), V2i(1, -1) };
        for(i32 i = 0;
            i < ArrayCount(DirectionVectors);
            i++)
        {
            v2i PositionBeihind = Pawn->CurrentBoardPosition + (DirectionVectors[i]*(-1));
            pawn *PawnBehind = GetPawnAtPosition(GameState, PositionBeihind);
            if(!PawnBehind)
            {
                b32 BreakLoop = false;
                for(i32 j = 0;
                    j < GameState->BoardSize.x;
                    j++)
                {
                    v2i CheckPosition = V2i(Pawn->CurrentBoardPosition.x + DirectionVectors[i].x*(j + 1),
                                            Pawn->CurrentBoardPosition.y + DirectionVectors[i].y*(j + 1));
                    pawn *PawnAtCheckPosition = GetPawnAtPosition(GameState, CheckPosition);
                    
                    if(PawnAtCheckPosition)
                    {
                        if(PawnAtCheckPosition->Owner != Pawn->Owner)
                        {
                            if(PawnAtCheckPosition->IsQueen || j == 0)
                            {
                                Result = true;
                            }
                        }
                    }
                }
                if(BreakLoop)
                {
                    break;
                }
            }
        }
    }
    
    return(Result);
}

internal b32
CanPlayerKill(game_state *GameState, player *Player, move *PreviousMove)
{
    b32 Result = false;
    if(PreviousMove)
    {
        pawn *Pawn = GetPawnAtPosition(GameState, PreviousMove->To);
        if(Pawn)
        {
            Result = CanPawnKill(GameState, Pawn).Result;
        }
    }
    else
    {
        for(i32 i = 0;
            i < PAWNS_COUNT_PER_PLAYER;
            i++)
        {
            pawn *Pawn = GetPlayersPawnForIndex(GameState, Player, i);
            if(Pawn)
            {
                Result = CanPawnKill(GameState, Pawn).Result;
                if(Result)
                {
                    break;
                }
            }
        }
    }
    return(Result);
}

struct is_move_valid_result
{
    b32 Result;
    pawn *KilledPawn;
    
    i32 ErrorID; 
    /* NOTE(maciek): 
       1 - wrong player, 
       2 - pawn not found, 
       3 - Pawn doesn't belong to player
       4 - Invalid move distance
       5 - Player has to kill
       6 - Too much pawns between start and destination
       7 - Can't kill own pawns
    */
};

internal is_move_valid_result
IsMoveValid(game_state *GameState, player *Player, v2i PositionFrom, v2i PositionTo)
{
    is_move_valid_result Result = {};
    if(GameState->CurrentPlayer == Player)
    {
        pawn *PawnFrom = GetPawnAtPosition(GameState, PositionFrom);
        if(PawnFrom)
        {
            pawn *PawnAtDestination = GetPawnAtPosition(GameState, PositionTo);
            
            Assert(PawnFrom->CurrentBoardPosition == PositionFrom);
            
            i32 YDistance = PositionFrom.y - PositionTo.y;
            i32 YDistanceAbs = ABS(YDistance);
            i32 XDistance = PositionFrom.x - PositionTo.x;
            i32 XDistanceAbs = ABS(XDistance);
            
            if(Player == PawnFrom->Owner && XDistanceAbs == YDistanceAbs && !PawnAtDestination)
            {
                if(!PawnFrom->IsQueen)
                {
                    if(YDistanceAbs == 2)
                    {
                        v2i MiddlePoint = V2i(Lerp(PositionFrom.x, 1, PositionTo.x), Lerp(PositionFrom.y, 1, PositionTo.y));
                        pawn *MiddlePointPawn = GetPawnAtPosition(GameState, MiddlePoint);
                        if(MiddlePointPawn && MiddlePointPawn->Owner != Player)
                        {
                            Result.Result = true;
                            Result.KilledPawn = MiddlePointPawn;
                        }
#if CHECKERIO_INTERNAL
                        else if(!MiddlePointPawn->Owner)
                        {
                            Result.ErrorID = 4; // NOTE(maciek): Invalid move distance
                        }
                        else
                        {
                            Result.ErrorID = 7; // NOTE(maciek): Can't kill own pawns
                        }
#endif
                    }
                    else if(YDistanceAbs == 1)
                    {
                        if(Player->Direction == YDistance && !CanPlayerKill(GameState, Player, 0))
                        {
                            Result.Result = true;
                        }
#if CHECKERIO_INTERNAL
                        else
                        {
                            Result.ErrorID = 5; // NOTE(maciek): Player has to kill
                        }
#endif
                    }
#if CHECKERIO_INTERNAL
                    else
                    {
                        Result.ErrorID = 4; // NOTE(maciek): Invalid move distance
                    }
#endif
                }
                else
                {
                    i32 PawnsBetweenCount = 0;
                    pawn *FirstPawnFound = 0;
                    for(i32 i = 1;
                        i < YDistanceAbs;
                        i++)
                    {
                        v2i CheckPosition = V2i(Lerp(PositionFrom.x, i, PositionTo.x), Lerp(PositionFrom.y, i, PositionTo.y));
                        pawn *Pawn = GetPawnAtPosition(GameState, CheckPosition);
                        if(Pawn)
                        {
                            PawnsBetweenCount++;
                            if(PawnsBetweenCount > 1)
                            {
                                break;
                            }
                            else
                            {
                                FirstPawnFound = Pawn;
                            }
                        }
                    }
                    
                    if(PawnsBetweenCount < 2)
                    {
                        if(FirstPawnFound)
                        {
                            if(FirstPawnFound->Owner != Player)
                            {
                                Result.Result = true;
                                Result.KilledPawn = FirstPawnFound;
                            }
#if CHECKERIO_INTERNAL
                            else
                            {
                                Result.ErrorID = 7; // NOTE(maciek): Can't kill own pawns
                            }
#endif
                        }
                        else if(!CanPlayerKill(GameState, Player, 0))
                        {
                            Result.Result = true;
                        }
#if CHECKERIO_INTERNAL
                        else
                        {
                            Result.ErrorID = 5; // NOTE(maciek): Player has to kill
                        }
#endif
                    }
#if CHECKERIO_INTERNAL
                    else
                    {
                        Result.ErrorID = 6; // NOTE(maciek): Too much pawns between start and destination
                    }
#endif
                }
            }
#if CHECKERIO_INTERNAL
            else
            {
                Result.ErrorID = 3; // NOTE(maciek): Pawn doesn't belong to player
            }
#endif
        }
#if CHECKERIO_INTERNAL
        else
        {
            Result.ErrorID = 2; // NOTE(maciek): Pawn not found
        }
#endif
    }
#if CHECKERIO_INTERNAL
    else
    {
        Result.ErrorID = 1; // NOTE(maciek): Wrong Player
    }
#endif
    
    return(Result);
}

struct player_make_move_result
{
    b32 IsValid;
    i32 ErrorID;
    pawn *KilledPawn;
    i32 WinnerID;
};

internal player_make_move_result
PlayerMakeMove(server_state *ServerState, game_state *GameState, player *Player, v2i PositionFrom, v2i PositionTo)
{
    player_make_move_result Result = {};
    is_move_valid_result IsMoveValidResult = IsMoveValid(GameState, Player, PositionFrom, PositionTo);
    Result.ErrorID = IsMoveValidResult.ErrorID;
    if(IsMoveValidResult.Result)
    {
        field *FieldFrom = GetBoardFieldAtPosition(GameState, PositionFrom);
        field *FieldTo = GetBoardFieldAtPosition(GameState, PositionTo);
        
        pawn *MovedPawn = FieldFrom->Pawn;
        FieldFrom->Pawn = 0;
        FieldTo->Pawn = MovedPawn;
        MovedPawn->CurrentBoardPosition = PositionTo;
        MovedPawn->AtSafePosition = IsPositionSafe(GameState, PositionTo);
        
        move *Move = PushStruct(&ServerState->WorldArena, move);
        *Move = { PositionFrom, PositionTo, 0, GameState->LastMove, Player };
        
        if(IsMoveValidResult.KilledPawn)
        {
            Result.KilledPawn = IsMoveValidResult.KilledPawn;
            KillPawn(GameState, IsMoveValidResult.KilledPawn);
            Move->KilledPawn = IsMoveValidResult.KilledPawn;
        }
        
        GameState->LastMove = Move;
        GameState->MoveChain[GameState->CurrentChainMovesCount++] = { Move->From, Move->To, Move->KilledPawn};
        
        player *OpponentPlayer = Player->PlayerNumber ? &GameState->Players[0] : &GameState->Players[1];
        if(OpponentPlayer->PawnsAliveCount)
        {
            if(!IsMoveValidResult.KilledPawn || !CanPawnKill(GameState, MovedPawn).Result)
            {
                GameState->CurrentPlayer = OpponentPlayer;
                
                if(Player->PlayerNumber == 0)
                {
                    if(PositionTo.y == GameState->BoardSize.y - 1)
                    {
                        MovedPawn->IsQueen = true;
                    }
                }
                else
                {
                    if(PositionTo.y == 0)
                    {
                        MovedPawn->IsQueen = true;
                    }
                }
            }
        }
        else
        {
            Result.WinnerID = GameState->CurrentPlayer->PlayerID;
            GameState->IsOccupied = false;
            GameState->CurrentPlayer = OpponentPlayer;
        }
        
        Result.IsValid = true;
    }
    return(Result);
}

#if CHECKERIO_INTERNAL
internal void
DrawCurrentBoardState(game_state *GameState)
{
    std::string Line = "   ";
    std::string XNumbers = "   ";
    for(i32 X = 0;
        X < GameState->BoardSize.x;
        X++)
    {
        Line += "----";
        char XLetter = (char)(X + 97);
        char XDebugNumber = (char)(X + 48);
        //XNumbers += " ";
        XNumbers += XLetter;
        XNumbers += "(";
        XNumbers += XDebugNumber;
        XNumbers += ")";
    }
    
    Line += "-";
    
    for(i32 Y = 0;
        Y < GameState->BoardSize.y;
        Y++)
    {
        char YNumber = (char)(Y + 48);
        std::string Row = " ";
        Row += YNumber;
        Row += " ";
        
        for(i32 X = 0;
            X< GameState->BoardSize.x;
            X++)
        {
            field *Field = GetBoardFieldAtPosition(GameState, V2i(X, Y));
            
            if(Field->Pawn)
            {
                if(Field->Pawn->Owner->PlayerNumber == 0)
                {
                    Row += Field->Pawn->IsQueen ? "|q0 " : "| 0 ";
                }
                else
                {
                    Row += Field->Pawn->IsQueen ? "|qX " : "| X ";
                }
            }
            else
            {
                Row += "|   ";
            }
        }
        
        Row += "|";
        
        std::cout << Line << std::endl; 
        std::cout << Row << std::endl;
    }
    std::cout << Line << std::endl;
    std::cout << XNumbers << std::endl;
}

inline player *
GetUserPlayer(game_state *GameState)
{
    //player *Result = &GameState->Players[GameState->UserPlayerNumber];
    // TODO(maciek): For debug purposes get current turn player
    player *Result = GameState->CurrentPlayer;
    return(Result);
}

internal void
ReverseLastMove(game_state *GameState)
{
    move *LastMove = GameState->LastMove;
    if(LastMove)
    {
        GameState->LastMove = LastMove->PreviousMove;
        field *FieldFrom = GetBoardFieldAtPosition(GameState, LastMove->From);
        field *FieldTo = GetBoardFieldAtPosition(GameState, LastMove->To);
        FieldFrom->Pawn = FieldTo->Pawn;
        FieldFrom->Pawn->CurrentBoardPosition = FieldFrom->BoardPosition;
        FieldTo->Pawn = 0;
        
        GameState->CurrentPlayer = LastMove->PreviousPlayer;
        
        if(LastMove->KilledPawn)
        {
            field *KilledPawnField = GetBoardFieldAtPosition(GameState, LastMove->KilledPawn->CurrentBoardPosition);
            KilledPawnField->Pawn = LastMove->KilledPawn;
            LastMove->KilledPawn->IsAlive = true;
            LastMove->KilledPawn->Owner->PawnsAliveCount++;
        }
    }
}

struct user_make_move_result
{
    move Move;
    
    // NOTE(maciek): 0 - InvalidMove, 1 - ValidMove, -1 - ReverseLastMove, 2 - Skip
    b32 IsMoveValid;
    i32 ErrorID;
    b32 KilledPawn;
    v2i KilledPawnPosition;
    i32 WinnerID;
};

struct user_move_data
{
    char *Data;
    i32 DataSize;
    i32 UserNumber;
};

internal user_make_move_result
UserMakeMove(server_state *ServerState, game_state *GameState, user_move_data MoveData)
{
    user_make_move_result Result = {};
    
#if 0
    std::cout << "Make move:" << std::endl;
    std::cout << "> ";
    
    char PlayerInput[1024] = {};
    std::cin.getline(PlayerInput, sizeof(PlayerInput));
    char *c = &PlayerInput[0];
#endif
    char *c = MoveData.Data;
    if(*c == 'r')
    {
        ReverseLastMove(GameState);
        Result.IsMoveValid = -1;
    }
#if 0
    else if(*c == 's')
    {
        std::string FileName = "";
        char *C = c + 2;
        while(*C != ' ' && *C != '\0')
        {
            FileName += *C++;
        }
        
        if(FileName.length() == 0)
        {
            FileName = "GameState";
        }
        
        FileName += ".cio";
        
        SaveCurrentMemoryState(Memory, (char *)FileName.c_str());
        Result.IsMoveValid = 2;
    }
    else if(*c == 'l')
    {
        std::string FileName = "";
        char *C = c + 2;
        while(*C != ' ' && *C != '\0')
        {
            FileName += *C++;
        }
        
        if(FileName.length() == 0)
        {
            FileName = "GameState";
        }
        
        FileName += ".cio";
        
        GameState->DEBUGLoadFileName = FileName;
        Memory->Reload = true;
        Result.IsMoveValid = 2;
    }
#endif
    else
    {
        i32 FromX = (i32)(*c++ - 97);
        i32 FromY = (i32)(*c++ - 48);
        *c++;
        i32 ToX = (i32)(*c++ - 97);
        i32 ToY = (i32)(*c - 48);
        
        v2i From = V2i(FromX, FromY);
        v2i To = V2i(ToX, ToY);
        
        if(IsPositionValid(GameState, From) && IsPositionValid(GameState, To))
        {
            //is_move_valid_result IsMoveValidResult = IsMoveValid(GameState, &GameState->Players[MoveData.UserNumber], From, To);
            player_make_move_result MoveResult = PlayerMakeMove(ServerState, GameState, &GameState->Players[MoveData.UserNumber], From, To);
            
            if(MoveResult.IsValid)
            {
                move Move = { From, To, MoveResult.KilledPawn };
                Result.IsMoveValid = 1;
                Result.Move = Move;
            }
            else
            {
                Result.IsMoveValid = false;
            }
            
            if(MoveResult.KilledPawn)
            {
                Result.KilledPawn = true;
                Result.KilledPawnPosition = MoveResult.KilledPawn->CurrentBoardPosition;
            }
            
            Result.WinnerID = MoveResult.WinnerID;
            Result.ErrorID = MoveResult.ErrorID;
        }
    }
    return(Result);
}
#endif

internal i32 
FindFirstFreeTable(server_state *ServerState)
{
    i32 Result = -1;
    
    for(i32 BoardCounter = 0;
        BoardCounter < MAX_CLIENTS_COUNT;
        BoardCounter++)
    {
        game_state *Board = ServerState->GameStates + BoardCounter;
        if(!Board->IsOccupied)
        {
            Result = BoardCounter;
            break;
        }
    }
    
    return(Result);
}

internal game_state * 
CreateNewGameTable(server_state *ServerState, i32 FirstPlayerID, i32 SecondPlayerID, i32 TableIndex)
{
    // NOTE(maciek): Assumes that table at TableIndex is free or outdated
    game_state *GameState = ServerState->GameStates + TableIndex;
    GameState->IsOccupied = true;
    
    i32 PawnRows = 3;
    
    i32 BoardSideSize = 8;
    GameState->BoardSize = V2i(BoardSideSize, BoardSideSize);
    //GameState->FieldsCount = (i32)(GameState->BoardSize.x*GameState->BoardSize.y);
    //PushArray(&ServerState->WorldArena, GameState->FieldsCount, field);
    GameState->LastMove = 0;
    
    //GameState->PlayersPawnCount = (i32)(GameState->BoardSize.x*PawnRows/2);
    GameState->Players[0] = { 0, -1, {}, PAWNS_COUNT_PER_PLAYER, FirstPlayerID };
    GameState->Players[1] = { 1, 1, {}, PAWNS_COUNT_PER_PLAYER, SecondPlayerID };
    i32 Player1PawnCounter = 0;
    i32 Player2PawnCounter = 0;
    
    for(i32 Y = 0;
        Y < GameState->BoardSize.y;
        Y++)
    {
        for(i32 X = 0;
            X < GameState->BoardSize.x;
            X++)
        {
            field *CurrentField = GetBoardFieldAtPosition(GameState, V2i(X, Y));                
            CurrentField->BoardPosition = V2i(X, Y);
            
            b32 IsSpecial = (X+Y)%2;
            pawn *Pawn = 0;
            if(IsSpecial)
            {
                if(Y < PawnRows)
                {
                    Pawn = GameState->Players[0].Pawns + Player1PawnCounter++;
                    Pawn->Owner = &GameState->Players[0];
                }
                else if(Y > GameState->BoardSize.y - PawnRows - 1)
                {
                    Pawn = GameState->Players[1].Pawns + Player2PawnCounter++;
                    Pawn->Owner = &GameState->Players[1];
                }
            }
            
            if(Pawn)
            {
                Pawn->IsQueen = false;
                Pawn->IsAlive = true;
                Pawn->CurrentBoardPosition = V2i(X, Y);
                //Pawn->AtSafePosition = IsPositionSafe(GameState, CurrentField->BoardPosition);
            }
            
            CurrentField->Pawn = Pawn;
        }
    }
    
    GameState->CurrentPlayer = &GameState->Players[0];
    return(GameState);
    //DrawCurrentBoardState(GameState);
}

struct request_make_move_result
{
    b32 Result;
    user_make_move_result MoveResult;
    i32 CurrentPlayerID;
    
#if CHECKERIO_INTERNAL
    game_state *GameState;
#endif
};

internal request_make_move_result
RequestMakeMove(server_state *ServerState, transient_state *TranState, request_move_data *MoveData)
{
    i32 GameID = -1;
    i32 GameUserNumber = -1;
    game_state *GameState = 0;
    for(i32 GameCounter = 0;
        GameCounter < MAX_CLIENTS_COUNT;
        GameCounter++)
    {
        GameState = ServerState->GameStates + GameCounter;
        if(GameState->Players[0].PlayerID == MoveData->PlayerID)
        {
            GameID = GameCounter;
            GameUserNumber = 0;
            break;
        }
        else if(GameState->Players[1].PlayerID == MoveData->PlayerID)
        {
            GameID = GameCounter;
            GameUserNumber = 1;
            break;
        }
    }
    
    request_make_move_result Result = { MOVE_INVALID, -1, 0 };
    if(GameID != -1 && GameUserNumber != -1 && GameState)
    {
        user_move_data UserMoveData = {};
        UserMoveData.DataSize = ArrayCount(MoveData->From) + ArrayCount(MoveData->To) + 1;
        UserMoveData.Data = (char *)PushSize(&TranState->TranArena, UserMoveData.DataSize);
        UserMoveData.UserNumber = GameUserNumber;
        char *C = UserMoveData.Data;
        char *FromPtr = MoveData->From;
        
        for(i32 FromCounter = 0;
            FromCounter < ArrayCount(MoveData->From);
            FromCounter++)
        {
            
            *C++ = *FromPtr++;
        }
        *C++ = ' ';
        
        char *ToPtr = MoveData->To;
        for(i32 ToCounter = 0;
            ToCounter < ArrayCount(MoveData->To);
            ToCounter++)
        {
            *C++ = *ToPtr++;
        }
        
        user_make_move_result MoveResult = UserMakeMove(ServerState, ServerState->GameStates + GameID, UserMoveData);
        Result.Result = MoveResult.IsMoveValid ? MOVE_OK : MOVE_INVALID;
        Result.CurrentPlayerID = GameState->CurrentPlayer->PlayerID;
        Result.MoveResult = MoveResult;
        
#if CHECKERIO_INTERNAL
        Result.GameState = ServerState->GameStates + GameID;
#endif
    }
    else
    {
        Assert("Invalid game id or user number");
    }
    
    return(Result);
}

internal inline i32 
GetNewPlayerID(server_state *ServerState)
{
    i32 Result = ++ServerState->LastPlayerID;
    return(Result);
}

internal b32
AddPlayerToQueue(server_state *ServerState, player_queue_entry QueueEntry)
{
    b32 Result = false;
    if(ServerState->PlayersQueue.CurrentCount <= MAX_PLAYER_QUEUE_ENTRIES)
    {
        i32 QueueIndex = ServerState->PlayersQueue.CurrentFirstInQueue + ServerState->PlayersQueue.CurrentCount;
        if(QueueIndex > MAX_PLAYER_QUEUE_ENTRIES)
        {
            QueueIndex = QueueIndex - MAX_PLAYER_QUEUE_ENTRIES;
        }
        
        player_queue_entry *EntryPtr = ServerState->PlayersQueue.Entries + QueueIndex;
        *EntryPtr = QueueEntry;
        ServerState->PlayersQueue.CurrentCount++;
        
        Result = true;
    }
    return(Result);
}

struct find_new_game_result
{
    i32 Response; // NOTE(maciek): 0 - Joined the queue, 1 - Found new game, -1 - Queue overflow
    i32 GameIndex;
    
    player_queue_entry PlayerFirstEntry;
    i32 PlayerFirstMoveDirection;
    player_queue_entry PlayerSecondEntry;
    i32 PlayerSecondMoveDirection;
    
    i32 CurrentPlayerID;
};

internal find_new_game_result
RequestFindNewGame(server_state *ServerState, i32 PlayerID)
{
    find_new_game_result Result = {};
    
    b32 AddToQueue = true;
    if(ServerState->PlayersQueue.CurrentCount)
    {
        Result.GameIndex = FindFirstFreeTable(ServerState);
        if(Result.GameIndex != -1)
        {
            AddToQueue = false;
        }
    }
    
    if(!AddToQueue)
    {
        // NOTE(maciek): Can create new game
        player_queue_entry *FirstPlayerInQueue = ServerState->PlayersQueue.Entries + ServerState->PlayersQueue.CurrentFirstInQueue;
        
        Result.PlayerFirstEntry = *FirstPlayerInQueue;
        Result.PlayerSecondEntry = { PlayerID };
        
        game_state *GameState = CreateNewGameTable(ServerState, Result.PlayerFirstEntry.PlayerID, 
                                                   Result.PlayerSecondEntry.PlayerID, Result.GameIndex);
        
        Result.CurrentPlayerID = GameState->CurrentPlayer->PlayerID;
        Result.PlayerFirstMoveDirection = GameState->Players[0].Direction;
        Result.PlayerFirstMoveDirection = GameState->Players[1].Direction;
        
        // NOTE(maciek): Move the queue
        ServerState->PlayersQueue.CurrentCount--;
        if(ServerState->PlayersQueue.CurrentCount)
        {
            i32 CurrentInQueue = ServerState->PlayersQueue.CurrentFirstInQueue;
            if(CurrentInQueue + 1 > MAX_PLAYER_QUEUE_ENTRIES)
            {
                ServerState->PlayersQueue.CurrentFirstInQueue = 0;
            }
            else
            {
                ServerState->PlayersQueue.CurrentFirstInQueue++;
            }
        }
        
        Result.Response = 1;
    }
    else
    {
        // NOTE(maciek): Can't create new game. Join to the queue
        
        Result.Response = AddPlayerToQueue(ServerState, { PlayerID }) ? 0 : -1;
    }
    
    return(Result);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{    
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);    
    
    server_state *ServerState = (server_state *)Memory->PermanentStorage;
    //i64 CurrentTime = Memory->CurrentTime;
    if(!Memory->IsInitialized)
    {
        // NOTE(maciek): INIT
        Memory->IsInitialized = true;
        // TODO(maciek): Move to 64-bit eventually
        InitializeArena(&ServerState->WorldArena, (i32)Memory->PermanentStorageSize - MAX_CLIENTS_COUNT*sizeof(game_state),
                        (u8 *)Memory->PermanentStorage + MAX_CLIENTS_COUNT*sizeof(game_state));
        
        player_queue_entry DummyQueueEntry = { 123456 };
        //AddPlayerToQueue(ServerState, &DummyQueueEntry);
    }
    
    // NOTE(Maciek): Transient initialization
    Assert(sizeof(transient_state) <= Memory->TransientStorageSize);    
    transient_state *TranState = (transient_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        // TODO(maciek): Move to 64-bit eventually
        InitializeArena(&TranState->TranArena, (i32)Memory->TransientStorageSize - sizeof(transient_state),
                        (u8 *)Memory->TransientStorage + sizeof(transient_state));
        
        TranState->IsInitialized = true;
        
        char *Test1 = Int32ToString(&TranState->TranArena, 1235324);
        char *Test2 = Int32ToString(&TranState->TranArena, 2);
        char *Test3 = Int32ToString(&TranState->TranArena, 1230);
    }
    
#if 0 //CHECKERIO_INTERNAL
    b32 UserMoveIsValid = false;
    
    system("cls");
    DrawCurrentBoardState(GameState);
    Memory->Reload = false;
    
    do
    {
        user_make_move_result UserMoveResult = UserMakeMove(GameState, Memory);
        
        if(UserMoveResult.IsMoveValid == 1)
        {
            UserMoveIsValid = true;
            PlayerMakeMove(GameState, GetUserPlayer(GameState), UserMoveResult.Move.From, UserMoveResult.Move.To); 
        }
        else if(UserMoveResult.IsMoveValid == -1)
        {
            UserMoveIsValid = true;
        }
        else if(UserMoveResult.IsMoveValid == 2)
        {
            break;
        }
        else
        {
            std::cout << "Invalid move" << std::endl;
        }
    }
    while(!UserMoveIsValid);
    
    if(Memory->Reload)
    {
        debug_read_file_result ReadResult = 
            Memory->DEBUGPlatformReadEntireFile(0, (char *)GameState->DEBUGLoadFileName.c_str());
        std::memcpy(Memory->PermanentStorage, ReadResult.Contents, ReadResult.ContentsSize);
    }
#endif
    
    // NOTE(maciek): Handle request message
    request_type RequestType = REQUEST_INVALID;
    request_move_data *RequestMoveData = 0;
    request_other_player_finished_data *RequestOtherPlayerFinished = 0;
    i32 RequestFoundNewGamePlayerID = -1;
    
    if(StrContains(Request.Data, "move"))
    {
        RequestType = REQUEST_MOVE;
        
        char *From = StrFindEndOf(Request.Data, "from=");
        char *To = StrFindEndOf(Request.Data, "to=");
        char *PlayerID = StrFindEndOf(Request.Data, "id=");
        
        if(From && To && PlayerID)
        {
            RequestMoveData = (request_move_data *)PushStruct(&TranState->TranArena, request_move_data);
            
            StrCpy(RequestMoveData->From, From, ArrayCount(RequestMoveData->From));
            StrCpy(RequestMoveData->To, To, ArrayCount(RequestMoveData->To));
            
            i32 SeparatorIndex = StrFindCharIndex(PlayerID, '&');
            RequestMoveData->PlayerID = StringToInt32(PlayerID, SeparatorIndex);
        }
        
    }
    else if(StrContains(Request.Data, "find_new_game"))
    {
        RequestType = REQUEST_FIND_NEW_GAME;
    }
    else if(StrContains(Request.Data, "found_new_game"))
    {
        RequestType = REQUEST_FOUND_NEW_GAME;
        char *PlayerID = StrFindEndOf(Request.Data, "player_id=");
        i32 MessageEnd = StrFindCharIndex(PlayerID, '$');
        RequestFoundNewGamePlayerID = StringToInt32(PlayerID, MessageEnd);
    }
    else if(StrContains(Request.Data, "other_player_finished"))
    {
        RequestType = REQUEST_OTHER_PLAYER_FINISHED;
        RequestOtherPlayerFinished = PushStruct(&TranState->TranArena, request_other_player_finished_data);
        
        char *PlayerID = StrFindEndOf(Request.Data, "player_id=");
        char *GameID = StrFindEndOf(Request.Data, "game_id=");
        char *PlayerIDSeparator = StrFindEndOf(PlayerID, "&&");
        i32 MessageEnd = StrFindCharIndex(GameID, '$');
        
        RequestOtherPlayerFinished->PlayerID = StringToInt32(PlayerID, PlayerIDSeparator - PlayerID - 2);
        RequestOtherPlayerFinished->GameID = StringToInt32(GameID, MessageEnd);
    }
    
    switch(RequestType)
    {
        case REQUEST_OTHER_PLAYER_FINISHED:
        {
            if(RequestOtherPlayerFinished->GameID >= 0 && RequestOtherPlayerFinished->GameID < MAX_CLIENTS_COUNT)
            {
                game_state *GameState = ServerState->GameStates + RequestOtherPlayerFinished->GameID;
                if(GameState->CurrentPlayer->PlayerID == RequestOtherPlayerFinished->PlayerID)
                {
                    if(GameState->CurrentChainMovesCount > 0)
                    {
                        char *ChainMovesStr = 0;
                        ChainMovesStr = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_PLAYER_FINISHED);
                        for(i32 ChainMovesIterator = 0;
                            ChainMovesIterator < GameState->CurrentChainMovesCount;
                            ChainMovesIterator++)
                        {
                            move_chain_link *ChainLink = GameState->MoveChain + ChainMovesIterator;
                            
                            char From[] = 
                            { (char)(ChainLink->From.x + 97), (char)(ChainLink->From.y + 48), char(0) };
                            char *FromStr = ConcatStrings(&TranState->TranArena, "&&from=", From);
                            
                            char To[] = 
                            { (char)(ChainLink->To.x + 97), (char)(ChainLink->To.y + 48), char(0) };
                            char *ToStr = ConcatStrings(&TranState->TranArena, "&&to=", To);
                            
                            char *ChainMoveStr = ConcatStrings(&TranState->TranArena, FromStr, ToStr);
                            
                            if(ChainLink->KilledPawn)
                            {
                                char KilledPawn[] = 
                                { (char)(ChainLink->KilledPawn->CurrentBoardPosition.x + 97), 
                                    (char)(ChainLink->KilledPawn->CurrentBoardPosition.y + 48), char(0) };
                                char *KilledPawnStr = ConcatStrings(&TranState->TranArena, "&&killed_pawn=", KilledPawn);
                                
                                ChainMoveStr = ConcatStrings(&TranState->TranArena, ChainMoveStr, KilledPawnStr);
                            }
                            
                            ChainMovesStr = ConcatStrings(&TranState->TranArena, ChainMovesStr, ChainMoveStr);
                        }
                        
                        Response->Data = ChainMovesStr;
                        
                        char *CurrentPlayerID = Int32ToString(&TranState->TranArena, GameState->CurrentPlayer->PlayerID);
                        CurrentPlayerID = ConcatStrings(&TranState->TranArena, "&&current_player=", CurrentPlayerID);
                        Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, CurrentPlayerID);
                        
                        if(!GameState->CurrentPlayer->PawnsAliveCount)
                        {
                            i32 OtherPlayerID = GameState->CurrentPlayer->PlayerID == GameState->Players[0].PlayerID ? GameState->Players[1].PlayerID : GameState->Players[0].PlayerID;
                            char *WinnerID = Int32ToString(&TranState->TranArena, OtherPlayerID);
                            WinnerID = ConcatStrings(&TranState->TranArena, "&&winner=", WinnerID);
                            Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, WinnerID);
                        }
                        
                        GameState->CurrentChainMovesCount = 0;
                    }
                    else
                    {
                        Response->Data = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_SERVER_ERROR);
                    }
                }
                else
                {
                    Response->Data = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_PLAYER_NOT_FINISHED);
                }
            }
            else
            {
                Response->Data = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_SERVER_ERROR);
            }
            
            Response->Size = StrLen(Response->Data, 2048);
        }
        break;
        
        case REQUEST_MOVE:
        {
            request_make_move_result RequestResult = RequestMakeMove(ServerState, TranState, RequestMoveData);
            
            char *ResponseString = CHECKERIO_PREFIX;
            switch(RequestResult.Result)
            {
                case MOVE_OK:
                {
                    ResponseString = ConcatStrings(&TranState->TranArena, ResponseString, STR_RESPONSE_MOVE_OK);
                    
                    char *CurrentPlayerID = Int32ToString(&TranState->TranArena, RequestResult.CurrentPlayerID);
                    CurrentPlayerID = ConcatStrings(&TranState->TranArena, "&&current_player_id=", CurrentPlayerID);
                    
                    ResponseString = ConcatStrings(&TranState->TranArena, ResponseString, CurrentPlayerID);
                    
                    if(RequestResult.MoveResult.KilledPawn)
                    {
                        v2i PawnPosition = RequestResult.MoveResult.KilledPawnPosition;
                        char KilledPawnPosition[3] = 
                        { (char)(PawnPosition.x + 97), (char)(PawnPosition.y + 48), char(0) };
                        
                        char *PawnPositionStr = ConcatStrings(&TranState->TranArena, "&&killed_pawn=", KilledPawnPosition);
                        
                        ResponseString = ConcatStrings(&TranState->TranArena, ResponseString, PawnPositionStr);
                        
                        if(RequestResult.MoveResult.WinnerID)
                        {
                            char *WinnerID = Int32ToString(&TranState->TranArena, RequestResult.MoveResult.WinnerID);
                            WinnerID = ConcatStrings(&TranState->TranArena, "&&winner=", WinnerID);
                            ResponseString = ConcatStrings(&TranState->TranArena, ResponseString, WinnerID);
                        }
                    }
                    
                    DrawCurrentBoardState(RequestResult.GameState);
                }
                break;
                
                case MOVE_INVALID:
                {
                    ResponseString = ConcatStrings(&TranState->TranArena, ResponseString, STR_RESPONSE_MOVE_INVALID);
                }
                break;
                
                case GAME_NO_LOGNER_EXISTS:
                {
                    ResponseString = ConcatStrings(&TranState->TranArena, ResponseString, 
                                                   STR_RESPONSE_GAME_NO_LONGER_EXISTS);
                }
                break;
            }
            
            Response->Data = ResponseString;
            
            if(RequestResult.Result != MOVE_OK)
            {
                char *ErrorCode = Int32ToString(&TranState->TranArena, RequestResult.MoveResult.ErrorID);
                ErrorCode = ConcatStrings(&TranState->TranArena, "&&error_id=", ErrorCode);
                Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, ErrorCode);
                /* NOTE(maciek): 
                1 - wrong player, 
                2 - pawn not found, 
                3 - Pawn doesn't belong to player
                4 - Invalid move distance
                5 - Player has to kill
                6 - Too much pawns between start and destination
                7 - Can't kill own pawns
                */
            }
            
            Response->Size = StrLen(Response->Data);
        }
        break;
        
        case REQUEST_FIND_NEW_GAME:
        {
            i32 PlayerID = GetNewPlayerID(ServerState);
            find_new_game_result FindNewGameResult = RequestFindNewGame(ServerState, PlayerID);
            
            switch(FindNewGameResult.Response)
            {
                case 0: // NOTE(maciek): Joined the queue
                {
                    Response->Type = RESPONSE_JOINED_QUEUE;
                    
                    Response->Data = 
                        ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_JOINED_QUEUE);
                    
                    char *NumberInQueue = Int32ToString(&TranState->TranArena, ServerState->PlayersQueue.CurrentCount);
                    NumberInQueue = ConcatStrings(&TranState->TranArena, "&&number_in_queue=", NumberInQueue);
                    Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, NumberInQueue);
                    
                    char *PlayerIDStr = Int32ToString(&TranState->TranArena, PlayerID);
                    PlayerIDStr = ConcatStrings(&TranState->TranArena, "&&player_id=", PlayerIDStr);
                    Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, PlayerIDStr);
                    
                    Response->Size = StrLen(Response->Data);
                }
                break;
                
                case 1: // NOTE(maciek): Found new game
                {
                    Response->Type = RESPONSE_FOUND_NEW_GAME;
                    new_game_info *GameInfo = (new_game_info *)PushStruct(&TranState->TranArena, new_game_info);
                    GameInfo->PlayerID = FindNewGameResult.PlayerSecondEntry.PlayerID;
                    
                    char *PlayerID = Int32ToString(&TranState->TranArena, GameInfo->PlayerID);
                    char *Prefix = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, 
                                                 STR_RESPONSE_FOUND_NEW_GAME); 
                    Response->Data = ConcatStrings(&TranState->TranArena, Prefix, PlayerID);
#if 1
                    // NOTE(maciek): Other player's id for debug
                    char *DebugID = Int32ToString(&TranState->TranArena, FindNewGameResult.PlayerFirstEntry.PlayerID);
                    DebugID = ConcatStrings(&TranState->TranArena, "&&dbgID=", DebugID);
                    Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, DebugID);
#endif
                    char *CurrentPlayer = Int32ToString(&TranState->TranArena, FindNewGameResult.CurrentPlayerID);
                    CurrentPlayer = ConcatStrings(&TranState->TranArena, "&&current_player=", CurrentPlayer);
                    Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, CurrentPlayer);
                    
                    i32 MoveDirection;
                    if(FindNewGameResult.PlayerFirstEntry.PlayerID == FindNewGameResult.CurrentPlayerID)
                    {
                        MoveDirection = FindNewGameResult.PlayerFirstMoveDirection;
                    }
                    else
                    {
                        MoveDirection = FindNewGameResult.PlayerSecondMoveDirection;
                    }
                    
                    char *MoveDirectionStr = Int32ToString(&TranState->TranArena, MoveDirection);
                    MoveDirectionStr = ConcatStrings(&TranState->TranArena, "&&move_direction=", MoveDirectionStr);
                    Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, MoveDirectionStr);
                    
                    char *GameID = Int32ToString(&TranState->TranArena, FindNewGameResult.GameIndex);
                    GameID = ConcatStrings(&TranState->TranArena, "&&game_id=", GameID);
                    Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, GameID);
                    
                    Response->Size = StrLen(Response->Data, 2048);
                }
                break;
                
                case -1: // NOTE(maciek): Queue overflow
                {
                    Response->Type = RESPONSE_SERVER_ERROR;
                }
                break;
            }
        }
        break;
        
        case REQUEST_FOUND_NEW_GAME:
        {
            if(RequestFoundNewGamePlayerID != -1)
            {
                b32 Found = false;
                for(i32 GamesIterator = 0;
                    GamesIterator < MAX_CLIENTS_COUNT;
                    GamesIterator++)
                {
                    game_state *GameState = ServerState->GameStates + GamesIterator;
                    if(GameState->IsOccupied && 
                       (GameState->Players[0].PlayerID == RequestFoundNewGamePlayerID ||
                        GameState->Players[1].PlayerID == RequestFoundNewGamePlayerID))
                    {
                        Response->Type = RESPONSE_FOUND_NEW_GAME;
                        new_game_info *GameInfo = (new_game_info *)PushStruct(&TranState->TranArena, new_game_info);
                        i32 PlayerNumber = 0;
                        i32 MoveDirection;
                        if(GameState->Players[1].PlayerID == RequestFoundNewGamePlayerID)
                        {
                            PlayerNumber = 1;
                        }
                        
                        GameInfo->PlayerID = GameState->Players[PlayerNumber].PlayerID;
                        MoveDirection = GameState->Players[PlayerNumber].Direction;
                        
                        char *PlayerID = Int32ToString(&TranState->TranArena, GameInfo->PlayerID);
                        char *Prefix = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, 
                                                     STR_RESPONSE_FOUND_NEW_GAME); 
                        Response->Data = ConcatStrings(&TranState->TranArena, Prefix, PlayerID);
                        
                        char *CurrentPlayer = Int32ToString(&TranState->TranArena, GameState->CurrentPlayer->PlayerID);
                        CurrentPlayer = ConcatStrings(&TranState->TranArena, "&&current_player=", CurrentPlayer);
                        Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, CurrentPlayer);
                        
                        char *MoveDirectionStr = Int32ToString(&TranState->TranArena, MoveDirection);
                        MoveDirectionStr = ConcatStrings(&TranState->TranArena, "&&move_direction=", MoveDirectionStr);
                        Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, MoveDirectionStr);
                        
                        char *GameID = Int32ToString(&TranState->TranArena, GamesIterator);
                        GameID = ConcatStrings(&TranState->TranArena, "&&game_id=", GameID);
                        Response->Data = ConcatStrings(&TranState->TranArena, Response->Data, GameID);
                        
                        Response->Size = StrLen(Response->Data, 2048);
                        
                        Found = true;
                        
                        
#if CHECKERIO_INTERNAL
                        DrawCurrentBoardState(GameState);
#endif
                        
                        
                        break;
                    }
                }
                
                if(!Found)
                {
                    Response->Data = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_GAME_NOT_YET_FOUND);
                }
            }
            else
            {
                Response->Data = ConcatStrings(&TranState->TranArena, CHECKERIO_PREFIX, STR_RESPONSE_SERVER_ERROR);
            }
            
            Response->Size = StrLen(Response->Data);
        }
        break;
        
        default:
        {
            Response->Type = RESPONSE_SERVER_ERROR;
        }
        break;
    }
}