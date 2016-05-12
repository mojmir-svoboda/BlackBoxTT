/*
   tiny cache reader: first checks, if the file had already been read, if not,
   reads the file into a malloc'ed buffer, then for each line separates
   keyword from value, cuts off leading and trailing spaces, strlwr's the
   keyword and adds both to a list of below defined structures, where k is
   the offset to the start of the value-string. Comments or other non-keyword
   lines have a "" keyword and the line as value.

   added checking for external updates by the user.

   added *wild*card* processing, it first looks for an exact match, if it
   cant find any, returns the first wildcard value, that matches, or null,
   if none...
*/

#include "bbrc.h"
#include <platform_win.h>
#include "utils_string.h"
#include "utils.h"
#include "tinylist.h"
#include "iminmax.h"

//#include "bblib2.h"
//#include "win0x500.h"

enum { e_MAX_KEYWORD_LENGTH = 200 };
// #define DEBUG_READER

rcreader_init * g_rc;

void init_rcreader (rcreader_init * init)
{
    g_rc = init;
}

int found_last_value ()
{
    return g_rc->found_last_value;
}

bool set_translate_065 (int f)
{
    bool const r = g_rc->translate_065;
    g_rc->translate_065 = f;
    return r;
}

lin_list * search_line (fil_list * fl, TCHAR const * key, int fwild, LONG * p_seekpos);

int translate_key070 (TCHAR * key)
{
    const TCHAR * const checklist [] = {
        TEXT("^toolbar")       ,
        TEXT("^slit")          ,
        // toolbar.*:
        TEXT("windowlabel")   ,
        TEXT("clock")         ,
        TEXT("label")         ,
        TEXT("button")        ,
        TEXT("pressed")       ,
        // menu.*:
        TEXT("frame")         ,
        TEXT("title")         ,
        TEXT("active")        ,
        // window.*:
        TEXT("focus")         ,
        TEXT("unfocus")       ,
        NULL,
        // from         -->   to
        TEXT("color")         ,  TEXT("color1")              ,
        TEXT("colorto")       ,  TEXT("color2")              ,
        TEXT("piccolor")      ,  TEXT("foregroundColor")     ,
        TEXT("bulletcolor")   ,  TEXT("foregroundColor")     ,
        TEXT("disablecolor")  ,  TEXT("disabledColor")       ,
        TEXT("justify")       ,  TEXT("alignment")           ,

        TEXT("^handlewidth")  ,  TEXT("window.handleHeight") ,
        TEXT("^borderwidth")  ,  TEXT("toolbar.borderWidth") ,
        TEXT("^bordercolor")  ,  TEXT("toolbar.borderColor") ,
        TEXT("^bevelwidth")   ,  TEXT("toolbar.marginWidth") ,
        TEXT("^frameWidth")   ,  TEXT("window.frame.borderWidth"),
        TEXT("focusColor")    ,  TEXT("focus.borderColor")     ,
        TEXT("unfocusColor")  ,  TEXT("unfocus.borderColor")   ,
    /*
        TEXT("titleJustify")  ,  TEXT("menu.title.alignment")   ,
        TEXT("menuJustify")   ,  TEXT("menu.frame.alignment")   ,
    */
        NULL
    };

    TCHAR const * const * pp, *p;
    TCHAR * d;
    int l, n, x, k, f, r = 0;

    l = (int)(_tcslen(key));
    if (0 == l)
        return 0;

    if (key[l-1] == ':')
        key[--l] = 0;

    _tcslwr(key);
    if (NULL != (d = _tcsstr(key, TEXT("hilite"))))
        tmemcpy(d, TEXT("active"), 6), r = 1;

    for (pp = checklist, x = 0; x++ < 2; ++pp)
    {
        for (n = 0; 0 != (p = *pp); ++n, pp += x) {
            f = p[0] == '^';
            k = (int)(_tcslen(p += f));
            d = key+l-k;
            if (!(f ? d == key : d > key && d[-1] == '.'))
                continue;
            if (0 != tmemcmp(d, p, k))
                continue;
            if (x == 1)
                _tcscpy(key+l, TEXT(".appearance"));
            else
                _tcscpy(d, pp[1]);
            return 1 + (x==2 && n==0);
        }
    }
    return r;
}

// This one converts all keys in a style from 065 to 070 style conventions
void make_style070 (fil_list * fl)
{
//     lin_list * tl, ** tlp, * sl, * ol;
//     TCHAR buffer[e_MAX_KEYWORD_LENGTH], *p;
//     int f;
//     for (tlp = &fl->lines; NULL != (tl = *tlp); tlp = &tl->m_next)
//     {
//         if (0 == tl->str[0])
//             continue;
//         tmemcpy(buffer, tl->str + tl->o, tl->k);
//         f = translate_key070(buffer);
//         if (f) {
//             for (ol = tl;;) {
//                 sl = make_line(fl, buffer, tl->str+tl->k);
//                 //dbg_printf(TEXT("%s -> %s"), tl->str, sl->str);
//                 sl->m_next = tl->m_next;
//                 tl->m_next = sl;
//                 tl = sl;
//                 if (0 == --f)
//                     break;
//                 // since I dont know (and dont want to check here)
//                 // whether its solid or gradient, I just translate
//                 // 'color' to both 'color1' and 'backgroundColor'
//                 p = _tcschr(buffer, 0) - (sizeof TEXT("color1") - 1);
//                 _tcscpy(p, TEXT("backgroundColor"));
//             }
//             *tlp = ol->m_next;
//             free_line(fl, ol);
//         }
//     }
}

/* ------------------------------------------------------------------------- */
bool translate_key065 (TCHAR * key)
{
    TCHAR const * pairs[] =
    {
        // from               -->   to
        TEXT(".appearance")       , TEXT("")                ,
        TEXT("alignment")         , TEXT("justify")         ,
        TEXT("color1")            , TEXT("color")           ,
        TEXT("color2")            , TEXT("colorTo")         ,
        TEXT("backgroundColor")   , TEXT("color")           ,
        TEXT("foregroundColor")   , TEXT("picColor")        ,
        TEXT("disabledColor")     , TEXT("disableColor")    ,
        TEXT("menu.active")       , TEXT("menu.hilite")     ,
        TEXT("window.handleHeight"),TEXT("handleWidth")     ,
        NULL
    };
    TCHAR const **p = pairs;
    bool ret = false;
    size_t k = 0;
    do
    {
        TCHAR * q = stristr(key, *p);
        if (q)
        {
            size_t lp = _tcslen(p[0]);
            size_t lq = _tcslen(q);
            size_t lr = _tcslen(p[1]);
            size_t k0 = k + lr - lp;
            tmemmove(q + lr, q + lp, lq - lp + 1);
            tmemmove(q, p[1], lr);
            k = k0;
            ret = true;
        }
    } while ((p += 2)[0]);
    return ret;
}

// This one converts all keys in a style from 070 to 065 style conventions
void make_style065 (fil_list *fl)
{
//     lin_list *tl = 0, **tlp = 0, *ol = 0;
//     TCHAR buffer[1024];
//     int f;
//     for (tlp = &fl->lines; NULL != (tl = *tlp); tlp = &tl->m_next)
//     {
//         if (0 == tl->str[0])
//             continue;
//         tmemcpy(buffer, tl->str+tl->o, tl->k);
//         f = translate_key065(buffer);
//         if (f) {
//             ol = tl;
//             *tlp = tl = make_line(fl, buffer, tl->str+tl->k);
//             tl->m_next = ol->m_next;
//             free_line(fl, ol);
//         }
//     }
}

// this one tries to satisfy a plugin that queries an 0.65 item
// from a style that has 0.70 syntax
lin_list * search_line_065 (fil_list * fl, TCHAR const * key)
{
    TCHAR buff[e_MAX_KEYWORD_LENGTH];
    int f;
    lin_list * tl = 0;
    TCHAR * d = 0;

	strcpy_max(buff, key, array_count(buff));
    f = translate_key070(buff);
    if (0 == f)
        return NULL;
    if (2 == f) {
        // we need to translate ".color" to "color1" for gradients
        // but to "backgroundColor" for solid (unless interlaced)
        d = _tcschr(buff, 0) - (sizeof TEXT(".color1") - 1);
        _tcscpy(d, TEXT(".appearance"));
        tl = search_line(fl, buff, true, NULL);
        if (tl && (stristr(tl->str+tl->k, TEXT("gradient"))
                || stristr(tl->str+tl->k, TEXT("interlaced"))))
            _tcscpy(d, TEXT(".color1"));
        else
            _tcscpy(d, TEXT(".backgroundColor"));
    }
    // dbg_printf("%s -> %s", key, buff);
    return search_line(fl, buff, true, NULL);
}

bool get_070 (TCHAR const * path)
{
	//return read_file(path)->is_070;
	return true;
}

void check_070 (fil_list * fl)
{
    lin_list * tl = 0;
    dolist(tl, fl->lines)
        if (tl->k > 11 && 0 == tmemcmp(tl->str+tl->k-11, TEXT("appearance"), 10))
            break;
    fl->is_070 = NULL != tl;
    dolist (tl, fl->lines)
        if (stristr(tl->str+tl->k, TEXT("gradient"))
         || stristr(tl->str+tl->k, TEXT("solid")))
            break;
    fl->is_style = NULL != tl;
}

bool is_stylefile (TCHAR const * path)
{
    TCHAR * temp = read_file_into_buffer(path, 10000);
    bool r = false;
    if (temp) {
        r = NULL != _tcsstr(_tcslwr(temp), TEXT("menu.frame"));
        m_free(temp);
    }
    return r;
}

/* ------------------------------------------------------------------------- */
FILE * create_rcfile (TCHAR const * path)
{
    FILE * fp = 0;
    //dbg_printf("writing to %s", path);
    if (NULL == (fp = _tfopen(path, g_rc->dos_eol ? TEXT("wt") : TEXT("wb")))) {
        if (g_rc->write_error)
            g_rc->write_error(path);
    }
    return fp;
}

void write_rcfile (fil_list * fl)
{
    FILE * fp = 0;
    unsigned ml = 0;
    lin_list * tl = 0;

#ifdef DEBUG_READER
    dbg_printf("writing file %s", fl->path);
#endif
    if (NULL == (fp = create_rcfile(fl->path)))
        return;

    if (fl->tabify) {
        // calculate the max. keyword length
        dolist (tl, fl->lines)
            if (tl->k > ml)
                ml = tl->k;
        ml = (ml+4) & ~3; // round up to the next tabstop
    }

    dolist (tl, fl->lines) {
        TCHAR const * s = tl->str + tl->k;
        if (0 == *tl->str)
            fprintf (fp, "%s\n", s); //comment
        else
            fprintf(fp, "%s:%*s%s\n", tl->str+tl->o, imax(1, ml - tl->k), "", s);
    }

    fclose(fp);
    fl->dirty = false;
}

void mark_rc_dirty (fil_list * fl)
{
    fl->dirty = true;
}

/* ------------------------------------------------------------------------- */
void delete_lin_list (fil_list * fl)
{
    freeall(&fl->lines);
    memset(fl->ht, 0, sizeof fl->ht);
    fl->wild = NULL;
}

void delete_fil_list (fil_list * fl)
{
    if (fl->dirty)
        write_rcfile(fl);
    delete_lin_list(fl);
    remove_item(&g_rc->rc_files, fl);
}

void reset_rcreader ()
{
    while (g_rc->rc_files)
        delete_fil_list(g_rc->rc_files);
#ifdef DEBUG_READER
    dbg_printf("RESET READER");
#endif
}

/* ------------------------------------------------------------------------- */
VOID CALLBACK reset_reader_proc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (g_rc) {
        if (g_rc->used) {
            g_rc->used = 0;
            return;
        }
        // @NOTE: first we remove the timer, because inside the reset_rcread an error can cause popup, which will in turn call reset_reader_proc... voila infinite recursion
        KillTimer(hwnd, idEvent);
        reset_rcreader();
        g_rc->timer_set = 0;
    }
    else
    {
        // dbg_printf("reset_reader %x %x %x %d", hwnd, uMsg, idEvent, dwTime);
        KillTimer(hwnd, idEvent);
    }
}

void set_reader_timer ()
{
    if (g_rc->timer_set)
        return;
    // dbg_printf("set_reader_timer");
    SetTimer(NULL, 0, 10, reset_reader_proc);
    g_rc->timer_set = 1;
}

/* ------------------------------------------------------------------------- */
// helpers

TCHAR * read_file_into_buffer (TCHAR const * path, int max_len)
{
    FILE * fp = 0;
    if (NULL == (fp = _tfopen(path, TEXT("rb"))))
        return NULL;

    fseek(fp,0,SEEK_END);
    int len = ftell (fp);
    fseek (fp,0,SEEK_SET);
    if (max_len && len >= max_len)
        len = max_len-1;

    TCHAR * buf = (TCHAR*)m_alloc(len+1);
    fread(buf, 1, len, fp);
    fclose(fp);

    buf[len] = 0;
    return buf;
}

// Scan one line in a buffer, advance read pointer, set start pointer
// and length for the caller. Returns first non-space char.
TCHAR scan_line (TCHAR * & pp, TCHAR * & ss, size_t & ll)
{
    TCHAR c, e, *d, *s, *p;
    for (p = pp; c = *p, IS_SPC(c) && 10 != c && c; p++)
        ;
    //find end of line, replace tabs with spaces
    for (s = p; 0 != (e = *p) && 10 != e; p++)
        if (e == 9)
            *p = ' ';
    //cut off trailing spaces
    for (d = p; d > s && IS_SPC(d[-1]); d--)
        ;
    //ready for next line
    *d = 0;
    pp = p + (10 == e);
    ss = s;
    ll = d - s;
    return c;
}

/* ------------------------------------------------------------------------- */
// XrmResource fake wildcard pattern matcher
// -----------------------------------------
// returns: 0 for no match, else a number that is somehow a measure for
// 'how much' the item matches. Btw 'toolbar*color' means just the
// same as 'toolbar.*.color' and both match e.g. 'toolbar.color'
// as well as 'toolbar.label.color', 'toolbar.button.pressed.color', ...

// this scans one group in a keyword, that is the portion between
// dots, may be literal or '*' or '?'

int scan_component (TCHAR const ** p)
{
    TCHAR const *s; TCHAR c; int n;
    for (s=*p, n=0; 0 != (c = *s); ++s, ++n)
    {
        if (c == '*' || c == '?') {
            if (n)
                break;
            do {
                c = *++s;
            } while (c == '.' || c == '*' || c == '?');
            n = 1;
            break;
        }
        if (c == '.') {
            do {
                c = *++s;
            } while (c == '.');
            break;
        }
    }
    //dbg_printf("scan_component: %d %.*s", n, n, *p);
    *p = s;
    return n;
}

int xrm_match (TCHAR const * key, TCHAR const * pat)
{
    TCHAR const * pp = 0, * kk = 0;
    int n, k, p;
    for (int c = 256, m = 0; ; key = kk, c /= 2)
    {
        kk = key, k = scan_component(&kk);
        pp = pat, p = scan_component(&pp);
        if (0==k)
            return 0==p ? m : 0;
        if (0==p)
            return 0;
        if ('*' == *pat) {
            n = xrm_match(key, pp);
            if (n)
                return m + n * c/384;
            continue;
        }
        if ('?' != *pat) {
            if (k != p || 0 != tmemcmp(key, pat, k))
                return 0;
            m+=c;
        }
        pat=pp;
    }
}

/* ------------------------------------------------------------------------- */
lin_list * make_line (fil_list * fl, TCHAR const * key, TCHAR const * val)
{
    TCHAR buffer[e_MAX_KEYWORD_LENGTH];
    size_t k = 0;
	bb_hash_t h = 0;

    int v = (int)_tcslen(val);
    if (key)
		h = lower_key_and_calc_hash(buffer, e_MAX_KEYWORD_LENGTH, key, k, _T(':'));

    lin_list * tl = (lin_list *)c_alloc(sizeof(lin_list) + k*2 + v);
    tl->hash = h;
    tl->k = k + 1;
    tl->o = k + v + 2;
    if (k)
    {
        tmemcpy(tl->str, buffer, k);
        tmemcpy(tl->str + tl->o, key, k);
    }
    tmemcpy(tl->str+tl->k, val, v);

    //if the key contains a wildcard
    if (k && (tmemchr(key, '*', k) || tmemchr(key, '?', k)))
    {
        // add it to the wildcard - list
        tl->wnext = fl->wild;
        fl->wild = tl;
        tl->is_wild = true;
    }
    else
    {
        // link it in the hash bucket
        lin_list ** tlp = &fl->ht[tl->hash % e_RCFILE_HTS];
        tl->hnext = *tlp;
        *tlp = tl;
    }
    return tl;
}

/*void del_from_list (void * tlp, void * tl, void * n)
{
    void * v;
    size_t o = (char*)n - (char*)tl;
    while (NULL != (v = *(void**)tlp))
    {
        void **np = (void **)((char *)v+o);
        if (v == tl)
        {
            *(void**)tlp = *np;
            break;
        }
        tlp = np;
    }
}

void free_line (fil_list * fl, lin_list * tl)
{
    if (tl->is_wild)
        del_from_list(&fl->wild, tl, &tl->wnext);
    else
        del_from_list(&fl->ht[tl->hash%RCFILE_HTS], tl, &tl->hnext);
    m_free(tl);
}*/

lin_list * search_line (fil_list * fl, TCHAR const * key, int fwild, LONG * p_seekpos)
{
    TCHAR buff[e_MAX_KEYWORD_LENGTH];
    lin_list * tl = 0;

    size_t key_len = 0;
	bb_hash_t const h = lower_key_and_calc_hash(buff, e_MAX_KEYWORD_LENGTH, key, key_len, _T(':'));
    if (0 == key_len)
        return NULL;

    ++key_len; // check terminating \0 too

    if (p_seekpos)
    {
        long seekpos = *p_seekpos;
        int n = 0;
        dolist (tl, fl->lines)
            if (++n > seekpos && tl->hash == h && 0==memcmp(tl->str, buff, key_len)) {
                *p_seekpos = n;
                break;
            }
        return tl;
    }

    // search hashbucket
    for (tl = fl->ht[h % e_RCFILE_HTS]; tl; tl = tl->hnext)
        if (0 == tmemcmp(tl->str, buff, key_len))
            return tl;

    if (fwild)
    {
        // search wildcards
        int best_match = 0;
        for (lin_list * wl = fl->wild; wl; wl = wl->wnext)
        {
            int n = xrm_match(buff, wl->str);
            //dbg_printf("match:%d <%s> <%s>", n, buff, sl->str);
            if (n > best_match)
                tl = wl, best_match = n;
        }
    }
    return tl;
}

void translate_new (TCHAR * buffer, size_t bufsize, TCHAR ** pkey, size_t & pklen, int syntax)
{
    TCHAR const * pairs [] =
    {
        // from         -->   to [OB -> 0.65 fork] 
        TEXT("padding.width")                 , TEXT("bevelWidth")     ,
        TEXT("menu.border.width")             , TEXT("menu.*.borderWidth")     ,
        TEXT("border.width")                  , TEXT("borderWidth")     ,
        TEXT("handle.width")                  , TEXT("handleWidth")     ,
        TEXT("menu.border.color")             , TEXT("menu.*.borderColor")     ,
        TEXT("border.color")                  , TEXT("borderColor")     ,
        TEXT("label.text.justify")            , TEXT("justify")                ,
        TEXT(".bg")                           , TEXT("")                ,
        TEXT("title.text.font")               , TEXT("title.font")                ,
        TEXT("items.font")                    , TEXT("frame.font")                ,
        TEXT("items.active")                  , TEXT("hilite")                ,
        TEXT("items")                         , TEXT("frame")                ,
        TEXT("disabled.text.color")           , TEXT("disableColor")                ,
        TEXT("active.label.text.font")        , TEXT("font")                ,
        TEXT(".active.title")                 , TEXT(".title.focus")                ,
        TEXT(".active.label")                 , TEXT(".label.focus")                ,
        TEXT(".active.handle")                , TEXT(".handle.focus")                ,
        TEXT(".active.grip")                  , TEXT(".grip.focus")                ,
        TEXT("window.active.button.*")        , TEXT("window.button.focus")        ,
        TEXT("window.inactive.button.*")      , TEXT("window.button.unfocus")     ,
        TEXT(".active.button.unpressed")      , TEXT(".button.focus")                ,
        TEXT(".active.button.pressed")        , TEXT(".button.pressed")            ,
        TEXT("inactive.title")                , TEXT("title.unfocus")                ,
        TEXT("inactive.label")                , TEXT("label.unfocus")                ,
        TEXT("inactive.handle")               , TEXT("handle.unfocus")                ,
        TEXT("inactive.grip")                 , TEXT("grip.unfocus")                ,
        TEXT("inactive.button.unpressed")     , TEXT("button.unfocus")                ,
        TEXT("text.justify")                  , TEXT("justify")                ,
        TEXT("text.color")                    , TEXT("textColor")                ,
        TEXT("image.color")                   , TEXT("picColor")             ,
        TEXT("osd")                           , TEXT("toolbar")     ,
        TEXT("unhighlight")                   , TEXT("button")     ,
        TEXT("highlight")                     , TEXT("button.pressed")     ,
        // frame values
        TEXT("window.client.padding.width")   , TEXT("window.frame.borderWidth")     ,
        TEXT("window.active.border.color")    , TEXT("window.*.focus.borderColor")  ,
        TEXT("window.inactive.border.color")  , TEXT("window.*.unfocus.borderColor") ,
        TEXT("active.client.color")           , TEXT("frame.focus.borderColor")     ,
        TEXT("inactive.client.color")         , TEXT("frame.unfocus.borderColor")   ,
        TEXT("frameColor")                    , TEXT("frame.focus.borderColor")     ,
       NULL
    };

    TCHAR const * FBpairs [] =    // older items
    {
        TEXT("window.frame.focusColor")       , TEXT("window.frame.focus.borderColor") ,
        TEXT("window.frame.unfocusColor")     , TEXT("window.frame.unfocus.borderColor") ,
        TEXT("window.frame.focus.color")      , TEXT("window.frame.focus.borderColor") ,
        TEXT("window.frame.unfocus.color")    , TEXT("window.frame.unfocus.borderColor") ,
        TEXT("frameWidth")                    , TEXT("window.frame.borderWidth")     ,
        TEXT("window.frameWidth")             , TEXT("window.frame.borderWidth") ,
      NULL
    };

    TCHAR const ** p = pairs;
    size_t k = pklen;
    if (k >= bufsize) return;
    *pkey = tmemcpy(buffer, *pkey, k);
    buffer[k] = 0;
    if (syntax == 1)
        p = FBpairs;
    do
    {
        TCHAR * q = stristr(buffer, *p);
        if (q)
        {
            size_t lp = _tcslen(p[0]);
            size_t lq = _tcslen(q);
            size_t lr = _tcslen(p[1]);
            size_t k0 = k + lr - lp;
            if (k0 >= bufsize) break;
            tmemmove(q + lr, q + lp, lq - lp + 1);
            tmemmove(q, p[1], lr);
            k = k0;
        }
    }
    while ((p += 2)[0]);

    pklen = k;
}

/* ------------------------------------------------------------------------- */
// searches for the filename and, if not found, builds a _new line-list

fil_list * read_file (TCHAR const * filename)
{
    lin_list ** slp = 0, *sl = 0;
    fil_list ** flp = 0, *fl = 0;
	TCHAR *buf = 0, *p = 0, *d = 0, *s = 0, *t = 0, *hilite = 0, c = 0;
	TCHAR hashname[MAX_PATH];
	TCHAR buff[e_MAX_KEYWORD_LENGTH];
    int is_OB, is_070;
	size_t k;

    // ----------------------------------------------
    // first check, if the file has already been read
	bb_hash_t h = lower_key_and_calc_hash(hashname, MAX_PATH, filename, k, _T('\0'));
    k = k + 1;
    for (flp = &g_rc->rc_files; NULL!=(fl=*flp); flp = &fl->m_next)
        if (fl->hash == h && 0 == tmemcmp(hashname, fl->path+fl->k, k))
        {
            ++g_rc->used;
            return fl; //... return cached line list.
        }

    // allocate a _new file structure, the filename is
    // stored twice, as is and strlwr'd for compare.
    fl = (fil_list *)c_alloc(sizeof(*fl) + k * 2);
    tmemcpy(fl->path, filename, k);
    tmemcpy(fl->path + k, hashname, k);
    fl->k = k;
    fl->hash = h;
    cons_node(&g_rc->rc_files, fl);

#ifdef DEBUG_READER
    dbg_printf("reading file %s", fl->path);
#endif
    set_reader_timer();

    buf = read_file_into_buffer(fl->path, 0);
    if (NULL == buf)
    {
        fl->newfile = true;
        return fl;
    }

    is_OB = is_070 = false;
    if(stristr(buf, TEXT("bg:")))
        is_OB = true;
    else if (stristr(buf, TEXT("appearance:")))
        is_070 = true;

    for (slp = &fl->lines, p = buf;;)
    {
        c = scan_line(p, s, k);
        if (0 == c)
            break;
        if (0 == k || c == '#' || c == '!') {
comment:
            // empty line or comment
            sl = make_line(fl, NULL, s);

        } else {
            d = tmemchr(s, ':', k);
            if (NULL == d)
                goto comment;
            for (t = d; t > s && IS_SPC(t[-1]); --t)
                ;
            *t = 0;
            if (t - s >= e_MAX_KEYWORD_LENGTH)
                goto comment;
            // skip spaces between key and value
            while (*++d == ' ')
                ;

            if (k && is_OB)
                translate_new(buff, sizeof buff, &s, k, 0);
            else 
            if (k && false == is_070)
                translate_new(buff, sizeof buff, &s, k, 1);

            // mojmir: i have no idea how did i break that shit... so this is a hotfix
            // the thing is that filelist has menu.hilite items from file,
            // while the read_style reads menu.active.
            if (is_070)
                if (NULL != (hilite = _tcsstr(s, TEXT("hilite"))))
                    tmemcpy(hilite, TEXT("active"), 6);

            sl = make_line(fl, s, d);
        }
        //append it to the list
        slp = &(*slp=sl)->m_next;
    }
    m_free(buf);
    check_070(fl);
    return fl;
}

/* ------------------------------------------------------------------------- */
// Purpose: Searches the given file for the supplied keyword and returns a
// pointer to the value - string
// In: TCHAR const * path = String containing the name of the file to be opened
// In: TCHAR const * szKey = String containing the keyword to be looked for
// In: LONG ptr: optional: an index into the file to start search.
// Out: TCHAR const * = Pointer to the value string of the keyword

TCHAR const * read_value (TCHAR const * path, TCHAR const * szKey, long * ptr)
{
    fil_list * fl = read_file(path);
    lin_list * tl = search_line(fl, szKey, true, ptr);

    if (NULL == tl && fl->is_style && g_rc->translate_065)
        tl = search_line_065(fl, szKey);

    TCHAR const * r = NULL;
    if (tl)
        r = tl->str + tl->k;

    g_rc->found_last_value = tl ? (tl->is_wild ? 2 : 1) : 0;

#ifdef DEBUG_READER
    { static int rcc; dbg_printf("read_value %d %s:%s <%s>", ++rcc, path, szKey, r); }
#endif
    return r;
}

/* ------------------------------------------------------------------------- */
// Find out a good place where to put an item if it was not yet found in the
// rc-file previously.

int simkey (TCHAR const * a0, TCHAR const * b0)
{
    TCHAR const * a = a0, *b = b0, *aa = 0, *bb = 0;
    int ca, cb, na, nb, m, f, e;
    for (f = e = m = na = nb = 0; ; )
    {
        aa = a, ca = scan_component(&a);
        bb = b, cb = scan_component(&b);
        if (0 == ca && 0 == cb)
            break;
        if (ca) ++na;
        if (cb) ++nb;
        if (0 == ca || 0 == cb || f)
            continue;
        if (ca == cb && 0 == tmemcmp(aa, bb, ca)) {
            m ++;
            e = 0;
        } else {
            f = 1;
            e = 0 == tmemcmp(aa, bb, 4);
        }
    }
    f = 2*m + e * (m && na == nb);
    //dbg_printf("sim %d <%s> <%s>", f, a0, b0);
    return f;
}

lin_list ** get_simkey (lin_list ** slp, TCHAR const * key)
{
    lin_list ** tlp = NULL, *sl = 0;
    int m = 1;
    int i = 0, k = 0;
    for (; NULL!=(sl=*slp); slp = &sl->m_next)
    {
        if (0 == sl->str[0])
            continue;
        int const n = simkey(sl->str, key);
        if (n != m)
            i = 0;
        if (n < m)
            continue;
        ++i;
        if (n > m || i > k)
        {
            m = n;
            k = i;
            tlp = &sl->m_next;
        }
    }
    return tlp;
}

/* ------------------------------------------------------------------------- */
// Search for the szKey in the file_list, replace, if found and the value
// did change, or append, if not found. Write to file on changes.

void write_value (TCHAR const * path, TCHAR const * szKey, TCHAR const * value)
{
// #ifdef DEBUG_READER
//     dbg_printf("write_value <%s> <%s> <%s>", path, szKey, value);
// #endif
// 
//     fil_list * fl = read_file(path);
//     lin_list * tl = search_line(fl, szKey, false, NULL);
// 
//     if (tl && value && 0 == _tcscmp(tl->str + tl->k, value))
//     {
//         // nothing changed
//         if (tmemcmp(tl->str + tl->o, szKey, tl->k-1)) {
//             // make shure that keyword has correct letter case
//             tmemcpy(tl->str + tl->o, szKey, tl->k-1);
//             mark_rc_dirty(fl);
//         }
//         tl->dirty = 1;
//     }
//     else 
//     {
//         lin_list ** tlp = 0;
//         for (tlp = &fl->lines; *tlp != tl; tlp = &(*tlp)->m_next)
//             ;
//         if (tl)
//         {
//             *tlp = tl->m_next;
//             free_line(fl, tl);
//         }
//         if (value)
//         {
//             lin_list *sl = 0, **slp = 0;
//             sl = make_line(fl, szKey, value);
//             sl->dirty = true;
//             if (NULL == tl && false == fl->newfile)
//             {
//                 // insert a new item below a similar one
//                 slp = get_simkey(&fl->lines, sl->str);
//                 if (slp) tlp = slp;
//             }
//             sl->m_next = *tlp;
//             *tlp = sl;
//         }
//         mark_rc_dirty(fl);
//     }
}

/* ------------------------------------------------------------------------- */

int rename_setting (TCHAR const * path, TCHAR const * szKey, TCHAR const * new_keyword)
{
//     TCHAR buff[e_MAX_KEYWORD_LENGTH];
//     lin_list *sl = 0, *tl = 0;
//     int dirty = 0;
// 	size_t k = 0;
// 	/*bb_hash_t h = */lower_key_and_calc_hash(buff, e_MAX_KEYWORD_LENGTH, szKey, k, _T(':'));
//     if (0 == k)
//         return false;
// 
//     fil_list * fl = read_file(path);
//     for (lin_list ** slp = &fl->lines; NULL != (sl = *slp); )
//     {
//         if (new_keyword)
//         {
//             if ((int)sl->k == 1+k && 0 == tmemcmp(sl->str, buff, k))
//             {
//                 tl = make_line(fl, new_keyword, sl->str+sl->k);
//                 tl->m_next = sl->m_next;
//                 *slp = tl;
//                 slp = &tl->m_next;
//                 free_line(fl, sl);
//                 ++dirty;
//                 continue;
//             }
//         }
//         else
//         {
//             if ((1 == k && '*' == buff[0]) || xrm_match(sl->str, buff))
//             {
//                 *slp = sl->m_next;
//                 free_line(fl, sl);
//                 ++dirty;
//                 continue;
//             }
//         }
//         slp = &sl->m_next;
//     }
//     if (dirty)
//         mark_rc_dirty(fl);
//     return 0 != dirty;
	return 0;
}

/* ------------------------------------------------------------------------- */

int delete_setting (LPTSTR path, LPTSTR szKey)
{
#ifdef DEBUG_READER
    dbg_printf("delete_setting <%s> <%s>", path, szKey);
#endif
    return rename_setting(path, szKey, NULL);
}

/* ------------------------------------------------------------------------- */
int read_next_line (FILE * fp, TCHAR * szBuffer, unsigned dwLength)
{
    if (fp && _fgetts(szBuffer, dwLength, fp))
    {
        TCHAR const * p = szBuffer;
        TCHAR * q = szBuffer, c;
        skip_spc(p);
        while (0 != (c = *p))
            *q++ = IS_SPC(c) ? ' ' : c, p++;
        while (q > szBuffer && IS_SPC(q[-1]))
            q--;
        *q = 0;
        return true;
    }
    szBuffer[0] = 0;
    return false;
}

/* ------------------------------------------------------------------------- */

