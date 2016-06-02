#ifndef INCLUDED_CWIN_H
#define INCLUDED_CWIN_H

#define smalloc(z) malloc(z,1)
#define snmalloc malloc
#define snewn(n, type) ((type *)snmalloc((n), sizeof(type)))
#define snew(type) ((type *)snmalloc(1, sizeof(type)))
//#define snewn(n, type) ((type *)snmalloc((n), sizeof(type)))
//#define sresize(ptr, n, type)                                           \
//    ((type *)snrealloc(sizeof((type *)0 == (ptr)) ? (ptr) : (ptr),      \
//                       (n), sizeof(type)))

typedef struct FontSpec {
	char *name;
	int isbold;
	int height;
	int width;
	int charset;
} FontSpec;
struct FontSpec *fontspec_new(const char *name,
	int bold, int height, int charset);

/*
* This structure is passed to event handler functions as the `dlg'
* parameter, and hence is passed back to winctrls access functions.
*/
struct dlgparam {
	HWND hwnd;			       /* the hwnd of the dialog box */
	struct winctrls *controltrees[8];  /* can have several of these */
	int nctrltrees;
	char *wintitle;		       /* title of actual window */
	char *errtitle;		       /* title of error sub-messageboxes */
	void *data;			       /* data to pass in refresh events */
	union control *focused, *lastfocused; /* which ctrl has focus now/before */
	char shortcuts[128];	       /* track which shortcuts in use */
	int coloursel_wanted;	       /* has an event handler asked for
								   * a colour selector? */
	struct { unsigned char r, g, b, ok; } coloursel_result;   /* 0-255 */
//	tree234 *privdata;		       /* stores per-control private data */
	int ended, endresult;	       /* has the dialog been ended? */
	int fixed_pitch_fonts;             /* are we constrained to fixed fonts? */
};

#endif