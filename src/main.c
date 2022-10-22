typedef struct app_state
{
    memory_arena Arena;
    int foo;
} app_state;

extern void
AppHandleCommand(app_memory *Memory, string Command, u32 ArgCount, string *Args)
{
}

internal void
AppInit(app_memory *Memory)
{
}

internal void
AppTick(app_memory *Memory, f32 dtForFrame)
{
}
