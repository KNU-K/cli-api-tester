#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "menu.h"
#include "loglib.h"
#include "network.h"
#define BUF_SZ 4096
void create_menu1(MenuList, int, int);
void create_menu2(MenuList, int, int);
void select_method(Method);
Target getTarget();
void process();
void initTarget();
void init_screen();
void initLogMode();
void setHostAddrOfTarget(HostAddr);

static Target t;
char buf[BUF_SZ];
static int current_menu = 0; // 0 for main menu, 1 for setting menu
static int selected = 0;     // Currently selected menu item
static int main_menu_size ;
static int setting_menu_size;
LogMode logMode;

char* getHostAddress() {
    char* hostAddress = (char*)malloc(1024); // Assuming IPv4 address, 15 characters plus null terminator
    if (hostAddress == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    printw("\n\nEnter IP address: ");
    refresh();
    echo(); // Enable echoing user input
    getstr(hostAddress); // Read the user input
    noecho(); // Disable echoing user input
    setHostAddrOfTarget(hostAddress);
    return hostAddress;
}

void main_menu(){
    MenuList main_menu_contents = {"START TESTING", "SETTING", "EXIT"};
     main_menu_size = sizeof(main_menu_contents) / sizeof(main_menu_contents[0]);
   
    clear();
    
    addstr("==========[CLI REST-API TESTER]=========");
    create_menu1(main_menu_contents, main_menu_size, selected);
}

void setting_menu(){
    MenuList setting_menu_contents = {"EDIT HOST", "EDIT METHOD", "LOG MODE", "BACK"};
    setting_menu_size = sizeof(setting_menu_contents) / sizeof(setting_menu_contents[0]);

    clear();
    t = getTarget();
    addstr("=================[INFO]=================\n");
    sprintf(buf, " > url : %s \n", t.hostAddr);
    addstr(buf);
    sprintf(buf, " > method : %s \n", t.method);
    addstr(buf);
    
    sprintf(buf, " > log mode : %s \n", logMode ==LOG_MODE_ON? "ON" : "OFF");
    addstr(buf);
    addstr("\n");
    addstr("=================[MENU]=================");
    create_menu2(setting_menu_contents, setting_menu_size, selected);
    
}
void run(){
    initTarget();
    init_screen();
    initLogMode();
    keypad(stdscr, TRUE);

    while (1) {
        switch (current_menu)
        {
        case 0:
            main_menu();
            break;
        default:
            setting_menu();
        }

        int ch;
        ch = getch();
        switch (ch) {
            case KEY_UP:
                if(current_menu == 0){
                    selected = (selected - 1 + main_menu_size) % main_menu_size;
                }else{
                    selected = (selected - 1 + setting_menu_size) %setting_menu_size;
                }   
                break;
            case KEY_DOWN:
            
                if(current_menu == 0){
                    selected = (selected + 1) % main_menu_size;
                }else{
                    selected = (selected + 1) % setting_menu_size;
                }
                break;
            case '\n':
                if (current_menu == 0) {
                    // Handle main menu selection
                    if (selected == 0) {
                        // RUN selected
                        // Add your logic for RUN menu item here
                        endwin();
                        process();


                    } else if (selected == 1) {
                        // SETTING selected
                        current_menu = 1;
                        selected = 0;
                    } else if (selected == 2) {
                        endwin();
                        exit(1);
                    }
                } else {
                    // Handle setting menu selection
                    if (selected == 0) {
                        getHostAddress();
                        current_menu = 1;
                        selected =0;
                    } else if (selected == 1) {
                        select_method(t.method);
                        current_menu = 1;
                        selected =0;
                        // BACK selected
                    }else if(selected == 2){
                        if(logMode==LOG_MODE_ON) {
                            logMode = LOG_MODE_OFF;
                            endwin();
                            system("clear");
                            printf("LOG MODE OFF!\nSaved in the log.txt file.\n");
                            sleep(2);
                        }
                        else {
                            logMode = LOG_MODE_ON;
                            endwin();
                            system("clear");
                            printf("LOG MODE ON!\nSaved in the log.txt file.\n");
                            sleep(2);
                        }
                        
                    }
                    else if(selected == 3){
                        current_menu = 0;
                        selected = 0;
                    }
                }
                break;
        }
    }

    endwin();
    exit(1);
}

int main() {
    run();
}
