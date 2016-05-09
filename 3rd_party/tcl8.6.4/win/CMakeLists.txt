project(tcl)

# cl -nologo -c -W3 -DUNICODE -D_UNICODE -W3 -D _CRT_SECURE_NO_DEPRECATE -D _CRT_NONSTDC_NO_DEPRECATE -Fp.\Release
# _AMD64_VC12\tcl_ThreadedStatic\ -O2  -Ot -Oi -fp:strict -Gs -GS -GL -MT -I"c:\devel\tcl864-src\tcl8.6.4\win\..\win" -I"c
# :\devel\tcl864-src\tcl8.6.4\win\..\generic" -I"c:\devel\tcl864-src\tcl8.6.4\win\..\libtommath" -DTCL_TOMMATH -DMP_PREC=4
#  -Dinline=__inline -DHAVE_ZLIB=1 -DTCL_CFGVAL_ENCODING=\"cp1252\" -DSTDC_HEADERS -DTCL_THREADS=1 -DUSE_THREAD_ALLOC=1 -D
#  STATIC_BUILD -DNDEBUG -DTCL_CFG_OPTIMIZED -DTCL_CFG_DO64BIT -Ic:\devel\tcl864-src\tcl8.6.4\win\..\compat\zlib -DBUILD_tc
#  l -Fo.\Release_AMD64_VC12\tcl_ThreadedStatic\tclZlib.obj c:\devel\tcl864-src\tcl8.6.4\win\..\generic\tclZlib.c

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-DTCL_TOMMATH)
add_definitions(-DMP_PREC=4)
add_definitions(-Dinline=__inline)
add_definitions(-DHAVE_ZLIB=1)
add_definitions(-DTCL_CFGVAL_ENCODING="cp1252")
add_definitions(-DSTDC_HEADERS)
add_definitions(-DTCL_THREADS=1)
add_definitions(-DUSE_THREAD_ALLOC=1)
add_definitions(-DSTATIC_BUILD)
add_definitions(-DNDEBUG)
add_definitions(-DTCL_CFG_OPTIMIZED)
add_definitions(-DBUILD_tcl)
#add_definitions(-D)

# detect 32 / 64bit environment
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(buildbits 64)
  message("tcl build=64bit")
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(buildbits 32)
  message("tcl build=32bit")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

if( buildbits EQUAL 64 )
  add_definitions(-DTCL_CFG_DO64BIT)
endif( buildbits EQUAL 64 )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../generic)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../compat/zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libtommath)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#        cl -nologo -c -W3 -DUNICODE -D_UNICODE -W3 -D _CRT_SECURE_NO_DEPRECATE -D _CRT_NONSTDC_NO_DEPRECATE -Fp.\Release
#_AMD64_VC12\tcl_ThreadedStatic\ -O2  -Ot -Oi -fp:strict -Gs -GS -GL -DTCL_CFGVAL_ENCODING=\"cp1252\" -DSTDC_HEADERS -DTC
#L_THREADS=1 -DUSE_THREAD_ALLOC=1 -DSTATIC_BUILD -DNDEBUG -DTCL_CFG_OPTIMIZED -DTCL_CFG_DO64BIT -Zl -DSTATIC_BUILD -I"c:\
#devel\tcl864-src\tcl8.6.4\win\..\win" -I"c:\devel\tcl864-src\tcl8.6.4\win\..\generic" -I"c:\devel\tcl864-src\tcl8.6.4\wi
#n\..\libtommath" -Fo.\Release_AMD64_VC12\tcl_ThreadedStatic\tclTomMathStubLib.obj c:\devel\tcl864-src\tcl8.6.4\win\..\ge
#neric\tclTomMathStubLib.c
#tclTomMathStubLib.c

set(stub_SOURCES
  ../generic/tclStubLib.c
  ../generic/tclOOStubLib.c
  ../generic/tclTomMathStubLib.c
)
add_library(tclstub STATIC ${stub_SOURCES})
set_property(TARGET tclstub PROPERTY FOLDER "3rd/tclstub")
#set_target_properties(tclstub PROPERTIES COMPILE_FLAGS "-DUSE_TCL_STUBS")

set(tcl_SOURCES
../generic/tclMain.c
../generic/tclZlib.c
../generic/regcomp.c
../generic/regerror.c
../generic/regexec.c
../generic/regfree.c
../generic/tclAlloc.c
../generic/tclAssembly.c
../generic/tclAsync.c
../generic/tclBasic.c
../generic/tclBinary.c
../generic/tclCkalloc.c
../generic/tclClock.c
../generic/tclCmdAH.c
../generic/tclCmdIL.c
../generic/tclCmdMZ.c
../generic/tclCompCmds.c
../generic/tclCompCmdsGR.c
../generic/tclCompCmdsSZ.c
../generic/tclCompExpr.c
../generic/tclCompile.c
../generic/tclConfig.c
../generic/tclDate.c
../generic/tclDictObj.c
../generic/tclDisassemble.c
../generic/tclEncoding.c
../generic/tclEnsemble.c
../generic/tclEnv.c
../generic/tclEvent.c
../generic/tclExecute.c
../generic/tclFCmd.c
../generic/tclFileName.c
../generic/tclGet.c
../generic/tclHash.c
../generic/tclHistory.c
../generic/tclIndexObj.c
../generic/tclInterp.c
../generic/tclIO.c
../generic/tclIOCmd.c
../generic/tclIOGT.c
../generic/tclIOSock.c
../generic/tclIOUtil.c
../generic/tclIORChan.c
../generic/tclIORTrans.c
../generic/tclLink.c
../generic/tclListObj.c
../generic/tclLiteral.c
../generic/tclLoad.c
../generic/tclMain.c
../generic/tclNamesp.c
../generic/tclNotify.c
../generic/tclOO.c
../generic/tclOOBasic.c
../generic/tclOOCall.c
../generic/tclOODefineCmds.c
../generic/tclOOInfo.c
../generic/tclOOMethod.c
../generic/tclOOStubInit.c
../generic/tclObj.c
../generic/tclOptimize.c
../generic/tclPanic.c
../generic/tclParse.c
../generic/tclPathObj.c
../generic/tclPipe.c
../generic/tclPkg.c
../generic/tclPosixStr.c
../generic/tclPreserve.c
../generic/tclProc.c
../generic/tclRegexp.c
../generic/tclResolve.c
../generic/tclResult.c
../generic/tclScan.c
../generic/tclStringObj.c
../generic/tclStrToD.c
../generic/tclStubInit.c
../generic/tclThread.c
../generic/tclThreadAlloc.c
../generic/tclThreadJoin.c
../generic/tclThreadStorage.c
../generic/tclTimer.c
../generic/tclTomMathInterface.c
../generic/tclTrace.c
../generic/tclUtf.c
../generic/tclUtil.c
../generic/tclVar.c
../compat/zlib/adler32.c
../compat/zlib/compress.c
../compat/zlib/crc32.c
../compat/zlib/deflate.c
../compat/zlib/infback.c
../compat/zlib/inffast.c
../compat/zlib/inflate.c
../compat/zlib/inftrees.c
../compat/zlib/trees.c
../compat/zlib/uncompr.c
../compat/zlib/zutil.c
	../libtommath/bncore.c
	../libtommath/bn_reverse.c
	../libtommath/bn_fast_s_mp_mul_digs.c
	../libtommath/bn_fast_s_mp_sqr.c
	../libtommath/bn_mp_add.c
	../libtommath/bn_mp_add_d.c
	../libtommath/bn_mp_and.c
	../libtommath/bn_mp_clamp.c
	../libtommath/bn_mp_clear.c
	../libtommath/bn_mp_clear_multi.c
	../libtommath/bn_mp_cmp.c
	../libtommath/bn_mp_cmp_d.c
	../libtommath/bn_mp_cmp_mag.c
	../libtommath/bn_mp_cnt_lsb.c
	../libtommath/bn_mp_copy.c
	../libtommath/bn_mp_count_bits.c
	../libtommath/bn_mp_div.c
	../libtommath/bn_mp_div_d.c
	../libtommath/bn_mp_div_2.c
	../libtommath/bn_mp_div_2d.c
	../libtommath/bn_mp_div_3.c
	../libtommath/bn_mp_exch.c
	../libtommath/bn_mp_expt_d.c
	../libtommath/bn_mp_grow.c
	../libtommath/bn_mp_init.c
	../libtommath/bn_mp_init_copy.c
	../libtommath/bn_mp_init_multi.c
	../libtommath/bn_mp_init_set.c
	../libtommath/bn_mp_init_set_int.c
	../libtommath/bn_mp_init_size.c
	../libtommath/bn_mp_karatsuba_mul.c
	../libtommath/bn_mp_karatsuba_sqr.c
	../libtommath/bn_mp_lshd.c
	../libtommath/bn_mp_mod.c
	../libtommath/bn_mp_mod_2d.c
	../libtommath/bn_mp_mul.c
	../libtommath/bn_mp_mul_2.c
	../libtommath/bn_mp_mul_2d.c
	../libtommath/bn_mp_mul_d.c
	../libtommath/bn_mp_neg.c
	../libtommath/bn_mp_or.c
	../libtommath/bn_mp_radix_size.c
	../libtommath/bn_mp_radix_smap.c
	../libtommath/bn_mp_read_radix.c
	../libtommath/bn_mp_rshd.c
	../libtommath/bn_mp_set.c
	../libtommath/bn_mp_set_int.c
	../libtommath/bn_mp_shrink.c
	../libtommath/bn_mp_sqr.c
	../libtommath/bn_mp_sqrt.c
	../libtommath/bn_mp_sub.c
	../libtommath/bn_mp_sub_d.c
	../libtommath/bn_mp_to_unsigned_bin.c
	../libtommath/bn_mp_to_unsigned_bin_n.c
	../libtommath/bn_mp_toom_mul.c
	../libtommath/bn_mp_toom_sqr.c
	../libtommath/bn_mp_toradix_n.c
	../libtommath/bn_mp_unsigned_bin_size.c
	../libtommath/bn_mp_xor.c
	../libtommath/bn_mp_zero.c
	../libtommath/bn_s_mp_add.c
	../libtommath/bn_s_mp_mul_digs.c
	../libtommath/bn_s_mp_sqr.c
	../libtommath/bn_s_mp_sub.c
  #cat.c
  tclWinReg.c
  tclWinDde.c
  tclAppInit.c
  tclWinChan.c
  tclWin32Dll.c
  tclWinConsole.c
  tclWinError.c
  tclWinFCmd.c
  tclWinFile.c
  tclWinInit.c
  tclWinLoad.c
  tclWinNotify.c
  tclWinPipe.c
  tclWinSerial.c
  tclWinSock.c
  tclWinTest.c
  tclWinThrd.c
  tclWinTime.c
  )

#        cl -DBUILD_tcl -nologo -c -W3 -DUNICODE -D_UNICODE -W3 -D _CRT_SECURE_NO_DEPRECATE -D _CRT_NONSTDC_NO_DEPRECATE
#-Fp.\Release_AMD64_VC12\tcl_ThreadedStatic\ -O2  -Ot -Oi -fp:strict -Gs -GS -GL -MT -I"c:\devel\tcl864-src\tcl8.6.4\win\
#..\win" -I"c:\devel\tcl864-src\tcl8.6.4\win\..\generic" -I"c:\devel\tcl864-src\tcl8.6.4\win\..\libtommath" -DTCL_TOMMATH
# -DMP_PREC=4 -Dinline=__inline -DHAVE_ZLIB=1 -DTCL_CFGVAL_ENCODING=\"cp1252\" -DSTDC_HEADERS -DTCL_THREADS=1 -DUSE_THREA
#D_ALLOC=1 -DSTATIC_BUILD -DNDEBUG -DTCL_CFG_OPTIMIZED -DTCL_CFG_DO64BIT  -DCFG_INSTALL_LIBDIR="\"C:\\Program Files\\Tcl\
#\lib\""  -DCFG_INSTALL_BINDIR="\"C:\\Program Files\\Tcl\\bin\""  -DCFG_INSTALL_SCRDIR="\"C:\\Program Files\\Tcl\\lib\\tc
#l8.6\""  -DCFG_INSTALL_INCDIR="\"C:\\Program Files\\Tcl\\include\""  -DCFG_INSTALL_DOCDIR="\"C:\\Program Files\\Tcl\\doc
#\""      -DCFG_RUNTIME_LIBDIR="\"C:\\Program Files\\Tcl\\lib\""  -DCFG_RUNTIME_BINDIR="\"C:\\Program Files\\Tcl\\bin\""
# -DCFG_RUNTIME_SCRDIR="\"C:\\Program Files\\Tcl\\lib\\tcl8.6\""  -DCFG_RUNTIME_INCDIR="\"C:\\Program Files\\Tcl\\include
#\""  -DCFG_RUNTIME_DOCDIR="\"C:\\Program Files\\Tcl\\doc\""      -Fo.\Release_AMD64_VC12\tcl_ThreadedStatic\tclPkgConfig
#.obj c:\devel\tcl864-src\tcl8.6.4\win\..\generic\tclPkgConfig.c
# ... fiery bowls of bloody hell!

add_library(tcl STATIC ${tcl_SOURCES} ../generic/tclPkgConfig.c)
set_property(TARGET tcl PROPERTY FOLDER "3rd/tcl")
