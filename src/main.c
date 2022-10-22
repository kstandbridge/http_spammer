typedef struct spam_work
{
    memory_arena Arena;
    random_state *RandomState;
} spam_work;

// NOTE(kstandbridge): This is a mandatory struct you must define, it needs to include Arena
// which will be initialized by the frame work. Place anything in here you want to keep for
// the scope of the application
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
    // NOTE(kstandbridge): This is called once at startup for any command line arguments passed.
    
    // NOTE(kstandbridge): Additionally while not used in this example if you called InitConsoleCommandLoop
    // then this will  be called again each time a command is entered into the console.
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
        // NOTE(kstandbridge): Temporary memory allows you to reuse the same chunk
        // When calling EndTemporaryMemory it just moves the pointer back to the
        // origional place in the Arena. This is safe as each spam_work has its own
        // arena
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
    // NOTE(kstandbridge): This is called once at startup, all your initialization code should be here
    // NOTE(kstandbridge): While not used in this example, if main.c is recompiled this method will NOT be called again
    app_state *AppState = Memory->AppState;
    
    // TODO(kstandbridge): Seed the random_state
    AppState->RandomState.Value = 65535;
    
    AppState->WorkerQueue = Platform.MakeWorkQueue(&AppState->Arena, 8);
    
    for(u32 WorkerIndex = 0;
        WorkerIndex < ArrayCount(AppState->SpamWorks);
        ++WorkerIndex)
    {
        // NOTE(kstandbridge): Bootsrap will allocate memory and place the returned pointer inside it. 
        // Giving you an arena with its first memory the struct
        spam_work *Worker = BootstrapPushStruct(spam_work, Arena);
        Worker->RandomState = &AppState->RandomState;
        AppState->SpamWorks[WorkerIndex] = Worker;
        
        Platform.AddWorkEntry(AppState->WorkerQueue, SpamThread, Worker);
    }
}

internal void
AppTick(app_memory *Memory, f32 dtForFrame)
{
    // NOTE(kstandbridge): This is the main application loop. Its called per frame in GUI based apps. 
    // Console app are set to 2 frames per second
    
    app_state *AppState = Memory->AppState;
    
    // NOTE(kstandbridge): This is a blocking call, it doesn't sleep but instead will take and process jobs from the queue
    Platform.CompleteAllWork(AppState->WorkerQueue);
    LogVerbose("All spam threads complete");
    
    // NOTE(kstandbridge): Setting this to false causes the main thread to stop looping which will close the application
    Memory->IsRunning = false;
}
