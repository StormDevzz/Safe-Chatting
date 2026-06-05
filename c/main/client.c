#include "../../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <signal.h>
#include <stdarg.h>
#include <strings.h>

static int sock = -1, running = 1;
static char in_buf[MAX_LINE];
static int in_len = 0;
static char my_name[64] = {0};
static struct termios old_tio;

static void cleanup(int sig)
{
    (void)sig;
    running = 0;
    tcsetattr(0, TCSANOW, &old_tio);
    if (sock >= 0) close(sock);
    printf("\033[0m\033[?25h\n");
    exit(0);
}

static void set_raw(int on)
{
    static struct termios new_tio;
    if (on) {
        tcgetattr(0, &old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &new_tio);
    } else {
        tcsetattr(0, TCSANOW, &old_tio);
    }
}

static int connect_to(const char *host, const char *port)
{
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res)) return -1;

    for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) continue;
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sock); sock = -1;
    }
    freeaddrinfo(res);
    return sock;
}

static void send_cmd(const char *fmt, ...)
{
    char b[MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(b, sizeof(b), fmt, ap);
    va_end(ap);
    write(sock, b, strlen(b));
    write(sock, "\n", 1);
}

static void print_msg(const char *from, const char *text)
{
    int h = 0;
    for (int i = 0; from[i]; i++) h = from[i] + ((h << 5) - h);
    int color = 31 + (abs(h) % 6);
    printf("\033[2K\r\033[1;%dm%s\033[0m: %s\n", color, from, text);
    printf("\033[33m%s\033[0m ➤ %s", my_name, in_buf);
    fflush(stdout);
}

static void show_prompt(void)
{
    printf("\033[2K\r\033[33m%s\033[0m ➤ %s", my_name, in_buf);
    fflush(stdout);
}

static void handle_server(const char *line)
{
    char typ[64], data[MAX_LINE * 4];
    if (sscanf(line, "%[^|]|%[^\n]", typ, data) < 1) return;

    if (strcmp(typ, RSP_MSG) == 0) {
        char from[64], msg[MAX_MSG];
        if (sscanf(data, "%[^|]|%[^\n]", from, msg) == 2)
            print_msg(from, msg);
    } else if (strcmp(typ, RSP_OK) == 0) {
        printf("\033[2K\r\033[32m✓ %s\033[0m\n", data);
        char tmp[64];
        if (sscanf(data, "logged in as %s", tmp) == 1)
            strncpy(my_name, tmp, sizeof(my_name) - 1);
        show_prompt();
    } else if (strcmp(typ, RSP_ERR) == 0) {
        printf("\033[2K\r\033[31m✗ %s\033[0m\n", data);
        show_prompt();
    } else if (strcmp(typ, RSP_USERS) == 0) {
        printf("\033[2K\r\033[36mПользователи:\033[0m %s\n", data);
        show_prompt();
    }
}

static int wait_msgs(int ms)
{
    struct pollfd pf = {.fd = sock, .events = POLLIN};
    int total = 0;
    while (poll(&pf, 1, ms) > 0) {
        char buf[MAX_LINE * 4];
        int n = read(sock, buf, sizeof(buf) - 1);
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
        struct pollfd pf[2] = {{.fd=0,.events=POLLIN},{.fd=sock,.events=POLLIN}};
        if (poll(pf, 2, -1) <= 0) break;
        if (pf[1].revents & POLLIN) { wait_msgs(0); continue; }
        char c;
        if (read(0, &c, 1) != 1) break;
        if (c == '\n') { out[pos] = 0; printf("\n"); break; }
        if ((c == 127 || c == '\b') && pos > 0) { pos--; printf("\b \b"); }
        else if (pos < max - 1) { out[pos++] = c; printf("*"); }
        fflush(stdout);
    }
}

int main(int argc, char **argv)
{
    const char *host = "127.0.0.1";
    const char *port = PORT;
    if (argc > 1) host = argv[1];
    if (argc > 2) port = argv[2];

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    printf("\033[1;35m"
" SSSSSSSSSSSSSSS                      ffffffffffffffff                               CCCCCCCCCCCCChhhhhhh                                       tttt               tttt            iiii                                       \n"
" SS:::::::::::::::S                    f::::::::::::::::f                           CCC::::::::::::Ch:::::h                                    ttt:::t            ttt:::t           i::::i                                      \n"
"S:::::SSSSSS::::::S                   f::::::::::::::::::f                        CC:::::::::::::::Ch:::::h                                    t:::::t            t:::::t            iiii                                       \n"
"S:::::S     SSSSSSS                   f::::::fffffff:::::f                       C:::::CCCCCCCC::::Ch:::::h                                    t:::::t            t:::::t                                                       \n"
"S:::::S              aaaaaaaaaaaaa    f:::::f       ffffffeeeeeeeeeeee          C:::::C       CCCCCC h::::h hhhhh         aaaaaaaaaaaaa  ttttttt:::::tttttttttttttt:::::ttttttt    iiiiiiinnnn  nnnnnnnn       ggggggggg   ggggg\n"
"S:::::S              a::::::::::::a   f:::::f           ee::::::::::::ee       C:::::C               h::::hh:::::hhh      a::::::::::::a t:::::::::::::::::tt:::::::::::::::::t    i:::::in:::nn::::::::nn    g:::::::::ggg::::g\n"
" S::::SSSS           aaaaaaaaa:::::a f:::::::ffffff    e::::::eeeee:::::ee     C:::::C               h::::::::::::::hh    aaaaaaaaa:::::at:::::::::::::::::tt:::::::::::::::::t     i::::in::::::::::::::nn  g:::::::::::::::::g\n"
"  SS::::::SSSSS               a::::a f::::::::::::f   e::::::e     e:::::e     C:::::C               h:::::::hhh::::::h            a::::atttttt:::::::tttttttttttt:::::::tttttt     i::::inn:::::::::::::::ng::::::ggggg::::::gg\n"
"    SSS::::::::SS      aaaaaaa:::::a f::::::::::::f   e:::::::eeeee::::::e     C:::::C               h::::::h   h::::::h    aaaaaaa:::::a      t:::::t            t:::::t           i::::i  n:::::nnnn:::::ng:::::g     g:::::g \n"
"       SSSSSS::::S   aa::::::::::::a f:::::::ffffff   e:::::::::::::::::e      C:::::C               h:::::h     h:::::h  aa::::::::::::a      t:::::t            t:::::t           i::::i  n::::n    n::::ng:::::g     g:::::g \n"
"            S:::::S a::::aaaa::::::a  f:::::f         e::::::eeeeeeeeeee       C:::::C               h:::::h     h:::::h a::::aaaa::::::a      t:::::t            t:::::t           i::::i  n::::n    n::::ng:::::g     g:::::g \n"
"            S:::::Sa::::a    a:::::a  f:::::f         e:::::::e                 C:::::C       CCCCCC h:::::h     h:::::ha::::a    a:::::a      t:::::t    tttttt  t:::::t    tttttt i::::i  n::::n    n::::ng::::::g    g:::::g \n"
"SSSSSSS     S:::::Sa::::a    a:::::a f:::::::f        e::::::::e                 C:::::CCCCCCCC::::C h:::::h     h:::::ha::::a    a:::::a      t::::::tttt:::::t  t::::::tttt:::::ti::::::i n::::n    n::::ng:::::::ggggg:::::g \n"
"S::::::SSSSSS:::::Sa:::::aaaa::::::a f:::::::f         e::::::::eeeeeeee          CC:::::::::::::::C h:::::h     h:::::ha:::::aaaa::::::a      tt::::::::::::::t  tt::::::::::::::ti::::::i n::::n    n::::n g::::::::::::::::g \n"
"S:::::::::::::::SS  a::::::::::aa:::af:::::::f          ee:::::::::::::e            CCC::::::::::::C h:::::h     h:::::h a::::::::::aa:::a       tt:::::::::::tt    tt:::::::::::tti::::::i n::::n    n::::n  gg::::::::::::::g \n"
" SSSSSSSSSSSSSSS     aaaaaaaaaa  aaaafffffffff            eeeeeeeeeeeeee               CCCCCCCCCCCCC hhhhhhh     hhhhhhh  aaaaaaaaaa  aaaa         ttttttttttt        ttttttttttt  iiiiiiii nnnnnn    nnnnnn    gggggggg::::::g \n"
"                                                                                                                                                                                                                        g:::::g \n"
"                                                                                                                                                                                                            gggggg      g:::::g \n"
"                                                                                                                                                                                                            g:::::gg   gg:::::g \n"
"                                                                                                                                                                                                             g::::::ggg:::::::g \n"
"                                                                                                                                                                                                              gg:::::::::::::g  \n"
"                                                                                                                                                                                                                ggg::::::ggg    \n"
"                                                                                                                                                                                                                   gggggg      \n"
"\033[0m"
           "\033[36m  Клиент SafeChatting\033[0m\n");

    if (connect_to(host, port) < 0)
        { fprintf(stderr, "Ошибка: %s:%s\n", host, port); return 1; }

    set_raw(1);
    printf("\033[?25l");
    wait_msgs(500);

    char choice[16] = {0};
    while (running && !my_name[0]) {
        char user[64] = {0}, pass[64] = {0};

        printf("\r\033[K\033[36mR)ег / L)ог / Q)вых:\033[0m ");
        fflush(stdout);
        int pos = 0;
        while (1) {
            struct pollfd pf[2] = {{.fd=0,.events=POLLIN},{.fd=sock,.events=POLLIN}};
            if (poll(pf, 2, -1) <= 0) break;
            if (pf[1].revents & POLLIN) { wait_msgs(0); continue; }
            char c;
            if (read(0, &c, 1) != 1) break;
            if (c == '\n') { choice[pos] = 0; printf("\n"); break; }
            if ((c == 127 || c == '\b') && pos > 0) { pos--; printf("\b \b"); }
            else if (pos < 15) { choice[pos++] = c; printf("%c", c); }
            fflush(stdout);
        }

        if (strcasecmp(choice, "q") == 0 || strcasecmp(choice, "exit") == 0) break;

        if (strcasecmp(choice, "r") == 0 || strcasecmp(choice, "register") == 0) {
            read_field("\033[33mИмя: \033[0m", user, 63);
            read_field("\033[33mПароль: \033[0m", pass, 63);
            send_cmd("REGISTER|%s|%s", user, pass);
            wait_msgs(500);
        } else if (strcasecmp(choice, "l") == 0 || strcasecmp(choice, "login") == 0) {
            read_field("\033[33mИмя: \033[0m", user, 63);
            read_field("\033[33mПароль: \033[0m", pass, 63);
            send_cmd("LOGIN|%s|%s", user, pass);
            wait_msgs(500);
        }
    }

    if (my_name[0]) {
        printf("\n\033[32mДобро пожаловать, %s!\033[0m\n", my_name);
        printf("  \033[36m@user текст\033[0m — написать\n");
        printf("  \033[36m/users\033[0m — список\n");
        printf("  \033[36m/exit\033[0m — выход\n\n");

        struct pollfd fds[2] = {{.fd=0,.events=POLLIN},{.fd=sock,.events=POLLIN}};

        while (running) {
            if (poll(fds, 2, -1) <= 0) break;

            if (fds[0].revents & POLLIN) {
                char c;
                if (read(0, &c, 1) != 1) break;

                if (c == '\n') {
                    if (in_len == 0) { show_prompt(); continue; }
                    in_buf[in_len] = 0;

                    if (strcmp(in_buf, "/exit") == 0 || strcmp(in_buf, "/quit") == 0) break;
                    if (strcmp(in_buf, "/users") == 0) { send_cmd("LIST"); }
                    else if (in_buf[0] == '@') {
                        char *sp = strchr(in_buf, ' ');
                        if (sp) {
                            *sp = 0;
                            send_cmd("SEND|%s|%s", in_buf + 1, sp + 1);
                            printf("\033[2K\r");
                            print_msg(my_name, sp + 1);
                        } else {
                            printf("\033[2K\r\033[31mФормат: @user текст\033[0m\n");
                        }
                    } else {
                        printf("\033[2K\r\033[31mФормат: @user текст\033[0m\n");
                    }
                    in_len = 0; in_buf[0] = 0;
                    show_prompt();
                } else if (c == 127 || c == '\b') {
                    if (in_len > 0) { in_buf[--in_len] = 0; show_prompt(); }
                } else if (c >= 32 && c < 127) {
                    if (in_len < MAX_LINE - 1) { in_buf[in_len++] = c; in_buf[in_len] = 0; show_prompt(); }
                }
            }

            if (fds[1].revents & POLLIN) {
                char buf[MAX_LINE * 4];
                int n = read(sock, buf, sizeof(buf) - 1);
                if (n <= 0) { printf("\n\033[31mСервер отключился\033[0m\n"); break; }
                buf[n] = 0;
                char *line = buf;
                while (line && *line) {
                    char *nl = strchr(line, '\n');
                    if (nl) *nl = 0;
                    if (*line) { printf("\r\033[K"); handle_server(line); }
                    if (nl) line = nl + 1; else break;
                }
                show_prompt();
            }
        }
    }

    cleanup(0);
    return 0;
}
