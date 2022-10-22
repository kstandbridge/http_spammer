typedef struct spam_work
{
    memory_arena Arena;
    random_state *RandomState;
} spam_work;

typedef struct app_state
{
    memory_arena Arena;
    random_state RandomState;
    
    platform_work_queue *WorkerQueue;
    spam_work *SpamWorks[16];
    
    
} app_state;

extern void
AppHandleCommand(app_memory *Memory, string Command, u32 ArgCount, string *Args)
{
}

internal void
SpamThread(spam_work *SpamWork)
{
    random_state *RandomState = SpamWork->RandomState;
    u32 SpamCount = RandomU32(RandomState) % 1000;
    
    u16 ThreadId = GetThreadID();
    
    platform_http_client Client = Platform.BeginHttpClient(String("localhost"), 8090);
    for(u32 SpamIndex = 0;
        SpamIndex < SpamCount;
        ++SpamIndex)
    {
        temporary_memory MemoryFlush = BeginTemporaryMemory(&SpamWork->Arena);
        
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

internal void
AppInit(app_memory *Memory)
{
    app_state *AppState = Memory->AppState;
    AppState->RandomState.Value = 65535;
    AppState->WorkerQueue = Platform.MakeWorkQueue(&AppState->Arena, 8);
    
    for(u32 WorkerIndex = 0;
        WorkerIndex < ArrayCount(AppState->SpamWorks);
        ++WorkerIndex)
    {
        spam_work *Worker = BootstrapPushStruct(spam_work, Arena);
        Worker->RandomState = &AppState->RandomState;
        AppState->SpamWorks[WorkerIndex] = Worker;
        
        Platform.AddWorkEntry(AppState->WorkerQueue, SpamThread, Worker);
    }
}

internal void
AppTick(app_memory *Memory, f32 dtForFrame)
{
    app_state *AppState = Memory->AppState;
    Platform.CompleteAllWork(AppState->WorkerQueue);
    LogVerbose("All spam threads complete");
    Memory->IsRunning = false;
}
