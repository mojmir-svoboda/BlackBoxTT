+!Right::
   Run, c:\bb_devel\bbcli.exe -c "(SwitchVertexViaEdge ""right"")"
Return

+!Left::
   Run, c:\bb_devel\bbcli.exe -c "(SwitchVertexViaEdge ""left"")"
Return

!F1::
   Run, c:\bb_devel\bbcli.exe -c "(SetCurrentVertexId ""f1"")"
Return

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