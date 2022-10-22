typedef struct app_state
{
    memory_arena Arena;
    random_state RandomState;
} app_state;

extern void
AppHandleCommand(app_memory *Memory, string Command, u32 ArgCount, string *Args)
{
}

internal void
AppInit(app_memory *Memory)
{
    app_state *AppState = Memory->AppState;
    AppState->RandomState.Value = 65535;
}

internal void
AppTick(app_memory *Memory, f32 dtForFrame)
{
    app_state *AppState = Memory->AppState;
    random_state *RandomState = &AppState->RandomState;
    u32 SpamCount = RandomU32(RandomState) % 10000;
    u16 ThreadId = GetThreadID();
    
    platform_http_client Client = Platform.BeginHttpClient(String("localhost"), 8090);
    for(u32 SpamIndex = 0;
        SpamIndex < SpamCount;
        ++SpamIndex)
    {
        temporary_memory MemoryFlush = BeginTemporaryMemory(&AppState->Arena);
        
        platform_http_request Request = Platform.BeginHttpRequest(&Client, HttpVerb_Get, "/echo");
        Request.Payload = FormatString(MemoryFlush.Arena, "ThreadId: %u Message: %u Random: %u", ThreadId, SpamIndex, RandomU32(RandomState));
        platform_http_response Response = Platform.GetHttpResonse(&Request);
        
        if(!StringsAreEqual(Request.Payload, Response.Payload))
        {
            LogError("'%S' does not match '%S'", Request, Response);
        }
        
        Platform.EndHttpRequest(&Request);
        
        EndTemporaryMemory(MemoryFlush);
    }
    Platform.EndHttpClient(&Client);
    
    LogVerbose("Sent %u requests", SpamCount);
}
