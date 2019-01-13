# AsyncConsoleIO-for-Debug
AsyncConsoleIO for Debug at Win32, MFC

// Start

ACIO::g_AsyncConsoleIO = ACIO::AsyncConsoleIO::GetInst();

// Destroy

ACIO::g_AsyncConsoleIO->DestroyInst();

// insert

ACIO::g_AsyncConsoleIO->bind_data("x", (void**)&g_lx, ACIO::EKeyType::Float);

// Watch
// https://www.youtube.com/watch?v=Om2rr3vXuho
