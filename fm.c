#include <curses.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

//Defining Window & Box Sizes
#define WINSTARTROW (getbegy(win) + 6)
#define WINENDROW (getmaxy(win) - 1)
#define WINSTARTCOL (getbegx(win) + 1)
#define WINENDCOL (getmaxx(win) - 1)
#define WINWIDTH (WINENDCOL - WINSTARTCOL - 2)
#define WINHEIGHT (WINENDROW - WINSTARTROW - 2)
#define BOXTROW (WINHEIGHT / 10 + WINSTARTROW)
#define BOXBROW (WINENDROW - WINHEIGHT / 10)
#define BOXLCOL (WINWIDTH / 15)
#define BOXRCOL (WINENDCOL - WINWIDTH / 15)
#define BOXWIDTH (BOXRCOL - BOXLCOL - 1)
#define BOXHEIGHT (BOXBROW - BOXTROW - 1)
#define BOXMCOL (BOXWIDTH / 3)

void init_screen(WINDOW *win)
{
	box(win, '|', '-');
	noecho();
	cbreak();
	curs_set(0);
	mvwhline(win, WINSTARTROW, WINSTARTCOL, '-', WINWIDTH);
	mvwprintw(win, WINSTARTROW - 3, WINWIDTH / 2 - 5, "FileManager");
	mvwhline(win, BOXTROW, BOXLCOL + 1, '-', BOXWIDTH);
	mvwhline(win, BOXBROW, BOXLCOL + 1, '-', BOXWIDTH);
	mvwvline(win, BOXTROW + 1, BOXLCOL, '|', BOXHEIGHT);
	mvwvline(win, BOXTROW + 1, BOXRCOL, '|', BOXHEIGHT);
	mvwvline(win, BOXTROW + 1, BOXMCOL, '|', BOXHEIGHT);
}

void root_print(char *dir, int n, struct dirent **namelist, int print_row, int print_col)
{

	int	i;
		
		for(i = 0; i < n; i++)
		{
			if(strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0)
			{
				free(namelist[i]);
				continue;
			}
			mvprintw(print_row, print_col, "> %s", namelist[i]->d_name);
			print_row++;
		}
}

int main()
{
	//Create window
	WINDOW *win;
	win = initscr();
	init_screen(win);

	//create initial print and cursor positions
	int rprint_row = BOXTROW + 1,
		rprint_col = BOXMCOL + 2,
		lprint_row = BOXTROW + 1,
		lprint_col = BOXLCOL + 2;
	int curs_row = BOXTROW + 1, 
		curs_col = BOXMCOL + 4;

	wmove(win, curs_row, curs_col);

	//Create lists and print to window
	struct dirent **namelist;
	DIR *dirp;
	char *rootdir, *seldir, *nxtdir, *prevdir;
	int i, n;
	rootdir = (char *) malloc(PATH_MAX);
	seldir = (char *) malloc(PATH_MAX);
	nxtdir = (char *) malloc(PATH_MAX);
	prevdir = (char *) malloc(PATH_MAX);
	getcwd(seldir, PATH_MAX);
	rootdir = dirname(*seldir);
	mvwprintw(win, BOXTROW - 1, BOXLCOL + 1, "%s", seldir);
	
	//print left side
	n = scandir(rootdir, &namelist, 0, alphasort);
	root_print(rootdir, n, namelist, lprint_row, lprint_col);

	//print right side
	n = scandir(seldir,&namelist,0, alphasort);
	dirp = opendir(seldir);
	if(dirp)
	{
		for(i = 0; i < n; i++)
		{
			if(strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0)
			{
				free(namelist[i]);
				continue;
			}
			else
			{
				mvwprintw(win, rprint_row, rprint_col, "> %s", namelist[i]->d_name);
				rprint_row++;
			}
		}
		closedir(dirp);
	}
	wmove(win, curs_row, curs_col);
	wrefresh(win);

	//Main Loop of navigation and commands issued by user
	char input = '\0';
	
	while (input != 'q')
	{
		input = getch();
	}

	//open directories and print new lists

	//rename files and directories

	//remove files and directories

	//copy/paste files and directories

	//move files and directories

	//exit application
	endwin();
	return 0;
}
