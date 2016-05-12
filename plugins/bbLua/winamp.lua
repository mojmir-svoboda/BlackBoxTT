--
-- winamp remote control module for bbLua
--
-- Include by adding require('winamp') to the top of your script
--

systemapi = {
	WM_COMMAND = 273;
}

winamp_commands = {
	previous = 40044;
	next = 40048;
	play = 40045;
	pause = 40046;
	stop = 40047;
	fadeandstop = 40147;
	stopaftercurrent = 40157;
	fastforward5sec = 40148;
	rewind5sec = 40144;
	gotoplayliststart = 40154;
	gotoplaylistend = 40158;
	openfile = 40029;
	openurl = 40155;
	openfileinfo = 40188;
	setdisplayelapsed = 40037;
	setdisplayremaining = 40038;
	togglepreferences = 40012;
	openvisoptions = 40190;
	openvispluginoptions = 40191;
	openvisplugin = 40192;
	openvisconfiguration = 40221;
	toggleabout = 40041;
	toggletitleautoscroll = 40189;
	togglealwaysontop = 40019;
	togglewindowshade = 40064;
	toggleplaylistwindowshade = 40266;
	toggledoublesize = 40165;
	toggleeq = 40036;
	toggleplaylist = 40040;
	togglemainwindow = 40258;
	toggleminibrowser = 40298;
	toggleeasymove = 40186;
	raisevolume = 40058;
	lowervolume = 40059;
	togglerepeat = 40022;
	toggleshuffle = 40023;
	jumptotime = 40193;
	jumptofile = 40194;
	openskinselector = 40219;
	reloadskin = 40291;
	closewinamp = 40001;
	playlistback10 = 40197;
	showbookmarks = 40320;
	addbookmark = 40321;
	playaudiocd = 40323;
	loadeqpreset = 40253;
	saveeqpreset = 40254;
	openeqpresetload = 40172;
	openeqpresetautoload = 40173;
	loadeqdefaultpreset = 40174;
	openeqsavepreset = 40175;
	openeqautoloadsavepreset = 40176;
	openeqdeletepreset = 40178;
	openeqdeleteautoloadpreset = 40180;
};

winamp_lua = {
	initialized = false;
};

function winamp_init()

	winamp_lua.hwnd = FindWindow("Winamp v1.x");
	winamp_lua.initialized = (winamp_lua.hwnd ~= 0);

end

function winamp_next() 

	if (winamp_lua.initialized == true) then
		SendMessage(winamp_lua.hwnd,systemapi.WM_COMMAND,winamp_commands.next);
	end

end

function winamp_prev()

	if (winamp_lua.initialized == true) then
		SendMessage(winamp_lua.hwnd,systemapi.WM_COMMAND,winamp_commands.previous);
	end

end