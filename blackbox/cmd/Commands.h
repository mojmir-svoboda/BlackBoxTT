#pragma once

/*=========================================================================== */
/* Blackbox messages */

#define BB_REGISTERMESSAGE      10001
#define BB_UNREGISTERMESSAGE    10002

/* ----------------------------------- */
#define BB_QUIT                 10101 /* lParam 0=ask/1=quiet */
#define BB_RESTART              10102
#define BB_RECONFIGURE          10103
#define BB_SETSTYLE             10104 /* lParam: const char* stylefile */

#define BB_EXITTYPE             10105 /* For plugins: receive only */
/* lParam values for BB_EXITTYPE: */
# define B_SHUTDOWN                 0 /* Shutdown/Reboot/Logoff */
# define B_QUIT                     1
# define B_RESTART                  2

#define BB_TOOLBARUPDATE        10106 /* toolbar changed position/size */
#define BB_SETTHEME             10107 /* xoblite */

/* ----------------------------------- */
#define BB_EDITFILE             10201
/* wParam values for BB_EDITFILE:
   0 = Current style,
   1 = menu.rc,
   2 = plugins.rc
   3 = extensions.rc
   4 = blackbox.rc
   -1 = filename in (const char*)lParam */

/* Send a command or broam for execution as (const char*)lParam: */
#define BB_EXECUTE              10202 /* see also BB_EXECUTEASYNC */
#define BB_ABOUTSTYLE           10203
#define BB_ABOUTPLUGINS         10204

/* ----------------------------------- */
/* Show special menu */
#define BB_MENU                 10301
/* wParam values: */
# define BB_MENU_ROOT               0   /* normal rightclick menu */
# define BB_MENU_TASKS              1   /* workspaces menu (mid-click) */
# define BB_MENU_TOOLBAR            2   /* obsolete */
# define BB_MENU_ICONS              3   /* iconized tasks menu */
# define BB_MENU_SIGNAL             4   /* do nothing, for BBSoundFX */
# define BB_MENU_BROAM              6   /* (const char *)lParam: id-string (e.g. "configuration") */
/* for internal use only: */
# define BB_MENU_UPDATE             8   /* update the ROOT menu */

#define BB_HIDEMENU             10302 /* hide not-pinned menus */
#define BB_TOGGLETRAY           10303 /* toggles systembar etc. */
#define BB_TOGGLESYSTEMBAR      10303 /* xoblite */
/* Set the toolbar label to (const char*)lParam - returns to normal after 2 seconds) */
#define BB_SETTOOLBARLABEL      10304
#define BB_TOGGLEPLUGINS        10305
#define BB_SUBMENU              10306 /* for BBSoundFX, receive only */
#define BB_TOGGLESLIT           10307 /* xoblite */
#define BB_TOGGLETOOLBAR        10308 /* xoblite */
#define BB_AUTOHIDE             10309 /* wParam: 1=enable/0=disable autohide, lParam: 2 */

/* ----------------------------------- */
/* Shutdown etc
    wParam: 0=Shutdown, 1=Reboot, 2=Logoff, 3=Hibernate, 4=Suspend, 5=LockWorkstation
    lParam; 0=ask, 1=quiet */
#define BB_SHUTDOWN             10401

/* Show the 'run' dialog box */
#define BB_RUN                  10402

/* ----------------------------------- */
/* Sent from blackbox on workspace change.  For plugins: receive only */
#define BB_DESKTOPINFO          10501 /* lParam: struct DesktopInfo* */

/* Depreciated, use GetDesktopInfo() */
#define BB_LISTDESKTOPS         10502

/* switch to workspace #n in lParam */
#define BB_SWITCHTON            10503

/* Activate window (restore if minimized). lParam: hwnd to activate */
#define BB_BRINGTOFRONT         10504
  /* wParam flag: Zoom window into current workspace (bblean 1.16+) */
# define BBBTF_CURRENT              4

/* ----------------------------------- */
#define BB_WORKSPACE            10505
  /* wParam values: */
# define BBWS_DESKLEFT              0
# define BBWS_DESKRIGHT             1
# define BBWS_ADDDESKTOP            2
# define BBWS_DELDESKTOP            3
# define BBWS_SWITCHTODESK          4  /* lParam: workspace to switch to. */
# define BBWS_GATHERWINDOWS         5
# define BBWS_MOVEWINDOWLEFT        6  /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_MOVEWINDOWRIGHT       7  /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_PREVWINDOW            8  /* lParam: 0=current / 1=all workspaces */
# define BBWS_NEXTWINDOW            9  /* lParam: 0=current / 1=all workspaces */
# define BBWS_LASTDESK             10
# define BBWS_TOGGLESTICKY         12 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_EDITNAME             13
# define BBWS_MAKESTICKY           14 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_CLEARSTICKY          15 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_ISSTICKY             16 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_TOGGLEONTOP          17 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_GETTOPWINDOW         18 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_TOGGLEONBG           19 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_MINIMIZEALL          20
# define BBWS_RESTOREALL           21
# define BBWS_TILEVERTICAL         22
# define BBWS_TILEHORIZONTAL       23
# define BBWS_CASCADE              24
# define BBWS_MAKEONBG             25 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_CLEARONBG            26 /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_ISONBG               27 /* lParam: hwnd or NULL for foregroundwindow */

# define BBWS_DESKUP               28
# define BBWS_DESKDOWN             29
# define BBWS_MOVEWINDOWUP         30  /* lParam: hwnd or NULL for foregroundwindow */
# define BBWS_MOVEWINDOWDOWN       31  /* lParam: hwnd or NULL for foregroundwindow */

/*------------------------------------------ */
#define BB_TASKSUPDATE          10506  /* For plugins: receive only */
  /* lParam for BB_TASKSUPDATE: */
# define TASKITEM_ADDED             0        /*  wParam: hwnd */
# define TASKITEM_MODIFIED          1     /*  wParam: hwnd */
# define TASKITEM_ACTIVATED         2    /*  wParam: hwnd */
# define TASKITEM_REMOVED           3      /*  wParam: hwnd */
# define TASKITEM_REFRESH           4      /*  wParam: NULL (sent on window moved to workspace) */
# define TASKITEM_FLASHED           5      /*  wParam: hwnd */
# define TASKITEM_LANGUAGE          6     /*  win9x only, wParam: hwnd */

#define BB_TRAYUPDATE           10507 /* For plugins: receive only */
  /* lParam for BB_TRAYUPDATE: */
# define TRAYICON_ADDED             0
# define TRAYICON_MODIFIED          1   /* wParam: NIF_XXX flags */
# define TRAYICON_REMOVED           2

/* cleanup dead trayicons */
//@NOTE: is used by SystemBarEx, it does not seem to be obsolette
#define BB_CLEANTRAY            10508
/* #define BB_CLEANTASKS        10509 - obsolete */

/* File dragged over/dropped on desktop.
   lParam: filename
   wParam 0:drop (on mousebutton up) 1:drag (on mouse over)
   The plugin should return TRUE if it wants the file, FALSE if not */
#define BB_DRAGTODESKTOP        10510 /* For plugins: receive only */

/* Move window to workspace, dont switch. wParam: new desk - lParam: hwnd */
#define BB_SENDWINDOWTON        10511
/* Move window and switch to workspace. wParam: new desk - lParam: hwnd */
#define BB_MOVEWINDOWTON        10512

/* ----------------------------------- */
/* ShellHook messages, obsolete - register for BB_TASKSUPDATE instead */
#define BB_ADDTASK              10601
#define BB_REMOVETASK           10602
#define BB_ACTIVATESHELLWINDOW  10603
#define BB_ACTIVETASK           10604
#define BB_MINMAXTASK           10605
#define BB_REDRAWTASK           10610

/* ----------------------------------- */
/* Window commands */
#define BB_WINDOWSHADE          10606 /* lParam: hwnd or NULL for foregroundwindow */
#define BB_WINDOWGROWHEIGHT     10607 /* ... */
#define BB_WINDOWGROWWIDTH      10608 /* ... */
#define BB_WINDOWLOWER          10609 /* ... */
#define BB_WINDOWMINIMIZE       10611 /* ... */
#define BB_WINDOWRAISE          10612 /* ... */
#define BB_WINDOWMAXIMIZE       10613 /* ... */
#define BB_WINDOWRESTORE        10614 /* ... */
#define BB_WINDOWCLOSE          10615 /* ... */
#define BB_WINDOWSIZE           10616 /* ... */
#define BB_WINDOWMOVE           10617 /* ... */
#define BB_WINDOWMINIMIZETOTRAY 10618 /* not implemented */

/* ----------------------------------- */
/* Broadcast a string (Bro@m) to core and all plugins */
#define BB_BROADCAST            10901 /* wParam: 0, (LPCSTR)lParam: command string */

/* ----------------------------------- */
/* BBSlit messages */
#define SLIT_ADD                11001   /* lParam: plugin's hwnd */
#define SLIT_REMOVE             11002   /* lParam: plugin's hwnd */
#define SLIT_UPDATE             11003   /* lParam: plugin's hwnd or NULL */

// bbInterface addition
#define BBI_MAX_LINE_LENGTH     4000
#define BBI_POSTCOMMAND         (WM_USER+10)
#define VALID_SHADOWCOLOR   (1<<13)

/* for bbStylemaker: request a redraw of the specified parts */
#define BB_REDRAWGUI            10881
  /* wParam bitflags: */
  #define BBRG_TOOLBAR (1<<0)
  #define BBRG_MENU    (1<<1)
  #define BBRG_WINDOW  (1<<2)
  #define BBRG_DESK    (1<<3)
  #define BBRG_FOCUS   (1<<4)
  #define BBRG_PRESSED (1<<5)
  #define BBRG_STICKY  (1<<6)
  #define BBRG_FOLDER  (1<<7)
  #define BBRG_SLIT    (1<<8)

	/* Post a command or broam for execution (in lParam),
	like BB_EXECUTE, but returns immediately. Use with SendMessage */
#define BB_EXECUTEASYNC         10882

	/* desktop clicked, lParam: 0=leftdown 1=left, 2=right, 3=mid, 4=x1, 5=x2, 6=x3) */
#define BB_DESKCLICK            10884

	/* win9x: left/right winkey pressed */
#define BB_WINKEY               10886