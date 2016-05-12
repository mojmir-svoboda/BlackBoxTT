#pragma once
#include <cstdio>
#include <platform_win.h>
#include <tchar.h>
//#include "bblibapi.h"

enum { e_RCFILE_HTS = 40 }; // hash table size

struct lin_list
{
    lin_list * m_next;
    lin_list * hnext;
    lin_list * wnext;
    unsigned hash, k, o;
    int i;
    bool is_wild;
    bool dirty;
    //char flags;
    TCHAR str[3];
};

struct fil_list
{
    fil_list * m_next;
    lin_list * lines;
    lin_list * wild;
    lin_list * ht[e_RCFILE_HTS];
    unsigned hash;

    bool dirty;
    bool newfile;
    bool tabify;
    bool write_error;
    bool is_style;
    bool is_070;

    int k;
    TCHAR path[1];
};

struct rcreader_init
{
    fil_list * rc_files;
    void (*write_error) (TCHAR const * filename);
    bool dos_eol;
    bool translate_065;
    bool timer_set;
    int used;
    int found_last_value;
};

void init_rcreader (rcreader_init * init);
void reset_rcreader ();

bool set_translate_065 (int f);
// check whether a style uses 0.70 conventions
bool get_070 (TCHAR const * path);
void check_070 (fil_list *fl);
bool is_stylefile(TCHAR const * path);

FILE * create_rcfile (TCHAR const * path);
TCHAR * read_file_into_buffer (TCHAR const * path, int max_len);
TCHAR scan_line(TCHAR ** pp, TCHAR ** ss, int * ll);
int read_next_line (FILE * fp, TCHAR * szBuffer, unsigned dwLength);

TCHAR const * read_value (TCHAR const * path, TCHAR const * szKey, long * ptr);
int found_last_value ();
void write_value (TCHAR const * path, TCHAR const * szKey, TCHAR const * value);
int rename_setting (TCHAR const * path, TCHAR const * szKey, TCHAR const * new_keyword);
int delete_setting (LPTSTR path, LPTSTR szKey);
// 
// /* ------------------------------------------------------------------------- */
// /* only used in bbstylemaker */
// 
// API_EXPORT int scan_component (TCHAR const ** p);
// API_EXPORT int xrm_match (TCHAR const * key, TCHAR const * pat);
// 
// API_EXPORT fil_list * read_file (const TCHAR * filename);
// API_EXPORT lin_list * make_line (fil_list * fl, TCHAR const * key, TCHAR const * val);
// API_EXPORT void free_line (fil_list * fl, lin_list * tl);
// API_EXPORT lin_list ** get_simkey (lin_list ** slp, TCHAR const * key);
// API_EXPORT void make_style070 (fil_list * fl);
// API_EXPORT void make_style065 (fil_list * fl);

