/* File: main-gtk.c */

/*
 * Copyright (c) 2000 Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "angband.h"


#ifdef USE_GTK

#include "main.h"
#include "maid-x11.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


#define NO_PADDING 0

#define MAX_TERM_DATA 8

/*
 * Path to the GTK settings file
 */
char settings[1024];


/*** Main gtk code ***/

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data
{
	term t;

	GtkWidget *window;
	GtkWidget *drawing_area;
	GdkFont *font;
	GdkPixmap *pixmap;
	GdkGC *gc;

	int font_wid;
	int font_hgt;

	int x_save;
	int y_save;

	int rows;
	int cols;

	cptr name;
	cptr lfont;
	cptr sfont;
};


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];


/*
 * game in progress
 */
static bool game_in_progress = FALSE;

/*
 * Number of active terms
 */
static int num_term = 1;


/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	int width, height;

	term_data *td = (term_data*)(Term->data);

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

	/* Find proper dimensions for rectangle */
	gdk_drawable_get_size(td->drawing_area->window, &width, &height);

	/* Clear the area */
	gdk_draw_rectangle(td->pixmap, td->drawing_area->style->black_gc,
	                   1, 0, 0, width, height);

	/* Copy it to the window */
	gdk_draw_drawable(td->drawing_area->window, td->gc, td->pixmap,
	                0, 0, 0, 0, width, height);

	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

	gdk_draw_rectangle(td->pixmap, td->drawing_area->style->black_gc,
	                   TRUE, x * td->font_wid, y * td->font_hgt,
	                   n * td->font_wid, td->font_hgt);

	/* Copy it to the window */
	gdk_draw_drawable(td->drawing_area->window, td->gc, td->pixmap,
	                x * td->font_wid, y * td->font_hgt,
	                x * td->font_wid, y * td->font_hgt,
	                n * td->font_wid, td->font_hgt);

	/* Success */
	return (0);
}


/*
 * Set foreground color
 */
static void set_foreground_color(term_data *td, byte a)
{
	static unsigned int failed = 0;

	GdkColor color;

	color.red   = color_table[a].rv * 256;
	color.green = color_table[a].gv * 256;
	color.blue  = color_table[a].bv * 256;

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

	if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, FALSE, TRUE))
		gdk_gc_set_foreground(td->gc, &color);
	else if (!failed++)
		g_print("Couldn't allocate color.\n");
}


/*
 * Draw some textual characters.
 */
static errr Term_text_gtk(int x, int y, int n, byte a, cptr s)
{
	int i;
	term_data *td = (term_data*)(Term->data);

	set_foreground_color(td, a);

	/* Clear the line */
	Term_wipe_gtk(x, y, n);

	/* Draw the text to the pixmap */
	for (i = 0; i < n; i++)
	{
		gdk_draw_text(td->pixmap, td->font, td->gc,
		              (x + i) * td->font_wid,
			      td->font->ascent + y * td->font_hgt,
			      s + i, 1);
	}

	/* Copy it to the window */
	gdk_draw_drawable(td->drawing_area->window, td->gc, td->pixmap,
	                x * td->font_wid, y * td->font_hgt,
	                x * td->font_wid, y * td->font_hgt,
	                n * td->font_wid, td->font_hgt);

	/* Success */
	return (0);
}


static errr CheckEvent(bool wait)
{
	if (wait)
	{
		gtk_main_iteration();
	}
	else
	{
		while (gtk_events_pending())
			gtk_main_iteration();
	}

	return (0);
}


static errr Term_flush_gtk(void)
{
	/* XXX */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_gtk(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE: gdk_beep(); return (0);

		/* Flush the output */
		case TERM_XTRA_FRESH: return (0);

		/* Process random events */
		case TERM_XTRA_BORED: return (CheckEvent(0));

		/* Process Events */
		case TERM_XTRA_EVENT: return (CheckEvent(v));

		/* Flush the events */
		case TERM_XTRA_FLUSH: return (Term_flush_gtk());

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: return (0);

		/* Clear the screen */
		case TERM_XTRA_CLEAR: return (Term_clear_gtk());

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
			if (v > 0) usleep(1000 * v);
			return (0);

		/* React to changes */
		case TERM_XTRA_REACT: return (0);
	}

	/* Unknown */
	return (1);
}


static errr Term_curs_gtk(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	set_foreground_color(td, TERM_YELLOW);

	gdk_draw_rectangle(td->pixmap, td->gc, FALSE,
	                   x * td->font_wid, y * td->font_hgt, td->font_wid - 1, td->font_hgt - 1);

	/* Copy it to the window */
	gdk_draw_drawable(td->drawing_area->window, td->gc, td->pixmap,
	                x * td->font_wid, y * td->font_hgt,
	                x * td->font_wid, y * td->font_hgt,
	                td->font_wid, td->font_hgt);

	/* Success */
	return (0);
}

static void save_game_gtk(void)
{
	if (game_in_progress && character_generated)
	{
		if (!inkey_flag)
		{
			plog("You may not do that right now.");
			return;
		}

		/* Hack -- Forget messages */
		msg_flag = FALSE;

		/* Save the game */
		do_cmd_save_game(FALSE);
	}
}


static void quit_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	save_game_gtk();

	quit(NULL);
}


static void destroy_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	quit(NULL);
}


static void hide_event_handler(GtkWidget *window, gpointer user_data)
{
	gtk_widget_hide(window);
}


static void new_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	if (game_in_progress)
	{
		plog("You can't start a new game while you're still playing!");
	}
	else
	{
		game_in_progress = TRUE;
		Term_flush();
		play_game(TRUE);
		cleanup_angband();
		quit(NULL);
	}
}


static void load_font(term_data *td, cptr fontname)
{
	td->font = gdk_font_load(fontname);

	/* Calculate the size of the font XXX */
	td->font_wid = gdk_char_width(td->font, '@');
	td->font_hgt = td->font->ascent + td->font->descent;
}

static errr Term_rows_gtk(bool fifty_rows)
{
	term_data *td = &data[0]; /* Main window */


	/* Set to 50-row display */
	if (fifty_rows)
	{
		screen_rows = 50;

		/* Change to small font */
		load_font(td, td->sfont);
	}

	/* Set to 25-line display */
	else
	{
		screen_rows = 25;

		/* Change to normal font */
		load_font(td, td->lfont);
	}

	/* Clear the screen */
	Term_clear_gtk();

	/* XXX XXX do something else? */


	/* Assume success */
	return (0);
}





static void file_ok_callback(GtkWidget *widget, GtkWidget *file_selector)
{
	const char *f = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));

	my_strcpy(savefile, f, sizeof(savefile));

	gtk_widget_destroy(file_selector);

	game_in_progress = TRUE;
	Term_flush();
	play_game(FALSE);
	cleanup_angband();
	quit(NULL);
}


static void open_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	GtkWidget *file_selector;
	char buf[1024];

	if (game_in_progress)
	{
		plog("You can't open a new game while you're still playing!");
	}
	else
	{
		/* Prepare the savefile path */
		path_build(buf, sizeof(buf), ANGBAND_DIR_SAVE, "*");

		file_selector = gtk_file_selection_new("Select a savefile");
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selector), buf);
		g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		                   "clicked", GTK_SIGNAL_FUNC(file_ok_callback), (gpointer)file_selector);

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		                          "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
		                          (gpointer)file_selector);

		g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
		                          "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
		                          (gpointer)file_selector);

		gtk_window_set_modal(GTK_WINDOW(file_selector), TRUE);
		gtk_widget_show(GTK_WIDGET(file_selector));
	}
}


static gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	save_game_gtk();

	/* Don't prevent closure */
	return (FALSE);
}


static gboolean keypress_event_handler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	int i, mc, ms, mo, mx;

	char msg[128];

	/* Extract four "modifier flags" */
	mc = (event->state & GDK_CONTROL_MASK) ? TRUE : FALSE;
	ms = (event->state & GDK_SHIFT_MASK) ? TRUE : FALSE;
	mo = (event->state & GDK_MOD1_MASK) ? TRUE : FALSE;
	mx = (event->state & GDK_MOD3_MASK) ? TRUE : FALSE;

	/*
	 * Hack XXX
	 * Parse shifted numeric (keypad) keys specially.
	 */
	if ((event->state == GDK_SHIFT_MASK) &&
	    (event->keyval >= GDK_KP_0) && (event->keyval <= GDK_KP_9))
	{
		/* Build the macro trigger string */
		sprintf(msg, "%cS_%X%c", 31, event->keyval, 13);

		/* Enqueue the "macro trigger" string */
		for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

		/* Hack -- auto-define macros as needed */
		if (event->length && (macro_find_exact(msg) < 0))
		{
			/* Create a macro */
			macro_add(msg, event->string);
		}

		return (TRUE);
	}

	/* Normal keys with no modifiers */
	if (event->length && !mo && !mx)
	{
		/* Enqueue the normal key(s) */
		for (i = 0; i < event->length; i++) Term_keypress(event->string[i]);

		/* All done */
		return (TRUE);
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch (event->keyval)
	{
		case GDK_Escape:
		{
			Term_keypress(ESCAPE);
			return (TRUE);
		}

		case GDK_Return:
		{
			Term_keypress('\r');
			return (TRUE);
		}

		case GDK_Tab:
		{
			Term_keypress('\t');
			return (TRUE);
		}

		case GDK_Delete:
		case GDK_BackSpace:
		{
			Term_keypress('\010');
			return (TRUE);
		}

		case GDK_Shift_L:
		case GDK_Shift_R:
		case GDK_Control_L:
		case GDK_Control_R:
		case GDK_Caps_Lock:
		case GDK_Shift_Lock:
		case GDK_Meta_L:
		case GDK_Meta_R:
		case GDK_Alt_L:
		case GDK_Alt_R:
		case GDK_Super_L:
		case GDK_Super_R:
		case GDK_Hyper_L:
		case GDK_Hyper_R:
		{
			/* Hack - do nothing to control characters */
			return (TRUE);
		}
	}

	/* Build the macro trigger string */
	strnfmt(msg, sizeof(msg), "%c%s%s%s%s_%X%c", 31,
	        mc ? "N" : "", ms ? "S" : "",
	        mo ? "O" : "", mx ? "M" : "",
	        event->keyval, 13);

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

	/* Hack -- auto-define macros as needed */
	if (event->length && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, event->string);
	}

	return (TRUE);
}


static gboolean expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	term_data *td = user_data;

	GdkPixmap *pixmap = g_object_get_data(G_OBJECT(widget), "pixmap");

	if (pixmap)
	{
		/* Restrict the area - Don't forget to reset it! XXX */
		/* gdk_gc_set_clip_rectangle(td->gc, &(event->area)); */

		g_assert(widget->window != 0);

		gdk_draw_drawable(widget->window, td->gc, pixmap,
		                event->area.x, event->area.y,
		                event->area.x, event->area.y,
		                event->area.width, event->area.height);
	}

	return (TRUE);
}


static void save_prefs(void)
{
	FILE *fff;
	int i;

	/* Open the settings file */
	fff = my_fopen(settings, "w");

	/* Oops */
	if (!fff) return;

	/* Header */
	fprintf(fff, "# %s GTK settings\n\n", VERSION_NAME);

	/* Number of term windows to open */
	fprintf(fff, "TERM_WINS=%d\n\n", num_term);

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		if (!td->t.mapped_flag) continue;

		/* Header */
		fprintf(fff, "# Term %d\n", i);

		/* Window specific location (x) */
		fprintf(fff, "AT_X_%d=%d\n", i, td->x_save);

		/* Window specific location (y) */
		fprintf(fff, "AT_Y_%d=%d\n", i, td->y_save);

		/* Window specific cols */
		fprintf(fff, "COLS_%d=%d\n", i, td->cols);

		/* Window specific rows */
		fprintf(fff, "ROWS_%d=%d\n", i, td->rows);

		/* Window specific font name */
		fprintf(fff, "FONT_%d=%s\n", i, td->lfont);

		if (i == 0)
		{
			/* Window specific small font name */
			fprintf(fff, "SFONT_%d=%s\n", i, td->sfont);
		}

		/* Footer */
		fprintf(fff, "\n");
	}
}


/*
 * Given a postion in the ISO Latin-1 character set, return
 * the correct character on this system.
 */
static char Term_xchar_gtk(char c)
{
	/* The GTK port uses the Latin-1 old standard */
	return (c);
}


static errr term_data_init(term_data *td, int i)
{
	term *t = &td->t;

	cptr font;
	cptr sfont;

	int x = 0;
	int y = 0;

	int cols;
	int rows;

	cptr str;

	int val;

	FILE *fff;

	char buf[1024];
	char cmd[40];
	char font_name[256];
	char sfont_name[256];

	int line = 0;


	cols = 80;
	rows = 25;

	/* Get default font for this term */
	font = get_default_font(i);
	sfont = get_default_small_font(i);

	/* Build the settings filename */
	path_build(settings, sizeof(settings), ANGBAND_DIR_USER, "gtk-settings.prf");

	/* Open the file */
	fff = my_fopen(settings, "r");

	/* File exists */
	if (fff)
	{
		/* Process the file */
		while (0 == my_fgets(fff, buf, sizeof(buf)))
		{
			/* Count lines */
			line++;

			/* Skip "empty" lines */
			if (!buf[0]) continue;

			/* Skip "blank" lines */
			if (isspace((unsigned char)buf[0])) continue;

			/* Skip comments */
			if (buf[0] == '#') continue;

			/* Window specific location (x) */
			sprintf(cmd, "AT_X_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				x = (str != NULL) ? atoi(str + 1) : -1;
				continue;
			}

			/* Window specific location (y) */
			sprintf(cmd, "AT_Y_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				y = (str != NULL) ? atoi(str + 1) : -1;
				continue;
			}

			/* Window specific cols */
			sprintf(cmd, "COLS_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) cols = val;
				continue;
			}

			/* Window specific rows */
			sprintf(cmd, "ROWS_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) rows = val;
				continue;
			}

			/* Window specific font name */
			sprintf(cmd, "FONT_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				if (str != NULL)
				{
					my_strcpy(font_name, str + 1, sizeof(font_name));
					font = font_name;
				}
				continue;
			}

			/* Window specific small font name */
			sprintf(cmd, "SFONT_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				if (str != NULL)
				{
					my_strcpy(sfont_name, str + 1, sizeof(sfont_name));
					sfont = sfont_name;
				}
				continue;
			}
		}

		/* Close */
		my_fclose(fff);
	}

	/* Font names must be stored in persistant strings */
	td->lfont = string_make(font);
	td->sfont = string_make(sfont);

	td->x_save = x;
	td->y_save = y;

 	td->cols = cols;
 	td->rows = rows;

	if (!i)
	{
		td->cols = MAX(cols, 80);
		td->rows = MAX(rows, 50);
	}

	/* Initialize the term */
	term_init(t, td->cols, td->rows, 1024);

	/* Store the name of the term */
	td->name = angband_term_name[i];

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	t->xtra_hook = Term_xtra_gtk;
	t->text_hook = Term_text_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->curs_hook = Term_curs_gtk;
	t->xchar_hook = Term_xchar_gtk;
	t->rows_hook = Term_rows_gtk;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/* Success */
	return (0);
}


static void init_gtk_window(term_data *td, int i)
{
	bool main_window = (i == 0) ? TRUE : FALSE;

	GtkWidget *menu_bar, *file_item, *file_menu, *box;
	GtkWidget *seperator_item, *file_exit_item, *file_new_item, *file_open_item;

	GdkGeometry hints;
	/* I can't get the size hints to work, so only set the resize hints */
	/* GdkWindowHints hmask = (GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE); */
	GdkWindowHints hmask = 0;

	int wwid, whgt;

	if (main_window)
	{
		/* Get the size of the small font ... */
		load_font(td, td->sfont);
		wwid = td->cols * td->font_wid;
		whgt = 50 * td->font_hgt;

		/* ... and the large one */
		load_font(td, td->lfont);
		if (wwid < td->cols * td->font_wid)
			wwid = td->cols * td->font_wid;
		if (whgt < 25 * td->font_hgt)
			whgt = 25 * td->font_hgt;

		/* Set hints */
		hints.min_width = wwid;
		hints.min_height = whgt ;
		hints.max_width = hints.min_width;
		hints.max_height = hints.min_height;

	}
	else
	{
		/* Get font size */
		load_font(td, td->lfont);
		wwid = td->cols * td->font_wid;
		whgt = td->rows * td->font_hgt;

		/* Set Hints */
		hints.min_width = td->font_wid;
		hints.min_height = td->font_hgt;
		hints.max_width = 255 * td->font_wid;
		hints.max_height = 255 * td->font_hgt;
		hmask |= GDK_HINT_RESIZE_INC;
		hints.width_inc = td->font_wid;
		hints.height_inc = td->font_hgt;
	}

	/* Create widgets */
	td->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	box = gtk_vbox_new(FALSE, 0);
	td->drawing_area = gtk_drawing_area_new();

	/* Set attributes */
	gtk_window_set_title(GTK_WINDOW(td->window), td->name);
	gtk_widget_set_size_request(td->drawing_area, wwid, whgt);

	/* Position window */
	gtk_window_move(GTK_WINDOW(td->window), td->x_save, td->y_save);

	/* I haven't been able to get the size hints to work correctly ... */
	gtk_window_set_geometry_hints(GTK_WINDOW(td->window), td->window, &hints, hmask);

	g_signal_connect(GTK_OBJECT(td->window), "delete_event", GTK_SIGNAL_FUNC(delete_event_handler), NULL);
	g_signal_connect(GTK_OBJECT(td->window), "key_press_event", GTK_SIGNAL_FUNC(keypress_event_handler), NULL);
	g_signal_connect(GTK_OBJECT(td->drawing_area), "expose_event", GTK_SIGNAL_FUNC(expose_event_handler), td);

	if (main_window)
		g_signal_connect(GTK_OBJECT(td->window), "destroy_event", GTK_SIGNAL_FUNC(destroy_event_handler), NULL);
	else
		g_signal_connect(GTK_OBJECT(td->window), "destroy_event", GTK_SIGNAL_FUNC(hide_event_handler), td);

	gtk_container_add(GTK_CONTAINER(td->window), box);

	/* Create main-menu */
	if (main_window)
	{
		/* Create the menu-bar and menu-items */
		menu_bar = gtk_menu_bar_new();
		file_item = gtk_menu_item_new_with_label("File");
		file_menu = gtk_menu_new();
		file_new_item = gtk_menu_item_new_with_label("New");
		file_open_item = gtk_menu_item_new_with_label("Open");
		seperator_item = gtk_menu_item_new();
		file_exit_item = gtk_menu_item_new_with_label("Exit");

		/* Register callbacks */
		g_signal_connect(GTK_OBJECT(file_exit_item), "activate", GTK_SIGNAL_FUNC(quit_event_handler), NULL);
		g_signal_connect(GTK_OBJECT(file_new_item), "activate", GTK_SIGNAL_FUNC(new_event_handler), NULL);
		g_signal_connect(GTK_OBJECT(file_open_item), "activate", GTK_SIGNAL_FUNC(open_event_handler), NULL);

		/* Build the menu bar */
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_new_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_open_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), seperator_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_exit_item);

		/* Pack the menu bar */
		gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, NO_PADDING);
	}

	/* Pack the display area */
	gtk_box_pack_start_defaults(GTK_BOX(box), td->drawing_area);

	/* Show the widgets */
	gtk_widget_show_all(td->window);

	/* Create a pixmap as buffer for screenupdates */
	td->pixmap = gdk_pixmap_new(td->drawing_area->window, td->cols * td->font_wid, td->rows * td->font_hgt, -1);
	g_object_set_data(G_OBJECT(td->drawing_area), "pixmap", td->pixmap);
	td->gc = gdk_gc_new(td->drawing_area->window);

	/* Clear the pixmap */
	gdk_draw_rectangle(td->pixmap, td->drawing_area->style->black_gc, TRUE,
	                   0, 0,
	                   td->cols * td->font_wid, td->rows * td->font_hgt);

	/* Copy it to the window */
	gdk_draw_drawable(td->drawing_area->window, td->gc, td->pixmap,
	                0, 0, 0, 0,
	                td->cols * td->font_wid, td->rows * td->font_hgt);
}


const char help_gtk[] =
	"GTK for X11, subopts -n<windows> and standard GTK options";



static void hook_quit(cptr str)
{
	save_prefs();

	(void)unregister_angband_fonts();

	gtk_exit(0);
}


/*
 * Initialization function
 */
errr init_gtk(int argc, char **argv)
{
	int i;
	GdkVisual *vis_sys;

	FILE *fff;

	char buf[1024];
	cptr str;
	int val;
	int line = 0;

	/*
	 * Check gtk-settings for the number of windows before handling
	 * command line options to allow for easy override
	 */

	/* Build the filename */
	path_build(settings, sizeof(settings), ANGBAND_DIR_USER, "gtk-settings.prf");

	/* Open the file */
	fff = my_fopen(settings, "r");

	/* File exists */
	if (fff)
	{
		/* Process the file */
		while (0 == my_fgets(fff, buf, sizeof(buf)))
		{
			/* Count lines */
			line++;

			/* Skip "empty" lines */
			if (!buf[0]) continue;

			/* Skip "blank" lines */
			if (isspace((unsigned char)buf[0])) continue;

			/* Skip comments */
			if (buf[0] == '#') continue;

			/* Number of terminal windows */
			if (prefix(buf, "TERM_WINS"))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) num_term = val;
				continue;
			}
		}

		/* Close */
		my_fclose(fff);
	}

	/* Initialize the environment */
	gtk_init(&argc, &argv);

	/* Make the new angband fonts available */
	(void)register_angband_fonts();

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}


	/* Test color depth */
	/* Number of colors = 2^depth: a depth of 8 means 256 colors */
	vis_sys = gdk_visual_get_system();
	if (vis_sys->depth >= 8)
	{
		/*
		 * Go no higher than 128, since the number of system colors
		 * could be too large compared to variable size.
		 */
		max_system_colors = 128;
	}
	/* Unusual color depths, check for completeness */
	else if (vis_sys->depth == 7)
	{
		max_system_colors = 128;
	}
	else if (vis_sys->depth == 6)
	{
		max_system_colors = 64;
	}
	else if (vis_sys->depth == 5)
	{
		max_system_colors = 32;
	}
	/* 16 colors */
	else if (vis_sys->depth == 4)
	{
		max_system_colors = 16;
	}
	/* Fewer that 16 colors not supported */
	else
	{
		quit("Sytems with fewer than 16 colors not supported!");
	}


	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;

		/* Init the window */
		init_gtk_window(td, i);
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Activate hooks */
	quit_aux = hook_quit;

	/* Set the system suffix */
	ANGBAND_SYS = "gtk";

	/* Catch nasty signals */
	signals_init();

	/* Initialize */
	init_angband();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 49, 17);
	Term_fresh();

	/* Processing loop */
	gtk_main();

	/* Stop now */
	quit(NULL);

	/* Success */
	return (0);
}

#endif /* USE_GTK */
