# AsyncConsoleIO-for-Debug
AsyncConsoleIO for Debug at Win32, MFC

#define USE_AsyncConsoleIO

#include "AsyncConsoleIO.h"

// Start

ACIO::g_AsyncConsoleIO = ACIO::AsyncConsoleIO::GetInst();

// Destroy

ACIO::g_AsyncConsoleIO->DestroyInst();

// bind

ACIO::g_AsyncConsoleIO->bind_data("x", (void**)&g_lx, ACIO::EKeyType::Float);

// Watch

// https://www.youtube.com/watch?v=Om2rr3vXuho

