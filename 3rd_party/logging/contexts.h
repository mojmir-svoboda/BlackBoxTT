#pragma once

namespace trace {
	typedef uint32_t context_t;
}

const trace::context_t CTX_INIT				=  (1 <<  0);
const trace::context_t CTX_BB					=  (1 <<  1);
const trace::context_t CTX_TASKS      =  (1 <<  2);
const trace::context_t CTX_WSPACE     =  (1 <<  3);
const trace::context_t CTX_NET        =  (1 <<  4);
const trace::context_t CTX_GFX				=  (1 <<  5);
const trace::context_t CTX_BBLIB      =  (1 <<  6);
const trace::context_t CTX_BBLIBCOMPAT=  (1 <<  7);
const trace::context_t CTX_CONFIG     =  (1 <<  8);
const trace::context_t CTX_SCRIPT     =  (1 <<  9);
const trace::context_t CTX_BIND       =  (1 << 10);
const trace::context_t CTX_RESOURCES  =  (1 << 11);
const trace::context_t CTX_PLUGINMGR  =  (1 << 12);
const trace::context_t CTX_PLUGIN     =  (1 << 13);
//const trace::context_t CTX_HOOK       =  (1 << 14);
//const trace::context_t CTX_EXPLORER   =  (1 << 15);
//const trace::context_t CTX_           =  (1 << 21);
