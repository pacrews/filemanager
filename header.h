#define TOPROW	(getbegy(stdscr) + 6)
#define BOTROW	(getmaxy(stdscr) - 1)
#define LCOL	(getbegx(stdscr) + 1)
#define RCOL	(getmaxx(stdscr) - 1)
#define WIDTH	(RCOL - LCOL)
#define HEIGHT	(BOTROW - TOPROW)
#define BOXTROW	(HEIGHT / 10 + TOPROW)
#define BOXBROW	(BOTROW - HEIGHT / 10)
#define BOXLCOL	(WIDTH / 15)
#define BOXRCOL (RCOL - WIDTH / 15)
#define BWIDTH	(BOXRCOL - BOXLCOL - 1)
#define BHEIGHT	(BOXBROW - BOXTROW - 1)
#define BOXMCOL	(BWIDTH / 3)
#define MAXLIST PATH_MAX

void init_screen(WINDOW *win);

void print_path(void);

void root_print(char selection[MAXLIST][42]);

void dirtree(char *check, char selection[MAXLIST][42], int x, int *s);

void dir_print(char selection[MAXLIST][42]);

void clear_box(void);

void move_left(void);

void move_down(int *curs_row, int curs_col, int print_row);

void move_up(int *curs_row, int curs_col);

void move_right(void);

void move_select(char lselection[MAXLIST][42], char rselection[MAXLIST][42]);

int dircheck(char lselection[MAXLIST][42], char rselection[MAXLIST][42], int *s);

void move_parent(char lselection[MAXLIST][42], char rselection[MAXLIST][42]);

int deldir(const char *fpath, const struct stat *sbm, int typeflag, struct FTW *ftwbuf);

void select_delete(char lselection[MAXLIST][42], char rselection[MAXLIST][42]);

void select_rename(char lselection[MAXLIST][42], char rselection[MAXLIST][42]);
