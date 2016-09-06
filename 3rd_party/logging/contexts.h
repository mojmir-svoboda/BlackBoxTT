#pragma once
#if _MSC_VER >= 1600
#endif

namespace trace {
	typedef uint32_t context_t;
}

const trace::context_t CTX_DEFAULT    =  (1 <<  0);
const trace::context_t CTX_INIT				=  (1 <<  1);
const trace::context_t CTX_BB					=  (1 <<  2);
const trace::context_t CTX_GFX				=  (1 <<  3);
const trace::context_t CTX_EXPLORER   =  (1 <<  4);
const trace::context_t CTX_HOOK       =  (1 <<  5);
const trace::context_t CTX_TRAY       =  (1 <<  6);
const trace::context_t CTX_CONFIG     =  (1 <<  7);
const trace::context_t CTX_SCRIPT     =  (1 <<  8);
const trace::context_t CTX_STYLE      =  (1 <<  9);
const trace::context_t CTX_PROFILING  =  (1 << 10);
const trace::context_t CTX_SERIALIZE  =  (1 << 11);
const trace::context_t CTX_MEMORY     =  (1 << 12);
const trace::context_t CTX_RESOURCES  =  (1 << 13);
const trace::context_t CTX_BBLIB      =  (1 << 14);
const trace::context_t CTX_BBLIBCOMPAT=  (1 << 15);
const trace::context_t CTX_PLUGINMGR  =  (1 << 16);
const trace::context_t CTX_PLUGIN     =  (1 << 17);
const trace::context_t CTX_NET        =  (1 << 18);
const trace::context_t CTX_WSPACE     =  (1 << 19);
const trace::context_t CTX_BIND       =  (1 << 20);
//const trace::context_t CTX_           =  (1 << 21);
