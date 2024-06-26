#include <ncurses.h>

#ifndef MENU__h
#define MENU__h


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define MAX_OPTIONS 31


void init_ncurses();

void draw_menu(WINDOW *menu_win, char *options[], int n_options,  int highlight, const char* title);

int get_options(char* repos[200], int n_repos, char* options[MAX_OPTIONS], int page);

int get_options_filtered(char* repos[200], int n_repos, char* options[MAX_OPTIONS], int page, char* filter);

void filter_search_terms(char *search_terms[], int num_terms);

char* user_select_personal_repo(char* token, const char* username);

char* user_select_foreign_repo(const char* username);

char* user_create_repo(int maxsize, const char* title);

int user_choose_visibility();

int user_choose_option();

void user_help();

#endif
