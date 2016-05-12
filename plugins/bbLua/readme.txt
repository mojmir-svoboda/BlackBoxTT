==============================================================================

 bbLua: A Lua Scripting Environment for BB4Win
 
==============================================================================

 
 Note: The stuff in here is mostly preliminary. Kinda like a way to think out
 loud while getting the readme in place. Changes may take place. And this
 plugin should not be distributed any time soon. Discuss it in the bbClean
 Developers forum. 
 

 LICENSE
 
    bbLua - Lua script engine for BlackBox
    Copyright © 2007 noccy

    This program is free software, released under the GNU General Public
    License (GPL version 2 or later). See:

    http://www.fsf.org/licenses/gpl.html

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
  

 USAGE
 
    When running as a standalone executable, you need to have the plugin
    bbKontroller-Proxy active in Blackbox. This is to allow calls from the
    outside world into blackbox memory space.
    
    Start bbLua with the name of the lua script to run as the parameter.
    The script engine will be initialized, and your script will load and
    run. When execution has completed, it will exit unless you specified
    the -d parameter.
    
    Use bbRun.exe -? for help on the available parameters.
    
    When running as a plugin, the file bbLua.lua will be loaded first,
    together with any requirements it might have. The main chunk is then
    executed, and from that point bbLua is just listening for events.
    
    Register an event like this:
    
      -- Require the stuff we require ;)
      require('bblua');
      -- Register the events
      hEvDisplayResize = registerEvent(EV_DISPLAYRESIZE,"ev_displayresize");
      hEvReconfigure = registerEvent(EV_RECONFIGURE,"ev_reconfigure");
      -- And unregister them again
      unregisterEvent(hEvDisplayResize);
      unregisterEvent(hEvReconfigure);
      
    For a full list of the events you can register, see the bblua.lua file.

    You can also register broams:
    
      -- Require the stuff we require ;)
      require('bblua');
      -- Register the broams
      brWinampPlay = registerBroam("@WinAmp.Play","br_winampplay");
      brWinampStop = registerBroam("@WinAmp.Stop","br_winampstop");
      -- And unregister them again
      unregisterBroam(brWinampPlay);
      unregisterBroam(brWinampStop);
      
    You can do some basic tweaking in the .rc file to optimize some minor
    parameters. 
    
    
 HOW DO I KNOW IF I'M RUNNING STANDALONE OR IN BLACKBOX?
 
    During standalone operation, the function GetProxyWindow() should return
    a value holding the hWnd of the bbKontroller Proxy window. This is not
    needed for the actual plugin version, so GetProxyWindow() should return
    0 in this case.


 WHAT CAN IT DO?
 
      * It can send broams. Lots of broams. 
      * It can build menus, listen to menus for events.
      * Register broams
      * manipulate window (findwindow, setwindowlong, getwindowlong, moving,
        sizing, transparency, nuking).
      * access the tray list to show/hide icons, or nuke tasks.
      * download porn to your system via wget at 03:00am every night.
      * Render updated statistics reports from databases nightly.
    
    Missing something? Go and add it yourself ;) The sourcecode is available
    for anonymous reading from svn://dev.noccy.com:969, but to participate
    and upload you must have an account. To get one of those buggers, drop
    an e-mail to noccy(at)angelicparticles(dot)net with your forum name, and
    your desired password.

 
 FUNCTIONS
 
    int GetBlackboxWindow() 
      Returns window handle to the blackbox desktop
    
    int GetProxyWindow()
      Returns window handle to the bbKontroller proxy if present
      
    void Print(text)
      Writes text to error console as defined
      
    int Reconfigure()
      Reconfigures blcakbox. Could come handy :)
    
    int SwitchDesktop(int nDesktop)
      Switches to another desktop
      
    int SetTimeout(int ms, str functionorcommand)
      Sets a one-time timeout. When it expires, it has be recreated
      in the target function.
      
    int FindWindow(str classname [,str windowtitle]);
      Finds a window and returns the handle. 0 if no match found.
      
    int SendMessage(int handle, int msg, int lparam [,int wparam]);
      Sends a message to a window.


 MODULES
 
    bblua.lua
      Purpose: Adds constants for bblua. Include this if you want to register
               events and such.
 
    winamp.lua 
      purpose: control winamp or foobar with foo_winamp_spam available from
               http://chron.visiondesigns.de/foobar2000/#foo_winamp_spam
      functs:  winamp_init() - Call prior to using any other function
               winamp_next() - Go to next track
               winamp_prev() - Go to previous track


 SOURCECODE
 
    The sourcecode is always available from the bbClean distribution site at 
    dev.noccy.com/bbclean, at www.lostinthebox.com, or via the SVN repository
    at svn://dev.noccy.com:969