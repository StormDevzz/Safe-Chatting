# SafeChatting for Linux

## Dependencies

**Debian / Ubuntu:**
```bash
sudo apt update
sudo apt install gcc make libssl-dev
```

**Arch / CachyOS:**
```bash
sudo pacman -S gcc make openssl
```

**Fedora:**
```bash
sudo dnf install gcc make openssl-devel
```

## Build

```bash
cd c/linux
make
```

## Run

```bash
cd c/linux
./build/chatserver    # server
./build/chat          # client
```
