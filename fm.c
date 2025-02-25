#include <stdio.h>
#include <curses.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <ftw.h>
#include "header.h"

WINDOW *win;
int frame = 1; //determines which pane the cursor is on (1 is right, 0 is left)
int lprint_row,
	rprint_row,
	rprint_col,
	lprint_col;
//need to add a scroll variable which indicates what is the first [s] printed in the window
//can calculate from there what selection is to be chosen when doing any actions
/*i.e. 
int list = 0; 
when curs_row is at BOXBROW - 1, list++ when scrolling down by 1. 
Then it reprints the list starting from selection[list] all the way to the bottom.
could possibly include hotkey that will scroll down by a whole page for speed
*/

//set cursor rows/cols
int	lcurs_row,
	lcurs_col,
	rcurs_row,
	rcurs_col;

//define directories
char *seldir;
char *nextdir,
	 *prevdir;

char input = '\0';

int main()
{
	//create window
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
	//q to quit
	while(input != 'q' && input != 27)
	{
		input = getch();

		if(input == 'h' || input == 'D')
			move_left();
		if(input == 'j' || input == 'B')
		{
			if(frame == 0)
				move_down(&lcurs_row, lcurs_col, lprint_row);
			if(frame == 1)
				move_down(&rcurs_row, rcurs_col, rprint_row);
		}
		if(input == 'k' || input == 'A')
		{
			if(frame == 0)
				move_up(&lcurs_row, lcurs_col);
			if(frame == 1)
				move_up(&rcurs_row, rcurs_col);
		}
		if(input == 'l' || input == 'C')
			move_right();
		if(input == ' ' || input == '\n')
			move_select(lselection, rselection);
		if(input == 'b')
			move_parent(lselection, rselection);
		if(input == 'd')
			select_delete(lselection, rselection);
		if(input == 'r')
			select_rename(lselection, rselection);
/*		if(input == 'm')
			select_move;
		if(input == 'c')
			select_copy;
		if(input == 'p')
			select_paste;
*/
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
	if(lprint_row < BOXBROW)
	{
		if(strcmp(namelist[i]->d_name, ".") == 0 ||
		   strcmp(namelist[i]->d_name, "..") == 0 ||
		   namelist[i]->d_type != 4)
		{
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
				dirtree(subcheck, selection, x, &s);
			}
	}	else
		{
			strcpy(&selection[s][0], namelist[i]->d_name);
			s++;
			strcpy(subcheck, "/");
			strcat(subcheck, namelist[i]->d_name);
			dirtree(subcheck, selection, x, &s);
		}
	}
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
		if(lprint_row < BOXBROW)
		{
			if(strcmp(sublist[a]->d_name, ".") == 0 ||
			   strcmp(sublist[a]->d_name, "..") == 0 ||
			   sublist[a]->d_type != 4)
			{
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
					dirtree(check, selection, x + 1, s);
				}
		}	else
			{
				strcpy(&selection[*s][0], sublist[a]->d_name);
				(*s)++;
				strcpy(check, subcheck);
				strcat(check, "/");
				strcat(check, sublist[a]->d_name);
				dirtree(check, selection, x+1, s);
			}
		}
	}
}

void dir_print(char selection[MAXLIST][42])
{
	struct dirent **namelist;
	int i, n, s = 0;
	n = scandir(seldir, &namelist, 0, alphasort);

	for(i = 0; i < n; i++)
	{
	if(rprint_row < BOXBROW)
	{
		if(strcmp(namelist[i]->d_name, ".") == 0 ||
		   strcmp(namelist[i]->d_name, "..") == 0 ||
		   namelist[i]->d_type != 8)
		{
			continue;
		}	else
			{
				mvprintw(rprint_row, rprint_col, "> %s", namelist[i]->d_name);
				strcpy(&selection[s][0], namelist[i]->d_name);
				s++;
				rprint_row++;
			}
	}	else
		{
			strcpy(&selection[s][0], namelist[i]->d_name);
			s++;
		}
	}
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
	int s;

	if(frame == 0)
		s = lcurs_row - (BOXTROW + 1);
		else return;
	if(dircheck(lselection, rselection, &s) == 0)
	{
		clear_box();
		print_path();
		root_print(lselection);
		dir_print(rselection);
		return;
	}
}

int dircheck(char lselection[MAXLIST][42], char rselection[MAXLIST][42], int *s)
{
	int i, n;
	struct dirent **namelist;
	char *temp = malloc(PATH_MAX);
	strcpy(temp, seldir);
	n = scandir(seldir, &namelist, 0, alphasort);
	for(i = 0; i < n; i++)
	{
		if(frame == 0)
		{
			if(namelist[i]->d_type != 8 && strcmp(namelist[i]->d_name, &lselection[*s][0]) == 0)
			{
				if(seldir == temp)
					return 1;
				if(strcmp(seldir, "/") != 0)
					strcat(seldir, "/");
				strcat(seldir, &lselection[*s][0]);
				return 0;
			}	else 
				{
					continue;
				}
		}	else
			{
				if(strcmp(namelist[i]->d_name, &rselection[*s][0]) == 0)
				{
					if(seldir == temp)
						return 1;
					strcat(seldir, "/");
					strcat(seldir, &rselection[*s][0]);
					return 0;
				}	else 
					{
						continue;
					}
			}
	}
	seldir = dirname(temp);
	dircheck(lselection, rselection, s);
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

int deldir(const char *fpath, const struct stat *sbm, int typeflag, struct FTW *ftwbuf)
{
	return remove(fpath);
}

void select_delete(char lselection[MAXLIST][42], char rselection[MAXLIST][42])
{
	char *selected = malloc(PATH_MAX),
		 *temp = malloc(PATH_MAX);
	int s;
	strcpy(temp, seldir);

	if(frame == 0)
	{
		s = lcurs_row - (BOXTROW + 1);
	}	else 
		{
			s = rcurs_row - (BOXTROW + 1);
		}

	dircheck(lselection, rselection, &s);

	if(frame == 1)
	{
		strcpy(selected, seldir);
		clear_box();
		mvprintw(BOXTROW + 1, BOXMCOL + 1, "Delete %s? [y/n]", selected);
		for(;;)
		{
			input = getch();
			if(input == 'y')
			{
				remove(selected);
				break;
			}
			if(input == 'n')
				break;
		}
	}	else
		{
			clear_box();
			strcpy(selected, seldir);
			mvprintw(BOXTROW + 1, BOXMCOL + 1, "Delete %s and all of its contents? [y/n]", selected);
			for(;;)
			{
				input = getch();
				if(input == 'y')
				{
					nftw(selected, deldir, 64, FTW_DEPTH | FTW_PHYS);
					break;
				}
				if(input == 'n')
				{
					break;
				}
			}
		}
	strcpy(seldir, temp);
	clear_box();
	print_path();
	root_print(lselection);
	dir_print(rselection);
}

void select_rename(char lselection[MAXLIST][42], char rselection[MAXLIST][42])
{
	char *new_name = malloc(PATH_MAX);
	char *temp = malloc(PATH_MAX);
	int i, s;
	if(frame == 0)
		s = lcurs_row - (BOXTROW + 1);
		else s = rcurs_row - (BOXTROW + 1);
	strcpy(temp, seldir);
	if(dircheck(lselection, rselection, &s) == 0)
	{
		if(frame == 0)
		{
			for(i = BOXLCOL + 2; i < BOXMCOL; i++)
			{
				mvprintw(lcurs_row, i, " ");
			}
		}	else
			{
				for(i = BOXMCOL + 2; i < BOXRCOL; i++)
				{
					mvprintw(rcurs_row, i, " ");
				}
			}
	}
	if(frame == 0)
		move(lcurs_row, lcurs_col);
		else move(rcurs_row, rcurs_col);
	curs_set(1);
	echo();
	getstr(new_name);
	curs_set(0);
	noecho();
	if(strcmp(seldir, temp) == 0)
	{
		rename(seldir, new_name);
	}	else
		{
			rename(seldir, new_name);
			strcpy(seldir, temp);
		}
	clear_box();
	print_path();
	root_print(lselection);
	dir_print(rselection);
}
