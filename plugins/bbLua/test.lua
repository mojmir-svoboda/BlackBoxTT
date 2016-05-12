require("bblua");
require("winamp");
require("winshell");

print("BlackBox hWnd = " .. GetBlackboxWindow());
-- SendBroam("@BBCore.About");
print("EV_RECONFIGURE is " .. EV_RECONFIGURE);
print("Initializing winamp support...");
winamp_init();
print("Winamp hWnd is " .. winamp_lua.hwnd);
-- print("Changing to next track...");
-- winamp_next();

msgbox("Hey, it worked!", "Lua Message Box");



evReconfigure = RegisterEvent(EV_RECONFIGURE,"bbReconfigure");

function bbReconfigure()

	print("BlackBox was just reconfigured!");

end
