/**
 * Copyright (C) 2011-2016 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once

#if defined TRACE_ENABLED

#	if defined (__GNUC__) && defined(__unix__)
#		define TRACE_API __attribute__ ((__visibility__("default")))
#	elif defined (WIN32) || defined (WIN64)
#		if defined TRACE_STATIC
#			define TRACE_API
#		elif defined TRACE_DLL
#			define TRACE_API __declspec(dllexport)
#		else
#			define TRACE_API __declspec(dllimport)
#		endif
#	elif defined (_XBOX)
#		if defined TRACE_STATIC
#			define TRACE_API:
#		elif defined TRACE_DLL
#			define TRACE_API __declspec(dllexport)
#		else
#			define TRACE_API __declspec(dllimport)
#		endif
#	endif

#	if  defined _MSC_VER
#		include <vadefs.h>
#	else
#		include <stdarg.h>	// for va_args
#	endif

#include "trace_scope_guard.h"
#include <cstdint>

/**	@macro		TRACE_CONFIG_INCLUDE
 *	@brief		overrides default config with user-specified one
 **/
#	if !defined TRACE_CONFIG_INCLUDE
#		include	"default_config.h"
#	else
#		include TRACE_CONFIG_INCLUDE
#	endif

/**	@macro		TRACE_LEVELS_INCLUDE
 *	@brief		overrides default levels with user-specified one
 **/
#	if !defined TRACE_LEVELS_INCLUDE
#		include	"default_levels.h"
#	else
#		include TRACE_LEVELS_INCLUDE
#	endif

/**	@macro		TRACE_CONTEXTS_INCLUDE
 *	@brief		overrides default contexts with user-specified one
 **/
#	if !defined TRACE_CONTEXTS_INCLUDE
#		include	"default_contexts.h"
#	else
#	include TRACE_CONTEXTS_INCLUDE
#	endif

/**	@macro		TRACE_ADAPT_LEVELS
 *	@brief		allows transformation of users level into trace levels
 **/
#	if !defined TRACE_ADAPT_LEVEL
#	define TRACE_ADAPT_LEVEL(lvl) lvl
#	endif

/*****************************************************************************/
/* Text logging macros                                                       */
/*****************************************************************************/

/**	@macro		TRACE_MSG
 *	@brief		logging of the form TRACE_MSG(lvl, ctx, fmt, ...)
 **/
#	define TRACE_MSG(level, context, fmt, ... )	\
		trace::Write(TRACE_ADAPT_LEVEL(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
/**	@macro		TRACE_MSG_IF
 *	@brief		logging of the form TRACE_MSG_IF((foo == bar), lvl, ctx, fmt, ...)
 **/
#	define TRACE_MSG_IF(condition, level, context, fmt, ... )	\
		if (condition)	\
			trace::Write(TRACE_ADAPT_LEVEL(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

/**	@macro		TRACE_MSG_VA
 *	@brief		logging of the form TRACE_MSG_VA(lvl, ctx, fmt, va_list)
 **/
#	define TRACE_MSG_VA(level, context, fmt, vaargs)	\
		trace::WriteVA(TRACE_ADAPT_LEVEL(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, vaargs)

/**	@macro		TRACE_SCOPE_MSG
 *	@brief		logs "entry to" and "exit from" scope
 *	@param[in]	fmt			formatted message appended to the scope
 **/
#	define TRACE_SCOPE_MSG(level, context, fmt, ...)	\
		trace::ScopedLog TRACE_UNIQUE(entry_guard_)(TRACE_ADAPT_LEVEL(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

/**	@macro		TRACE_SCOPE_MSG_IF
 *	@brief		logs "entry to" and "exit from" scope if condition is true
 *	@param[in]	fmt			formatted message appended to the scope
 **/
#	define TRACE_SCOPE_MSG_IF(condition, level, context, fmt, ...)	\
		trace::ScopedLog TRACE_UNIQUE(entry_guard_)(condition, TRACE_ADAPT_LEVEL(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

/**	@macro		TRACE_SCOPE
 *	@brief		logs "entry to" and "exit from" scope
 **/
#	define TRACE_SCOPE(level, context)	TRACE_SCOPE_MSG(TRACE_ADAPT_LEVEL(level), context, "%s", __FUNCTION__)

/**	@macro		TRACE_SCOPE_IF
 *	@brief		logs "entry to" and "exit from" scope
 **/
#	define TRACE_SCOPE_IF(condition, level, context)	TRACE_SCOPE_MSG(condition, TRACE_ADAPT_LEVEL(level), context, "%s", __FUNCTION__)

/**	@macro		TRACE_CODE
 *	@brief		code that is executed only when trace is enabled
 **/
#	define TRACE_CODE(code) code


/*****************************************************************************/
/* Plot logging macros                                                       */
/*****************************************************************************/

/**	@macro		TRACE_PLOT_XY
 *	@brief		logging of 2d xy data in the form of 2d plot
 **/
#	define TRACE_PLOT_XY(level, context, x, y, fmt, ... )	\
	trace::WritePlot(TRACE_ADAPT_LEVEL(level), context, x, y, fmt, __VA_ARGS__)

/**	@macro		TRACE_PLOT_XY_MARKER
 *	@brief		place marker on 2d plot
 **/
#	define TRACE_PLOT_XY_MARKER(level, context, x, y, fmt, ... ) \
	trace::WritePlotMarker(TRACE_ADAPT_LEVEL(level), context, x, y, fmt, __VA_ARGS__)
/**	@macro		TRACE_PLOT_XYZ
 *	@brief		logging of 3d xyz data
 **/
#	define TRACE_PLOT_XYZ(level, context, x, y, z, fmt, ... ) \
	trace::WritePlot(TRACE_ADAPT_LEVEL(level), context, x, y, z, fmt, __VA_ARGS__)
/**	@macro		TRACE_PLOT_CLEAR
 *	@brief		clear curve data identified by "tag/curve" or clear all plot using "tag" only
 **/
#	define TRACE_PLOT_CLEAR(level, context, fmt, ...)	\
	trace::WritePlotClear(TRACE_ADAPT_LEVEL(level), context, fmt, __VA_ARGS__)

/*****************************************************************************/
/* Table logging macros                                                      */
/*****************************************************************************/

/**	@macro		TRACE_TABLE
 *	@brief		logging of tabular data
 *	@see		trace::WriteTable
 **/
#	define TRACE_TABLE          trace::WriteTable
#	define TRACE_TABLE_HHEADER	trace::WriteTableSetHHeader
#	define TRACE_TABLE_COLOR	trace::WriteTableSetColor
#	define TRACE_TABLE_CLEAR	trace::WriteTableClear


/*****************************************************************************/
/* Gantt logging macros                                                      */
/*****************************************************************************/

/**	@macro		TRACE_GANTT_.*
 *	@brief		logging of the form TRACE_GANTT_MSG(lvl, ctx, fmt, ...)
 **/
#	define TRACE_GANTT_BGN                  trace::WriteGanttBgn
#	define TRACE_GANTT_END                  trace::WriteGanttEnd

#	define TRACE_GANTT_FRAME_BGN            trace::WriteGanttFrameBgn
#	define TRACE_GANTT_FRAME_END            trace::WriteGanttFrameEnd

/**	@macro		TRACE_GANTT_MSG_VA
 *	@brief		traces event into gantt chart  of the form TRACE_GANTT_MSG_VA(lvl, ctx, fmt, va_list)
 **/
#	define TRACE_GANTT_MSG_VA(fmt, vaargs)	trace::WriteGanttVA(fmt, vaargs);

/**	@macro		TRACE_GANTT_SCOPE
 *	@brief		traces event into gantt chart
 **/
#	define TRACE_GANTT_SCOPE                trace::ScopedGantt TRACE_UNIQUE(profile_entry_guard_)

/**	@macro		TRACE_GANTT_FRAME_SCOPE
 *	@brief		traces frame into gantt chart
 **/
#	define TRACE_GANTT_FRAME_SCOPE          trace::ScopedGanttFrame TRACE_UNIQUE(profile_entry_guard_)

#	define TRACE_GANTT_CLEAR                trace::WriteGanttClear

/*****************************************************************************/
/* Misc notification macros                                                  */
/*****************************************************************************/

/**	@macro		TRACE_SOUND.*
 *	@brief		logging of the form TRACE_SOUND(lvl, ctx, fmt, ...)
 **/
#	define TRACE_SOUND(level, context, vol, loop, fmt, ...)	\
	trace::WriteSound(level, context, vol, loop, fmt, ___VA_ARGS__)



/*****************************************************************************/
/* Basic setup and utilitary macros                                          */
/*****************************************************************************/

/**	@macro		TRACE_APPNAME
 *	@brief		sets application name that will be sent to server to identify
 **/
#	define TRACE_APPNAME(name) trace::SetAppName(name)
/**	@macro		TRACE_CONNECT
 *	@brief		connects to server and sends application name to server
 **/
#	define TRACE_CONNECT(addr, port) trace::Connect(addr, port)
/**	@macro		TRACE_DISCONNECT
 *	@brief		disconnects from server
 **/
#	define TRACE_DISCONNECT() trace::Disconnect()

/**	@macro		TRACE_SET_LEVEL_MASK
 *	@brief		sets initial level mask
 **/
#	define TRACE_SET_LEVEL(ctx, lvl) trace::SetRuntimeLevelForContext(ctx, lvl)
/**	@macro		TRACE_SET_CONTEXT_MASK
 *	@brief		set initial context mask
 **/
#	define TRACE_SET_CONTEXT_MASK(n) trace::SetRuntimeContextMask(n)
/**	@macro		TRACE_SETBUFFERED
 *	@brief		switch between buffered/unbuffered
 **/
#	define TRACE_SETBUFFERED(n) trace::SetRuntimeBuffering(n)

/** @macro		TRACE_EXPORT_CSV
 *  @brief      causes export of current server content as csv format
 */
#	define TRACE_EXPORT_CSV(file)	trace::ExportToCSV(file)

/** @macro		TRACE_SET_LEVEL_DICT
 *  @brief      
 */
#	define TRACE_SET_LEVEL_DICTIONARY(dictpairs, size)		trace::SetLevelDictionary(dictpairs, size)
#	define TRACE_SET_CONTEXT_DICTIONARY(dictpairs, size)		trace::SetContextDictionary(dictpairs, size)

/** @macro		TRACE_FLUSH
 *  @brief      forces flush of all buffers into socket
 *				comes handy when you expect crash and want the data to be sent
 *				out immeadiately.
 *				typical usage would be logging of a text from assert for example
 */
#	define TRACE_FLUSH()		trace::Flush()


	namespace trace {

		/**@fn		SetAppName
		 * @brief	sets identification string of client application
		 **/
		TRACE_API void SetAppName (char const *);
		TRACE_API char const * GetAppName ();

		/**@fn		SetHostName
		 * @brief	sets address of the host the client will connect to
		 *				if not specified client will connect to "localhost" by default
		 **/
		TRACE_API void SetHostName (char const * addr);
		TRACE_API char const * GetHostName ();

		/**@fn		SetHostPort
		 * @brief	sets port of the host the client will connect to
		 *				if not specified client will connect to "13127" by default
		 **/
		TRACE_API void SetHostPort (char const * port);
		TRACE_API char const * GetHostPort ();

		TRACE_API void Connect (char const * host, char const * port);
		TRACE_API void Disconnect ();

		/**@fn		SetRuntimeLevelForContext
		 * @brief	adjusts run-time context of log message filtering
		 **/
		TRACE_API void SetRuntimeLevelForContext (context_t ctx, level_t level);
		TRACE_API level_t GetRuntimeLevelForContext (context_t ctx);

		struct DictionaryPair {
			uint64_t first;
			char const * second;
		};
		TRACE_API void SetLevelDictionary (DictionaryPair const * pairs, size_t sz);
		TRACE_API void SetContextDictionary (DictionaryPair const * pairs, size_t sz);

		/**@fn		SetRuntimeBuffering
		 * @brief	adjusts run-time buffering of log message filtering
		 **/
		TRACE_API void SetRuntimeBuffering (bool level);
		TRACE_API bool GetRuntimeBuffering ();

		/**@fn		ExportToCSV
		 * @brief	client sends command to force server dump content to csv
		 **/
		TRACE_API void ExportToCSV (char const *);


		TRACE_API void Flush ();

		/**@fn		RuntimeFilterPredicate
		 * @brief	decides if message will be logged or not
		 */
		TRACE_API bool RuntimeFilterPredicate (level_t level, context_t context);

		/**@fn		Write to log
		 * @brief	write to log of the form (fmt, va_list)
		 **/
		TRACE_API void WriteVA (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list);

		/**@fn		Write to log
		 * @brief	write to log of the form (fmt, ...)
		 **/
#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		TRACE_API void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...) __attribute__ ((format(printf, 6, 7) ));
#elif defined _MSC_VER
		TRACE_API void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...);
#endif

		/**@fn		WritePlot
		 * @brief	writes data to be plotted in server part
		 *
		 * @param[in]	x	float x-coordinate
		 * @param[in]	y	float y-coordinate
		 * @param[in]	fmt		message to server
		 * @Note:
		 *		the format determines how and where your data is plotted in the server
		 *	counterpart. the message consists of two mandatory parts: plot_name and curve_name
		 *	in the form
		 *			plot_name/curve_name
		 *
		 * @Example:
		 *		WritePlot(lvl, ctx, 1.0f, 2.0f, "my_plot/curve1");
		 *		WritePlot(lvl, ctx, 1.0f, 6.0f, "my_plot/curve2");
		 *		WritePlot(lvl, ctx, 1.0f,-1.0f, "my_plot2/c");
		 *		WritePlotMarker(lvl, ctx, 1.0f,-1.0f, "my_plot2");
		 *		will add value 2 in curve1 and value in curve2, but they will be in the same
		 *		plot "my_plot"
		 *		third value of -1 will take place into another plot widget.
		 */
		TRACE_API void WritePlot (level_t level, context_t context, float x, float y, char const * fmt, ...);
		TRACE_API void WritePlotMarker (level_t level, context_t context, float x, float y, char const * fmt, ...);
		TRACE_API void WritePlot (level_t level, context_t context, float x, float y, float z, char const * fmt, ...);
		TRACE_API void WritePlotClear (level_t level, context_t context, char const * fmt, ...);

		/**@class	ScopedLog
		 * @brief	RAII class for logging entry on construction and exit on destruction **/
		struct TRACE_API ScopedLog
		{
			enum E_Type { e_None = 0, e_Entry = 1, e_Exit = 2 };
			level_t m_level;
			context_t m_context;
			char const * m_file;
			int m_line;
			char const * m_fn;
			unsigned long long m_start;
			bool m_enabled;

			ScopedLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...);
			ScopedLog (bool enabled, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...);
			~ScopedLog ();
		};


		/**@fn		Write table to log
		 * @brief	writes tavle to log of the form (fmt, ...)
		 *
		 * @param[in]	x	x-coordinate of the table. -1, 0, ...X
		 * @param[in]	y	y-coordinate of the table  -1, 0, ...Y
		 * Note:
		 *		-1 is special value of the x,y coordinates.
		 *		-1 means append
		 *	if x or y >= 0 appropriate cell is found (created if not found) and value
		 *  is set onto that cell
		 *
		 * @param[in]	fmt format for the value to be written
		 * Note:
		 *	Message can set values more cells at once separating them by the
		 *	colum. For example:
		 *		WriteTable(lvl, ctx, 0, -1, "%i|%i|%i|0", a, b, c);
		 *	sets a,b,c to columns of new row and 0 to 4th column
		 *
		 **/
		TRACE_API void WriteTable (level_t level, context_t context, int x, int y, char const * fmt, ...);
		struct Color {
			unsigned char r,g,b,a;
			Color (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff) : r(red), g(green), b(blue), a(alpha) {  }
		};
		TRACE_API void WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, ...);
		TRACE_API void WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...);
		TRACE_API void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, ...);
		TRACE_API void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...);
		TRACE_API void WriteTableSetHHeader (level_t level, context_t context, int x, char const * name, char const * fmt, ...);
		TRACE_API void WriteTableClear (level_t level, context_t context, char const * fmt, ...);


		/**@fn		WriteBgnGanttVA
		 * @brief	write begin to gantt event in form (fmt, va_list) **/
		TRACE_API void WriteGanttBgnVA (level_t level, context_t context, char const * fmt, va_list);

		/**@fn		WriteGanttBgn
		 * @brief	write begin to gantt evet in form (fmt, ...) **/
#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		TRACE_API void WriteGanttBgn (level_t level, context_t context, char const * fmt, ...) __attribute__ ((format(printf, 3, 4) ));
#elif defined _MSC_VER
		TRACE_API void WriteGanttBgn (level_t level, context_t context, char const * fmt, ...);
#endif
		//TRACE_API void WriteGanttBgn (level_t level, context_t context);
		//TRACE_API void WriteGanttEnd (level_t level, context_t context);
		TRACE_API void WriteGanttEnd (level_t level, context_t context, char const * fmt, ...);

#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		TRACE_API void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, ...) __attribute__ ((format(printf, 3, 4) ));
#elif defined _MSC_VER
		TRACE_API void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, ...);
#endif
		TRACE_API void WriteGanttFrameBgn (level_t level, context_t context);
		TRACE_API void WriteGanttFrameEnd (level_t level, context_t context);
		TRACE_API void WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, ...);
		TRACE_API void WriteGanttClear (level_t level, context_t context, char const * fmt, ...);

		/**@class	ScopedGantt
		 * @brief	RAII class for gantt begin on construction and gantt end on destruction **/
		struct ScopedGantt
		{
			level_t m_level;
			context_t m_context;
			char m_tag[256];

			TRACE_API ScopedGantt (level_t level, context_t context, char const * fmt, ...);
			TRACE_API ~ScopedGantt ();
		};
		/**@class	ScopedGanttFrame
		 * @brief	RAII class for gantt begin on construction and gantt end on destruction **/
		struct ScopedGanttFrame
		{
			level_t m_level;
			context_t m_context;
			char m_tag[256];

			TRACE_API ScopedGanttFrame (level_t level, context_t context, char const * fmt, ...);
			TRACE_API ~ScopedGanttFrame ();
		};

		TRACE_API void WriteSound (level_t level, context_t context, float vol, int loop, char const * fmt, ...);
	}

#else // no tracing at all
# include "trace_dummy.h"
#endif // !TRACE_ENABLED

