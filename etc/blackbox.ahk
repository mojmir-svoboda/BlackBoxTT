; # = Win  ! = Alt  ^ = Control  + = Shift

RButton::
RButton Up::
	; if bbmenu returns 1 it means it accepts the click. otherwise forward the click to explorer
 	RunWait, c:\bb_devel\bbmenu.exe -c "(ToggleDesktopMenu ""bbTT Menu"")",, UseErrorLevel
	if ErrorLevel = 0
		MouseClick, right 
Return

+!h::
   Run, c:\bb_devel\bbcli.exe -c "(SwitchVertexViaEdge ""left"")"
Return
+!j::
   Run, c:\bb_devel\bbcli.exe -c "(SwitchVertexViaEdge ""down"")"
Return
+!k::
   Run, c:\bb_devel\bbcli.exe -c "(SwitchVertexViaEdge ""up"")"
Return
+!l::
   Run, c:\bb_devel\bbcli.exe -c "(SwitchVertexViaEdge ""right"")"
Return

+!F1::
   Run, c:\bb_devel\bbcli.exe -c "(SetCurrentVertexId ""Desktop 1"")"
Return
+!F2::
   Run, c:\bb_devel\bbcli.exe -c "(SetCurrentVertexId ""Desktop 2"")"
Return

+!Left::
   Run, c:\bb_devel\bbcli.exe -c "(MoveTopWindowToVertexViaEdge ""left"")"
Return
+!Down::
   Run, c:\bb_devel\bbcli.exe -c "(MoveTopWindowToVertexViaEdge ""down"")"
Return
+!Up::
   Run, c:\bb_devel\bbcli.exe -c "(MoveTopWindowToVertexViaEdge ""up"")"
Return
+!Right::
   Run, c:\bb_devel\bbcli.exe -c "(MoveTopWindowToVertexViaEdge ""right"")"
Return

^!h::
   Run, c:\bb_devel\bbcli.exe -c "(MaximizeTopWindow ""horizontal"")"
Return
^!j::
   Run, c:\bb_devel\bbcli.exe -c "(MaximizeTopWindow ""vertical"")"
Return
^!m::
   Run, c:\bb_devel\bbcli.exe -c "(MaximizeTopWindow ""both"")"
Return
^!i::
   Run, c:\bb_devel\bbcli.exe -c "(SetTaskManIgnored ""toggle"")"
Return



; ---------------------------------------------------------------------------------------

+!e::
   Run, Explorer.exe
Return

+!t::
   Run, c:\totalcmd\totalcmd64.exe
Return

+!d::
   Run, C:\cygwin\bin\mintty.exe -i /Cygwin-Terminal.ico /bin/bash -
Return

+!r::
	shell:=ComObjCreate("Shell.Application")
	#R::shell.FileRun()
Return
