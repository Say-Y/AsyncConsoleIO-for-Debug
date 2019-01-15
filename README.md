# AsyncConsoleIO-for-Debug
AsyncConsoleIO for Debug at Win32, MFC

- Watch
  - https://www.youtube.com/watch?v=7a-KTMNPXPA
  - https://www.youtube.com/watch?v=iRV6-qBKSr4

- Define
  - #define USE_AsyncConsoleIO
  - #include "AsyncConsoleIO.h"

- Start
  - ACIO::g_AsyncConsoleIO = ACIO::AsyncConsoleIO::GetInst();

- Destroy
  - ACIO::g_AsyncConsoleIO->DestroyInst();

- Bind
  - ACIO::g_AsyncConsoleIO->bind_data("x", (void**)&g_fx, ACIO::EKeyType::Float);
  - ACIO::g_AsyncConsoleIO->bind_data_forced("y", (void**)&iy, ACIO::EKeyType::Int);

- Force value to be fixed
  - ACIO::g_AsyncConsoleIO->bind_data_forced("b", (void**)&b, ACIO::EKeyType::Float);
    - At Console -> Inupt: key value (ex. b 10)
      - when is call bind_data_forced -> b = 10;
    - At Console -> Input: value failed (ex. a -)
      - when is call bind_data_forced -> a = default
