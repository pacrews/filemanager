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
#define MAXLIST (BHEIGHT + 1)

void init_screen(WINDOW *win);

void print_path(void);

void root_print(char selection[MAXLIST]);

void dirtree(char *check, char selection[MAXLIST], int x, int *s);

void dir_print(char selection[MAXLIST]);

void clear_box(void);

void move_left(void);

void move_down(int *curs_row, int curs_col, int print_row);

void move_up(int *curs_row, int curs_col);

void move_right(void);

void move_select(char lselection[MAXLIST], char rselection[MAXLIST]);

void move_parent(char lselection[MAXLIST], char rselection[MAXLIST]);

