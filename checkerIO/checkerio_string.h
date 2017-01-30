#ifndef CHECKERIO_STRING
#define CHECKERIO_STRING

internal b32 
StrContains(char *Str, char *Data, i32 MaxCharsToCheck = 128)
{
    b32 Result = false;
    
    char *StrPtr = Str;
    i32 Iterator = 0;
    
    char *DataPtr = Data;
    while(*Str && Iterator++ < MaxCharsToCheck && *DataPtr)
    {
        if(*DataPtr == *Str)
        {
            *DataPtr++;
        }
        else
        {
            DataPtr = Data;
        }
        *Str++;
    }
    
    if(!*DataPtr)
    {
        Result = true;
    }
    
    return(Result);
}

internal char *
StrFindEndOf(char *Str, char *Data, i32 MaxCharsToCheck = 128)
{
    char *Result = 0;
    
    char *StrPtr = Str;
    i32 Iterator = 0;
    char *DataPtr = Data;
    
    while(*Str && Iterator++ < MaxCharsToCheck && *DataPtr)
    {
        if(*DataPtr == *Str)
        {
            *DataPtr++;
        }
        else
        {
            DataPtr = Data;
        }
        *Str++;
    }
    
    if(!*DataPtr)
    {
        Result = Str;
    }
    
    return(Result);
}

internal void
StrCpy(char *Destination, char *Source, i32 Length)
{
    for(i32 LengthIterator = 0;
        LengthIterator < Length;
        LengthIterator++)
    {
        *Destination++ = *Source++;
    }
}

internal i32
StringToInt32(char *String, i32 MaxChars = 128)
{
    i32 Result = 0;
    char *Str = String;
    i32 Iterator = 0;
    b32 IsNegative = false;
    if(*Str == '-')
    {
        Str++;
        IsNegative = true;
        Iterator++;
    }
    
    while(*Str && Iterator++ < MaxChars)
    {
        Result *= 10;
        Result += *Str - 48;
        Str++;
    }
    
    if(IsNegative)
    {
        Result *= -1;
    }
    
    return(Result);
}

internal char *
Int32ToString(memory_arena *Arena, i32 Value, i32 MaxCharacters = 128)
{
    char *Result = (char *)PushSize(Arena, MaxCharacters);
    char *Temp = (char *)PushSize(Arena, MaxCharacters);
    i32 Digits = 0;
    b32 IsNegative = Value < 0 ? true : false;
    if(IsNegative)
    {
        Value *= -1;
        Digits++;
    }
    
    if(!Value)
    {
        *Result = '0';
    }
    else
    {
        char *C = Temp;
        while(Value)
        {
            i32 TempValue = Value % 10;
            Value /= 10;
            *C++ = (char)(TempValue + 48);
            Digits++;
        }
        
        char *ResultC = Result + Digits;
        *ResultC-- = 0;
        for(i32 CharIterator = 0;
            CharIterator < Digits - IsNegative;
            CharIterator++)
        {
            *ResultC-- = *Temp++;
        }
        if(IsNegative)
        {
            *ResultC = '-';
        }
    }
    
    return(Result);
}

internal i32
StrLen(char *Str, i32 MaxSize = 1024)
{
    char *C = Str;
    i32 Iterator = 0;
    while(*C++ && Iterator++ < MaxSize);
    return(Iterator);
}

internal char *
ConcatStrings(memory_arena *MemoryArena, char *Lhs, char *Rhs)
{
    i32 LhsLen = StrLen(Lhs);
    i32 RhsLen = StrLen(Rhs);
    char *Result = (char *)PushSize(MemoryArena, LhsLen + RhsLen + 1);
    char *C = Result;
    
    char *CopyC = Lhs;
    for(i32 CopyIterator = 0;
        CopyIterator < LhsLen;
        CopyIterator++)
    {
        *C++ = *CopyC++;
    }
    
    CopyC = Rhs;
    for(i32 CopyIterator = 0;
        CopyIterator < RhsLen;
        CopyIterator++)
    {
        *C++ = *CopyC++;
    }
    *C = 0;
    
    return(Result);
}

internal i32
StrFindCharIndex(char *String, char Separator)
{
    i32 Result = 0;
    char *C = String;
    while(*C)
    {
        if(*C++ != Separator)
        {
            Result++;
        }
        else
        {
            break;
        }
    }
    return(Result);
}

#endif