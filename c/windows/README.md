# SafeChatting for Windows

## Build

### Option 1: MinGW (recommended)
1. Install [MinGW-w64](https://www.mingw-w64.org/) or [MSYS2](https://www.msys2.org/)
2. Open MSYS2 / MinGW terminal
3. Install OpenSSL: `pacman -S mingw-w64-x86_64-openssl`
4. Build:
```bash
cd c/windows
make
```

### Option 2: MSVC (Visual Studio)
1. Open "Developer Command Prompt for VS"
2. Install OpenSSL for Windows
3. Build:
```cmd
cd c\windows
cl /O2 /I. main\server.c connection\connection.c chatting\chatting.c encryption\encryption.c encryption\crypto.c files\files.c /Fe:build\chatserver.exe /link libcrypto.lib ws2_32.lib
cl /O2 /I. main\client.c /Fe:build\chat.exe /link ws2_32.lib
```

## Run

```cmd
cd c\windows
build\chatserver.exe    # server
build\chat.exe          # client
```

## Notes
- Requires OpenSSL 3.x installed
- Server listens on port 8080 by default
- Client connects to 127.0.0.1:8080 by default
