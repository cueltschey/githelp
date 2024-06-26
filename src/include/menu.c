#include "repos.h"
#include "menu.h"
#include "util.h"
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>


void init_ncurses(){
    initscr();
    curs_set(0);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

void draw_menu(WINDOW *menu_win, char *options[], int n_options,  int highlight, const char* title) {
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    int x = 2;
    int y = 2;
    werase(menu_win);
    box(menu_win, 0, 0);
    int title_width = strlen(title) + 4; // Add padding for borders
    mvwprintw(menu_win, 0, (getmaxx(menu_win) - title_width) / 2, "[ %s ]", title);
    //wprintw(menu_win, "<< prev | next >>");

    int lines;
    if(n_options >= 30){
      lines = n_options;
    }
    else{
      lines = n_options - 1;
    }  
    for (int i = 0; i < lines; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", options[i]);
            wattroff(menu_win, A_REVERSE);
        } else {
            if(options[i][0] == '*'){
              attron(COLOR_PAIR(2));
              mvwprintw(menu_win, y, x, "%s", options[i]);
              attroff(COLOR_PAIR(2));
            }
            else{
              mvwprintw(menu_win, y, x, "%s", options[i]);
            }
        }
        ++y;
    }
    wrefresh(menu_win);
}

int get_options(char* repos[200], int n_repos, char* options[MAX_OPTIONS], int page){
    int start_index = (page - 1) * 30;
    int end_index = start_index + 30;
    if(end_index > n_repos) end_index = n_repos;

    // Fetch values for the given page
    for (int i = start_index; i < end_index; i++) {
      options[i - start_index] = repos[i];
    }
    return end_index - start_index;
}

int get_options_filtered(char* repos[200], int n_repos, char* options[MAX_OPTIONS], int page, char* filter){
    char** filtered_repos = (char**)malloc(n_repos * sizeof(char*));
    if (filtered_repos == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    int resultSize = 0;
    for (int i = 0; i < n_repos; ++i) {
        if(repos[i] == NULL) break;
        if(checkSubstring(repos[i], filter) == 1 || strstr(repos[i], filter) != NULL){
            filtered_repos[resultSize] = strdup(repos[i]);
            if (filtered_repos[resultSize] == NULL) {
               fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            resultSize++;
        }
    }
    int start_index = (page - 1) * 30;
    int end_index = start_index + 30;
    if(end_index > resultSize) end_index = resultSize;

    // Fetch values for the given page
    for (int i = start_index; i < end_index; i++) {
      options[i - start_index] = filtered_repos[i];
    }
    return end_index - start_index;
}

char* user_select_personal_repo(char* token, const char* username) {

    clear();
    refresh();

    char* repos[200];
    int n_repos = get_repos(username, repos, token);

    int choice = 0;
    WINDOW *menu_win;
    int c = 0;
    int page = 1;

    char *options[MAX_OPTIONS];
    int n_options = get_options(repos, n_repos, options, page);
    int highlight = 1;
    int height = n_options + 4; // Adjust the height of the menu window
    int width = 60; // Adjust the width of the menu window
    int starty = (LINES - height) / 2; // Center vertically
    int startx = (COLS - width) / 2; // Center horizontally
    const char* title = "Personal Repos";
    menu_win = newwin(height, width, starty, startx);
    char* search_term = "";
    int search_term_size = 0;
    keypad(menu_win, TRUE);
    while (1) {
        if(search_term_size != 0){
          draw_menu(menu_win, options, n_options, highlight, search_term);
        } else{
          draw_menu(menu_win, options, n_options, highlight, title);
        }
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_options;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_options)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case KEY_RIGHT:
                if(page * 30 < n_repos){
                  page++; 
                  choice = -1;
                }
                break;
            case KEY_LEFT:
                if(page > 1){
                  choice = -1;
                  page--;
                }
                break;
            case KEY_BACKSPACE:
                if(search_term_size != 0){
                  search_term = popChar(search_term);
                  search_term_size--;
                  page = 1;
                  n_options = get_options_filtered(repos, n_repos, options, page, search_term);
                } 
                if(search_term_size == 0){
                  n_options = get_options(repos, n_repos, options, page);
                }
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                search_term = appendChar(search_term, c);
                search_term_size++;
                page = 1;
                highlight = 1;
                n_options = get_options_filtered(repos, n_repos, options, page, search_term);
                break;
        }
        if (choice != 0){
          if(choice == -1){
            n_options = get_options(repos, n_repos, options, page);
            highlight = 1;
            choice = 0;
          }
          else break;
        }
    }
    clrtoeol();
    refresh();
    delwin(menu_win);
    endwin();
    return options[choice - 1];
}

char* user_select_foreign_repo(const char* username) {

    clear();
    refresh();

    char* repos[200];
    int n_repos = get_repos_noauth(username, repos);

    int choice = 0;
    WINDOW *menu_win;
    int c = 0;
    int page = 1;

    char *options[MAX_OPTIONS];
    int n_options = get_options(repos, n_repos, options, page);
    int highlight = 1;
    int height = n_options + 4; // Adjust the height of the menu window
    int width = 60; // Adjust the width of the menu window
    int starty = (LINES - height) / 2; // Center vertically
    int startx = (COLS - width) / 2; // Center horizontally
    char title[200];
    snprintf(title, sizeof(title), "Repos for User: %s", username);
    menu_win = newwin(height, width, starty, startx);
    char* search_term = "";
    int search_term_size = 0;
    keypad(menu_win, TRUE);
    while (1) {
        if(search_term_size != 0){
          draw_menu(menu_win, options, n_options, highlight, search_term);
        } else{
          draw_menu(menu_win, options, n_options, highlight, title);
        }
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_options;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_options)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case KEY_RIGHT:
                if(page * 30 < n_repos){
                  page++; 
                  choice = -1;
                }
                break;
            case KEY_LEFT:
                if(page > 1){
                  choice = -1;
                  page--;
                }
                break;
            case KEY_BACKSPACE:
                if(search_term_size != 0){
                  search_term = popChar(search_term);
                  search_term_size--;
                  page = 1;
                  n_options = get_options_filtered(repos, n_repos, options, page, search_term);
                } 
                if(search_term_size == 0){
                  n_options = get_options(repos, n_repos, options, page);
                }
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                search_term = appendChar(search_term, c);
                search_term_size++;
                page = 1;
                highlight = 1;
                n_options = get_options_filtered(repos, n_repos, options, page, search_term);
                break;
        }
        if (choice != 0){
          if(choice == -1){
            n_options = get_options(repos, n_repos, options, page);
            highlight = 1;
            choice = 0;
          }
          else break;
        }
    }
    clrtoeol();
    refresh();
    delwin(menu_win);
    endwin();
    return options[choice - 1];
}

char* user_create_repo(int max_length, const char* title) {
    curs_set(1);

    WINDOW *inputwin = newwin(3, max_length + 20, (LINES - 3) / 2, (COLS - max_length - 20) / 2);
    box(inputwin, 0, 0);
    refresh();
    wrefresh(inputwin);

    int title_width = strlen(title) + 4; // Add padding for borders
    mvwprintw(inputwin, 0, (getmaxx(inputwin) - title_width) / 2, "[ %s ]", title);
    keypad(inputwin, TRUE);

    mvwprintw(inputwin, 1, 1, "Enter text: ");
    wrefresh(inputwin);

    char *input = (char *)malloc(max_length + 1);
    memset(input, 0, max_length + 1);
    int ch, i = 0;
    // TODO: fix text jump error
    while ((ch = wgetch(inputwin)) != '\n') {
        if(ch == ' ') continue;
        if (ch == KEY_BACKSPACE && i > 0) {
            mvwprintw(inputwin, 1, i + 13, " ");
            wmove(inputwin, 1, i + 13);
            i--;
        } else if (isprint(ch) && i < max_length) {
            input[i++] = ch;
            mvwprintw(inputwin, 1, i + 13, "%c", ch);
            wmove(inputwin, 1, i + 14);
        }
        wrefresh(inputwin);
    }

    // Clean up
    delwin(inputwin);

    return input;
}

int user_choose_visibility(){
    curs_set(0);

    int highlight = 1;
    int x, y;
    int num_choices = 2;
    const char *choices[] = {
        "Private",
        "Public"
    };

    getmaxyx(stdscr, y, x);

    // Clear the screen
    clear();

    // Create a window for input
    WINDOW *win = newwin(7, x - 4, (y - 7) / 2, 2);
    box(win, 0, 0); // Draw a box around the window
    refresh();
    wrefresh(win); // Refresh the window

    // Print the menu
    mvprintw((y - 7) / 2 + 1, (x - strlen("Choose the visibility of the repository:")) / 2, "Choose the visibility of the repository:");
    refresh();

    // Print options
    for(int i = 0; i < num_choices; i++) {
        if(i == highlight - 1)
            attron(A_REVERSE);
        mvprintw((y - 7) / 2 + 3 + i, (x - strlen(choices[i])) / 2, "%s", choices[i]);
        attroff(A_REVERSE);
    }
    refresh();
    int choice = 0;

    // Loop to navigate through options
    while(1) {
        int c = getch();
        switch(c) {
            case KEY_UP:
                if(highlight == 1)
                    highlight = num_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if(highlight == num_choices)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case 10: // Enter key pressed
                choice = highlight;
                break;
            default:
                break;
        }
        // Highlight the current choice
        for(int i = 0; i < num_choices; i++) {
            if(i == highlight - 1)
                attron(A_REVERSE);
            mvprintw((y - 7) / 2 + 3 + i, (x - strlen(choices[i])) / 2, "%s", choices[i]);
            attroff(A_REVERSE);
        }
        refresh();
        if(choice != 0) // User made a selection
            break;
    }
    
    delwin(win);
    clear();
    endwin();

    return choice == 1;
}

void draw_quarter(int y, int x, int height, int width, const char* text, const char* desc, bool selected) {
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (selected) {
                attron(A_REVERSE); // Highlight if selected
            }
            mvaddch(i, j, ' ');
            attroff(A_REVERSE);
        }
    }
    int text_y = y + height / 2;
    int text_x = x + (width - strlen(text)) / 2;
    mvprintw(text_y - 2, text_x, "[ %s ]", text);
    int term_rows, term_cols;
    getmaxyx(stdscr, term_rows, term_cols);
    // Check if the terminal dimensions match the screen dimensions
    if (term_rows >= 40  && term_cols >= 40) {
      mvprintw(text_y, text_x - 18, "%s", desc);
    } 
}

int user_choose_option() {
    curs_set(0);
    keypad(stdscr, TRUE);
    int height, width;
    getmaxyx(stdscr, height, width);

    int box_height = height / 2;
    int box_width = width / 2;

    int selected_box = 0;
    int key;
    int choice = -1;
    char* options[4] = {
      "Clone Person Repo",
      "Clone Any Repo",
      "Create New Repo",
      "git CLI info"
    };
    
    char* descriptions[4] = {
      "Get a List of all personal repos and choose one of them.",
      "Enter the Repo name and Username to clone a repo",
      "Enter Name and privacy of a new Repo.",
      "Get help using git from the CLI"
    };

    while (1) {
        clear();
        int index = 0;

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                int y = i * box_height;
                int x = j * box_width;
                draw_quarter(y, x, box_height, box_width, options[index], descriptions[index], (i * 2 + j) == selected_box);
                index++;
            }
        }

        refresh();

        key = getch();
        switch (key) {
            case KEY_UP:
                selected_box = (selected_box - 2 + 4) % 4;
                break;
            case KEY_DOWN:
                selected_box = (selected_box + 2) % 4;
                break;
            case KEY_LEFT:
                selected_box = (selected_box - 1 + 4) % 4;
                break;
            case KEY_RIGHT:
                selected_box = (selected_box + 1) % 4;
                break;
            case '\n':
              choice = selected_box;
              break;

            default:
                break;
        }
        if(choice >= 0){
            break;
        }
    }
    clear();
    refresh();
    return choice + 1;
}

void user_help(){

  clear();
  refresh();

  mvprintw(1 , 1, "Git Tutorial:");
  mvprintw(3, 1, "1. To initialize a new repository, use:");
  mvprintw(4, 3, "git init");
  mvprintw(6, 1, "2. To add files to the staging area, use:");
  mvprintw(7, 3, "git add <file>");
  mvprintw(8, 1, "   To add all files, use:");
  mvprintw(9, 3, "git add .");
  mvprintw(11, 1, "3. To commit changes, use:");
  mvprintw(12, 3, "git commit -m \"Commit message\"");
  mvprintw(14, 1, "4. To push changes to a remote repository, use:");
  mvprintw(15, 3, "git push <remote_name> <branch_name>");
  mvprintw(17, 1, "Press any key to exit.");

  getch();
  endwin();
}




