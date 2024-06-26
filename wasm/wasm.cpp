#include "..\base.h"

#include <stdatomic.h>

#define WASM_EXPORT(name) __attribute__((export_name(#name))) name
#define WASM_IMPORT(name) __attribute__((import_name(#name))) name

static memory_arena Temp;

static u32 WasmPagesAllocated = 0;
#define PAGE_SIZE 65536


memory_arena AllocateArenaFromOS(u32 Size, u64 StartingAddress) {
    memory_arena Result = {0};

    u32 CommitSize = RoundUpPowerOf2(Size, (u32)PAGE_SIZE);

    Result.Start = (void *)(WasmPagesAllocated * PAGE_SIZE);
    Result.Size = CommitSize;
    Result.Offset = Result.Start;

    u32 PagesNeeded = CommitSize / PAGE_SIZE;
    s32 Success = __builtin_wasm_memory_grow(0, PagesNeeded);
    Assert(Success != -1);

    WasmPagesAllocated += PagesNeeded;

    (void)Success;
    (void)StartingAddress; // WASM already has a fixed address space

    return Result;
}

memory_arena memory_arena::CreateScratch() {
    memory_arena Result = {0};
    Result.Start = this->Offset;
    Result.Offset = Result.Start;
    Result.Size = this->Size - ((u8 *)this->Offset - (u8 *)this->Start);
    return Result;
}

void *memory_arena::Push(u64 Size, u32 Alignment) {
    Assert(PopCount(Alignment) == 1);

    u8 *AlignedOffset = (u8 *)this->Offset;
    AlignedOffset = (u8 *)RoundUpPowerOf2((u64)AlignedOffset, (u64)Alignment);

    u8 *NewOffset = AlignedOffset + Size;
    Assert((u64)NewOffset - (u64)this->Start < this->Size);
    this->Offset = (void *)NewOffset;

    return AlignedOffset;
}

void memory_arena::Pop(void *Ptr) {
    Assert((u64)Ptr >= (u64)this->Start && (u64)Ptr < (u64)this->Offset);
    this->Offset = Ptr;
}

void memory_arena::Reset() {
    this->Offset = this->Start;
}

f64 WASM_IMPORT(__now)();

f64 QueryTimestampInMilliseconds() {
    return __now();
}

void WASM_IMPORT(__break)();

void __WasmBreak() {
    __break();
}

u32 WASM_IMPORT(__getProcessorThreadCount)();
u32 GetProcessorThreadCount() {
    return __getProcessorThreadCount();
}

struct wasm_work_queue_data {
    atomic_uint_fast32_t WorkIndex;
    atomic_uint_fast32_t WorkCompleted;
    u32 WorkItemCount;

    thread_callback ThreadCallback;
    u32 ThreadCount;
};

void work_queue::Create(memory_arena *Arena, thread_callback ThreadCallback, u32 Count) {
    wasm_work_queue_data *WorkQueueData = (wasm_work_queue_data *)Arena->Push(sizeof(wasm_work_queue_data));

    __builtin_memset(WorkQueueData, 0, sizeof(wasm_work_queue_data));
    u32 ThreadCount = 1;
    WorkQueueData->ThreadCount = ThreadCount;
    WorkQueueData->ThreadCallback = ThreadCallback;

    this->OSData = WorkQueueData;
}

void work_queue::Start(u32 WorkItemCount) {
    wasm_work_queue_data *WorkQueueData = (wasm_work_queue_data *)this->OSData;
    WorkQueueData->WorkItemCount = WorkItemCount;
    WorkQueueData->WorkIndex = 0;
    WorkQueueData->WorkCompleted = 0;
}

void work_queue::Wait() {
    wasm_work_queue_data *WorkQueueData = (wasm_work_queue_data *)this->OSData;

    u32 WorkItemCount = WorkQueueData->WorkItemCount;
    u32 WorkEntry = atomic_fetch_add(&WorkQueueData->WorkIndex, 1);
    while (WorkEntry < WorkItemCount) {
        work_queue_context Context = {
            .WorkEntry = WorkEntry,
            .ThreadIndex = 0
        };
        WorkQueueData->ThreadCallback(&Context);
        ++WorkQueueData->WorkCompleted;
        WorkEntry = atomic_fetch_add(&WorkQueueData->WorkIndex, 1);
    }
}

static void InitWASMEnvironmentProperties() {
    WasmPagesAllocated = __builtin_wasm_memory_size(0);
    Temp = AllocateArenaFromOS(MB(256), 0);
}

void WASM_EXPORT(init)() {
    InitWASMEnvironmentProperties();
    init_params Params;
    OnInit(&Params);
}

static u64 KeyState[2] = {0};
static u64 PrevKeyState[2] = {0};

void *WASM_EXPORT(draw)(u32 Width, u32 Height) {
    PrevKeyState[0] = KeyState[0];
    PrevKeyState[1] = KeyState[1];

    memory_arena Scratch = Temp.CreateScratch();
    image Image = CreateImage(&Scratch, Width, Height, format::R8G8B8A8_U32);
    OnRender(Image);
    return Image.Data;
}

void WASM_EXPORT(updateKeyState)(u32 KeyCode, u32 IsDown) {
    bool HighBits = KeyCode >= 64;
    if (HighBits) KeyCode -= 64;
    u64 BitToUpdate = 1 << KeyCode;

    if (IsDown > 0) {
        KeyState[HighBits] |= BitToUpdate;
    } else {
        KeyState[HighBits] &= ~BitToUpdate;
    }
}

void WASM_EXPORT(resetKeyState)() {
    PrevKeyState[0] = 0;
    PrevKeyState[1] = 0;
    KeyState[0] = 0;
    KeyState[1] = 0;
}

bool IsDown(key Key) {
    bool HighBits = (u32)Key >= 64;
    u64 BitToSet = (u64)Key;
    if (HighBits) BitToSet -= 64;
    bool Result = (KeyState[HighBits] & (1 << BitToSet)) != 0;
    return Result;
}
bool IsUp(key Key) {
    bool HighBits = (u32)Key >= 64;
    u64 BitToSet = (u64)Key;
    if (HighBits) BitToSet -= 64;
    bool Result = (KeyState[HighBits] & (1 << BitToSet)) == 0;
    return Result;
}
bool WasReleased(key Key) {
    return false;
}
bool WasPressed(key Key) {
    return false;
}

bool IsDown(button Button) {
    return false;
}
bool IsUp(button Button) {
    return false;
}
bool WasReleased(button Button) {
    return false;
}
bool WasPressed(button Button) {
    return false;
}

bool IsDown(mouse_button Button) {
    return false;
}
bool IsUp(mouse_button Button) {
    return false;
}
bool WasReleased(mouse_button Button) {
    return false;
}
bool WasPressed(mouse_button Button) {
    return false;
}