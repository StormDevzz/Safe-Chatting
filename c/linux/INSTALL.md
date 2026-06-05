# Safe Chatting для Linux

## Установка зависимостей

**Debian / Ubuntu / Mint:**
```
sudo apt update
sudo apt install gcc make libssl-dev
```

**Arch / CachyOS / Manjaro:**
```
sudo pacman -S gcc make openssl
```

## Сборка

```
cd c/linux
make
```

## Запуск

```
cd c/linux
./build/chatserver   ← сервер
./build/chat         ← клиент
```

## Как пользоваться

1. Запусти сервер на одном компьютере
2. Запусти клиент на двух разных терминалах (или компьютерах)
3. В клиенте: зарегистрируйся (R) или войди (L)
4. Пиши `@имя_пользователя текст` — сообщение уйдёт адресату
5. `/users` — список пользователей, `/exit` — выход

Сервер слушает порт 8080. Клиент по умолчанию стучится на 127.0.0.1:8080.
