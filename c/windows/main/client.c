#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

static SOCKET sock = INVALID_SOCKET;
static int running = 1;
static char in_buf[4096];
static int in_len = 0;
static char my_name[64] = {0};
static HANDLE hStd = NULL;

static void cleanup(void)
{
    running = 0;
    if (sock != INVALID_SOCKET) closesocket(sock);
    WSACleanup();
    printf("\n");
}

BOOL WINAPI handler(DWORD dwCtrlType)
{
    (void)dwCtrlType;
    cleanup();
    exit(0);
    return TRUE;
}

static int connect_to(const char *host, const char *port)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) return -1;

    for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == INVALID_SOCKET) continue;
        if (connect(sock, rp->ai_addr, (int)rp->ai_addrlen) == 0) break;
        closesocket(sock); sock = INVALID_SOCKET;
    }
    freeaddrinfo(res);
    return sock != INVALID_SOCKET ? 0 : -1;
}

static void send_cmd(const char *fmt, ...)
{
    char b[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(b, sizeof(b), fmt, ap);
    va_end(ap);
    send(sock, b, (int)strlen(b), 0);
    send(sock, "\n", 1, 0);
}

static void print_msg(const char *from, const char *text)
{
    int h = 0;
    for (int i = 0; from[i]; i++) h = from[i] + ((h << 5) - h);
    int color = 10 + (abs(h) % 6);
    if (color == 10) color = 11;
    SetConsoleTextAttribute(hStd, color);
    printf("%s: ", from);
    SetConsoleTextAttribute(hStd, 7);
    printf("%s\n", text);
    SetConsoleTextAttribute(hStd, 14);
    printf("%s ➤ %s", my_name, in_buf);
    SetConsoleTextAttribute(hStd, 7);
    fflush(stdout);
}

static void show_prompt(void)
{
    printf("\r");
    for (int i = 0; i < 80; i++) printf(" ");
    printf("\r");
    SetConsoleTextAttribute(hStd, 14);
    printf("%s ➤ %s", my_name, in_buf);
    SetConsoleTextAttribute(hStd, 7);
    fflush(stdout);
}

static void handle_server(const char *line)
{
    char typ[64], data[4096 * 4];
    if (sscanf(line, "%[^|]|%[^\n]", typ, data) < 1) return;

    if (strcmp(typ, "MSG") == 0) {
        char from[64], msg[2048];
        if (sscanf(data, "%[^|]|%[^\n]", from, msg) == 2)
            print_msg(from, msg);
    } else if (strcmp(typ, "OK") == 0) {
        SetConsoleTextAttribute(hStd, 10);
        printf("✓ %s\n", data);
        SetConsoleTextAttribute(hStd, 7);
        char tmp[64];
        if (sscanf(data, "logged in as %s", tmp) == 1)
            strncpy(my_name, tmp, sizeof(my_name) - 1);
        show_prompt();
    } else if (strcmp(typ, "ERR") == 0) {
        SetConsoleTextAttribute(hStd, 12);
        printf("✗ %s\n", data);
        SetConsoleTextAttribute(hStd, 7);
        show_prompt();
    } else if (strcmp(typ, "USERS") == 0) {
        SetConsoleTextAttribute(hStd, 11);
        printf("Users: %s\n", data);
        SetConsoleTextAttribute(hStd, 7);
        show_prompt();
    }
}

static int wait_msgs(int ms)
{
    WSAPOLLFD pf = {.fd = sock, .events = POLLIN};
    int total = 0;
    while (WSAPoll(&pf, 1, ms) > 0) {
        char buf[4096 * 4];
        int n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = 0;
        char *line = buf;
        while (line && *line) {
            char *nl = strchr(line, '\n');
            if (nl) *nl = 0;
            if (*line) handle_server(line);
            if (nl) line = nl + 1; else break;
        }
        total = 1;
    }
    return total;
}

static void read_field(const char *prompt, char *out, int max)
{
    int pos = 0;
    printf("%s", prompt); fflush(stdout);
    while (1) {
        if (_kbhit()) {
            char c = _getch();
            if (c == '\r') { out[pos] = 0; printf("\n"); break; }
            if ((c == 8 || c == 127) && pos > 0) { pos--; printf("\b \b"); }
            else if (pos < max - 1) { out[pos++] = c; printf("*"); }
            fflush(stdout);
        }
        WSAPOLLFD pf = {.fd = sock, .events = POLLIN};
        if (WSAPoll(&pf, 1, 50) > 0) wait_msgs(0);
    }
}

int main(int argc, char **argv)
{
    const char *host = "127.0.0.1";
    const char *port = "8080";
    if (argc > 1) host = argv[1];
    if (argc > 2) port = argv[2];

    hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCtrlHandler(handler, TRUE);

    CONSOLE_CURSOR_INFO ci = {1, 0};
    SetConsoleCursorInfo(hStd, &ci);

    printf("SafeChatting Client\n");

    if (connect_to(host, port) < 0)
        { fprintf(stderr, "Cannot connect to %s:%s\n", host, port); return 1; }

    wait_msgs(500);

    char choice[16] = {0};
    while (running && !my_name[0]) {
        char user[64] = {0}, pass[64] = {0};

        printf("\rR)egister / L)ogin / Q)uit: ");
        fflush(stdout);
        int pos = 0;
        while (1) {
            if (_kbhit()) {
                char c = _getch();
                if (c == '\r') { choice[pos] = 0; printf("\n"); break; }
                if ((c == 8 || c == 127) && pos > 0) { pos--; printf("\b \b"); }
                else if (pos < 15) { choice[pos++] = c; printf("%c", c); }
                fflush(stdout);
            }
            WSAPOLLFD pf = {.fd = sock, .events = POLLIN};
            if (WSAPoll(&pf, 1, 50) > 0) wait_msgs(0);
        }

        if (strcmp(choice, "q") == 0 || strcmp(choice, "exit") == 0) break;

        if (strcmp(choice, "r") == 0 || strcmp(choice, "register") == 0) {
            read_field("Name: ", user, 63);
            read_field("Password: ", pass, 63);
            send_cmd("REGISTER|%s|%s", user, pass);
            wait_msgs(500);
        } else if (strcmp(choice, "l") == 0 || strcmp(choice, "login") == 0) {
            read_field("Name: ", user, 63);
            read_field("Password: ", pass, 63);
            send_cmd("LOGIN|%s|%s", user, pass);
            wait_msgs(500);
        }
    }

    if (my_name[0]) {
        printf("\nWelcome, %s!\n", my_name);
        printf(" @user text — send\n /users — list\n /exit — quit\n\n");

        while (running) {
            if (_kbhit()) {
                char c = _getch();
                if (c == '\r') {
                    if (in_len == 0) { show_prompt(); continue; }
                    in_buf[in_len] = 0;

                    if (strcmp(in_buf, "/exit") == 0 || strcmp(in_buf, "/quit") == 0) break;
                    if (strcmp(in_buf, "/users") == 0) send_cmd("LIST");
                    else if (in_buf[0] == '@') {
                        char *sp = strchr(in_buf, ' ');
                        if (sp) {
                            *sp = 0;
                            send_cmd("SEND|%s|%s", in_buf + 1, sp + 1);
                            print_msg(my_name, sp + 1);
                        } else {
                            printf("Format: @user text\n");
                        }
                    } else {
                        printf("Format: @user text\n");
                    }
                    in_len = 0; in_buf[0] = 0;
                    show_prompt();
                } else if ((c == 8 || c == 127) && in_len > 0) {
                    in_buf[--in_len] = 0; show_prompt();
                } else if (c >= 32 && c < 127) {
                    if (in_len < 4095) { in_buf[in_len++] = c; in_buf[in_len] = 0; show_prompt(); }
                }
            }

            WSAPOLLFD pf = {.fd = sock, .events = POLLIN};
            if (WSAPoll(&pf, 1, 50) > 0) {
                char buf[4096 * 4];
                int n = recv(sock, buf, sizeof(buf) - 1, 0);
                if (n <= 0) { printf("\nServer disconnected\n"); break; }
                buf[n] = 0;
                char *line = buf;
                while (line && *line) {
                    char *nl = strchr(line, '\n');
                    if (nl) *nl = 0;
                    if (*line) handle_server(line);
                    if (nl) line = nl + 1; else break;
                }
                show_prompt();
            }
        }
    }

    cleanup();
    return 0;
}
