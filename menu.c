#include "menu.h"
#include "network.h"
#include <curses.h>
#include <string.h>
void setMethodOfTarget(Method);
void init_screen(){
    initscr();
    clear();
    noecho();
    cbreak();
}
void create_menu1(MenuList menuList, int size, int selected) {
    int i;
    for (i = 0; i < size; i++) {
        if (i == selected) {
            attron(A_REVERSE);
            mvprintw(i + 2, 1, "%s", menuList[i]);
            attroff(A_REVERSE);
        } else {
            mvprintw(i + 2, 1, "%s", menuList[i]);
        }
    }
    refresh();
}
void create_menu2(MenuList menuList, int size, int selected) {
    int i;
    for (i = 0; i < size; i++) {
        if (i == selected) {
            attron(A_REVERSE);
            mvprintw(i + 6, 1, "%s", menuList[i]);
            attroff(A_REVERSE);
        } else {
            mvprintw(i + 6, 1, "%s", menuList[i]);
        }
    }
    refresh();
}
void select_method(Method method) {
    MenuList select_method_menu = {"GET", "POST", "PUT", "DELETE","BACK"};

    int select_method_size = sizeof(select_method_menu) / sizeof(select_method_menu[0]);
    int i;
    for(i=0;i<select_method_size;i++){
        if(strcmp(select_method_menu[i],method) == 0)break;
    }
    int selected_method = i;

    while (1) {
        clear();
        
        addstr("==============[METHOD SETTING]=============\n");
        create_menu1(select_method_menu, select_method_size, selected_method);

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected_method = (selected_method - 1 + select_method_size) % select_method_size;
                break;
            case KEY_DOWN:
                selected_method = (selected_method + 1) % select_method_size;
                break;
            case '\n':
                clear();
                refresh();
                if(strcmp(select_method_menu[selected_method],"BACK")==0) return;
                setMethodOfTarget(select_method_menu[selected_method]);
               // return select_method_menu[selected_method];
               return;
        }
    }
}