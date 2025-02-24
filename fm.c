#include <curses.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "header.h"

int frame = 1; //determines which pane the cursor is on (1 is right, 0 is left)
int lprint_row,
	rprint_row,
	rprint_col,
	lprint_col;

//set cursor rows/cols
int	lcurs_row,
	lcurs_col,
	rcurs_row,
	rcurs_col;

//define directories
char *seldir;
char *nextdir,
	 *prevdir;

int main()
{
	//create window
	WINDOW *win;
	win = initscr();
	init_screen(win);

	//set print rows/cols
	lprint_row = BOXTROW + 1,
	rprint_row = BOXTROW + 1,
	rprint_col = BOXMCOL + 2,
	lprint_col = BOXLCOL + 2;

	//set cursor rows/cols
	lcurs_row = BOXTROW + 1,
	lcurs_col = BOXLCOL + 2,
	rcurs_row = BOXTROW + 1,
	rcurs_col = BOXMCOL + 2;
	
	seldir = malloc(PATH_MAX);
	getcwd(seldir, PATH_MAX);
	nextdir = malloc(PATH_MAX),
	prevdir = malloc(PATH_MAX);

	//define lists to pull selections from
	char	lselection[MAXLIST][42],
			rselection[MAXLIST][42];

	//print current path above box
	print_path();

	//print directory list on left pane
	root_print(lselection);

	//print contents on right pane
	dir_print(rselection);

	//init formatting; setting default cursor on right pane and right box blinks
	//to indicate where cursor is
	move_right();

	//user loop
	char input = '\0';
	//q to quit
	while(input != 'q')
	{
		input = getch();

		if(input == 'h')
			move_left();
		if(input == 'j')
		{
			if(frame == 0)
				move_down(&lcurs_row, lcurs_col, lprint_row);
			if(frame == 1)
				move_down(&rcurs_row, rcurs_col, rprint_row);
		}
		if(input == 'k')
		{
			if(frame == 0)
				move_up(&lcurs_row, lcurs_col);
			if(frame == 1)
				move_up(&rcurs_row, rcurs_col);
		}
		if(input == 'l')
			move_right();
		if(input == ' ')
			move_select(lselection, rselection);
		if(input == 'b')
			move_parent(lselection, rselection);
		if(input == 'd')
			select_del(lselection, rselection);
		wrefresh(win);
	}

	//end window and close application after exiting loop with 'q'
	endwin();
	return 0;
}

void init_screen(WINDOW *win)
{
	box(win, '|', '-');
	noecho();
	cbreak();
	curs_set(0);
	mvwhline(win, TOPROW, LCOL, '-', WIDTH);
	mvwprintw(win, TOPROW - 3, WIDTH / 2 - 5, "FileManager");
	mvwhline(win, BOXTROW, BOXLCOL + 1, '-', BWIDTH);
	mvwhline(win, BOXBROW, BOXLCOL + 1, '-', BWIDTH);
	mvwvline(win, BOXTROW + 1, BOXLCOL, '|', BHEIGHT);
	mvwvline(win, BOXTROW + 1, BOXRCOL, '|', BHEIGHT);
	mvwvline(win, BOXTROW + 1, BOXMCOL, '|', BHEIGHT);
	mvchgat(BOXTROW - 1, BOXLCOL + 1, BOXRCOL - BOXLCOL, A_BOLD, A_COLOR, NULL);
}

void print_path(void)
{
	int i, n;
	for(i = BOXTROW - 1, n = BOXLCOL + 1; n < BOXRCOL; n++)
	{
		mvprintw(i, n, " ");
	}
	mvprintw(BOXTROW - 1, BOXLCOL + 1, "%s", seldir);
}

void root_print(char selection[MAXLIST][42])
{
	char *dir = malloc(2);
	strcpy(dir, "/");
	struct dirent **namelist;
	int i, n, x = 1, s = 0;
	char *subcheck = malloc(PATH_MAX);
	n = scandir(dir, &namelist, 0, alphasort);

	for(i = 0; i < n; i++)
	{
		if(strcmp(namelist[i]->d_name, ".") == 0 ||
		   strcmp(namelist[i]->d_name, "..") == 0 ||
		   namelist[i]->d_type != 4)
		{
			free(namelist[i]);
			continue;
		}	else
			{
				mvprintw(lprint_row, lprint_col, "> %s", namelist[i]->d_name);
				strcpy(&selection[s][0], namelist[i]->d_name);
				s++;
				strcpy(subcheck, "/");
				strcat(subcheck, namelist[i]->d_name);
				if(strcmp(subcheck, seldir) == 0)
				{
					mvchgat(lprint_row, BOXLCOL + 2, BOXMCOL - BOXLCOL - 3, A_STANDOUT, A_COLOR, NULL);
					lcurs_row = lprint_row;
				}
				lprint_row++;
				free(namelist[i]);
				dirtree(subcheck, selection, x, &s);
			}
	}
	free(namelist);
}

void dirtree(char *check, char selection[MAXLIST][42], int x, int *s)
{
	struct dirent **sublist;
	int a, b;
	char *subcheck = malloc(PATH_MAX);
	strcpy(subcheck, check);
	if(strncmp(check, seldir, strlen(check)) == 0)
	{
		b = scandir(check, &sublist, 0, alphasort);
		for(a = 0; a < b; a++)
		{
			if(strcmp(sublist[a]->d_name, ".") == 0 ||
			   strcmp(sublist[a]->d_name, "..") == 0 ||
			   sublist[a]->d_type != 4)
			{
				free(sublist[a]);
				continue;
			}	else
				{
					mvprintw(lprint_row, lprint_col + x, "> %s", sublist[a]->d_name);
					strcpy(&selection[*s][0], sublist[a]->d_name);
					(*s)++;
					strcpy(check, subcheck);
					strcat(check, "/");
					strcat(check, sublist[a]->d_name);
					if(strcmp(check, seldir) == 0)
					{
						mvchgat(lprint_row, BOXLCOL + 2, BOXMCOL - BOXLCOL - 3, A_STANDOUT, A_COLOR, NULL);
						lcurs_row = lprint_row;
					}
					lprint_row++;
					free(sublist[a]);
					dirtree(check, selection, x + 1, s);
				}
		}
		free(sublist);
	}
}

void dir_print(char selection[MAXLIST][42])
{
	struct dirent **namelist;
	int i, n, s = 0;
	n = scandir(seldir, &namelist, 0, alphasort);

	for(i = 0; i < n; i++)
	{
		if(strcmp(namelist[i]->d_name, ".") == 0 ||
		   strcmp(namelist[i]->d_name, "..") == 0 ||
		   namelist[i]->d_type != 8)
		{
			free(namelist[i]);
			continue;
		}	else
			{
				mvprintw(rprint_row, rprint_col, "> %s", namelist[i]->d_name);
				strcpy(&selection[s][0], namelist[i]->d_name);
				s++;
				rprint_row++;
				free(namelist[i]);
			}
	}
	free(namelist);
	if(rprint_row == BOXTROW + 1)
	{
		mvprintw(rprint_row, rprint_col, "No files in directory");
		move_left();
	}
		else
		{
			mvchgat(BOXTROW + 1, BOXMCOL + 2, BOXRCOL - BOXMCOL - 3, A_STANDOUT, A_COLOR, NULL);
			move_right();
		}
}

void clear_box(void)
{
	int i, n;

	for(i = BOXTROW + 1; i < BOXBROW; i++)
	{
		for(n = BOXLCOL + 1; n < BOXMCOL; n++)
		{
			mvprintw(i, n, " ");
		}
		for(n = BOXMCOL + 1; n < BOXRCOL; n++)
		{
			mvprintw(i, n, " ");
		}
	}
	mvchgat(lcurs_row, lcurs_col, BOXMCOL-BOXLCOL-3, A_NORMAL, A_COLOR, NULL);
	mvchgat(rcurs_row, rcurs_col, BOXRCOL-BOXMCOL - 3, A_NORMAL, A_COLOR, NULL);
	rcurs_row = BOXTROW + 1;
	lprint_row = BOXTROW + 1;
	rprint_row = BOXTROW + 1;
}

void move_left(void)
{
	int i, n;
	if(frame == 1)
	{
		frame = 0;
	}
	mvchgat(BOXTROW, BOXMCOL, BOXRCOL - BOXMCOL, A_NORMAL, A_COLOR, NULL);
	mvchgat(BOXBROW, BOXMCOL, BOXRCOL - BOXMCOL, A_NORMAL, A_COLOR, NULL);
	for(i = BOXTROW + 1; i < BOXBROW; i++)
	{
		mvchgat(i, BOXRCOL, 1, A_NORMAL, A_COLOR, NULL);
	}

	mvchgat(BOXTROW, BOXLCOL + 1, BOXMCOL - BOXLCOL, A_BLINK, A_COLOR, NULL);
	mvchgat(BOXBROW, BOXLCOL + 1, BOXMCOL - BOXLCOL, A_BLINK, A_COLOR, NULL);
	for(n = BOXTROW + 1; n < BOXBROW; n++)
	{
		mvchgat(n, BOXLCOL, 1, A_BLINK, A_COLOR, NULL);
	}
	move(lcurs_row, lcurs_col);
}

void move_down(int *curs_row, int curs_col, int print_row)
{
	if(*curs_row < print_row - 1)
	{
		if(frame == 0)
			mvchgat(*curs_row, curs_col, BOXMCOL - BOXLCOL - 3, A_NORMAL, A_COLOR, NULL);
		if(frame == 1)
			mvchgat(*curs_row, curs_col, BOXRCOL - BOXMCOL - 3, A_NORMAL, A_COLOR, NULL);
		(*curs_row)++;
		move(*curs_row, curs_col);
		if(frame == 0)
			mvchgat(*curs_row, curs_col, BOXMCOL - BOXLCOL - 3, A_STANDOUT, A_COLOR, NULL);
		if(frame == 1)
			mvchgat(*curs_row, curs_col, BOXRCOL - BOXMCOL - 3, A_STANDOUT, A_COLOR, NULL);
	}
}

void move_up(int *curs_row, int curs_col)
{
	if(*curs_row > BOXTROW + 1 )
	{
		if(frame == 0)
			mvchgat(*curs_row, curs_col, BOXMCOL - BOXLCOL - 3, A_NORMAL, A_COLOR, NULL);
		if(frame == 1)
			mvchgat(*curs_row, curs_col, BOXRCOL - BOXMCOL - 3, A_NORMAL, A_COLOR, NULL);
		(*curs_row)--;
		move(*curs_row, curs_col);
		if(frame == 0)
			mvchgat(*curs_row, curs_col, BOXMCOL - BOXLCOL - 3, A_STANDOUT, A_COLOR, NULL);
		if(frame == 1)
			mvchgat(*curs_row, curs_col, BOXRCOL - BOXMCOL - 3, A_STANDOUT, A_COLOR, NULL);
	}
}

void move_right(void)
{
	int i, n;
	if(frame == 0)
	{
		frame = 1;
	}
	mvchgat(BOXTROW, BOXMCOL, BOXRCOL - BOXMCOL, A_BLINK, A_COLOR, NULL);
	mvchgat(BOXBROW, BOXMCOL, BOXRCOL - BOXMCOL, A_BLINK, A_COLOR, NULL);
	for(i = BOXTROW + 1; i < BOXBROW; i++)
	{
		mvchgat(i, BOXRCOL, 1, A_BLINK, A_COLOR, NULL);
	}

	mvchgat(BOXTROW, BOXLCOL + 1, BOXMCOL - BOXLCOL, A_NORMAL, A_COLOR, NULL);
	mvchgat(BOXBROW, BOXLCOL + 1, BOXMCOL - BOXLCOL, A_NORMAL, A_COLOR, NULL);
	for(n = BOXTROW + 1; n < BOXBROW; n++)
	{
		mvchgat(n, BOXLCOL, 1, A_NORMAL, A_COLOR, NULL);
	}
	move(rcurs_row, rcurs_col);
}

//this function currently only allows selection of directories for navigation
void move_select(char lselection[MAXLIST][42], char rselection[MAXLIST][42])
{
	int n, s;
	char *temp = malloc(PATH_MAX);
	strcpy(temp, seldir);

	clear_box();

	if(frame == 0)
		s = lcurs_row - (BOXTROW + 1);
		else return;
	if(dircheck(lselection, rselection, &s) == 0)
		return;
	seldir = dirname(temp);
	move_select(lselection, rselection);
}

int dircheck(char lselection[MAXLIST][42], char rselection[MAXLIST][42], int *s)
{
	int i, n;
	struct dirent **namelist;
	n = scandir(seldir, &namelist, 0, alphasort);
	for(i = 0; i < n; i++)
	{
		if(strcmp(namelist[i]->d_name, &lselection[*s][0]) == 0)
		{
			strcat(seldir, "/");
			strcat(seldir, &lselection[*s][0]);
			print_path();
			root_print(lselection);
			dir_print(rselection);
			return 0;
		}	else 
			{
				free(namelist[i]);
				continue;
			}
	}
}

void move_parent(char lselection[MAXLIST][42], char rselection[MAXLIST][42])
{
	if(strcmp(seldir, "/") == 0)
		return;

	prevdir = dirname(seldir);
	strcpy(seldir, prevdir);
	
	clear_box();
	
	print_path();
	root_print(lselection);
	dir_print(rselection);
}

void select_del(char lselection[MAXLIST][42], char rselection[MAXLIST][42])
{
	char *selected = malloc(PATH_MAX),
		 *temp = malloc(PATH_MAX);
	int s;
	struct dirent **namelist;
	strcpy(temp, seldir);

	if(frame == 0)
	{
		s = lcurs_row - (BOXTROW + 1);
	}	else 
		{
			s = rcurs_row - (BOXTROW + 1);
		}

	dircheck(lselection, rselection, &s);

	remove(selected);
	clear_box();
	print_path();
	root_print(lselection);
	dir_print(rselection);
}
