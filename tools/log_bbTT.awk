#!/usr/bin/gawk

BEGIN {

	m_scope_print = 1;

	m_bg[0] = "\033[0;40m"; # blck
	m_bg[1] = "\033[0;41m"; # r
	m_bg[2] = "\033[0;42m"; # g
	m_bg[3] = "\033[0;43m"; # y
	m_bg[4] = "\033[0;44m"; # b
	m_bg[5] = "\033[0;45m"; # m
	m_bg[6] = "\033[0;46m"; # c
	m_bg[7] = "\033[0;47m"; # w
	m_bg_def = "\033[0;49m"; # default

    m_fg[0] = "\033[0;30m"; # blck
    m_fg[1] = "\033[0;31m"; # r
    m_fg[2] = "\033[0;32m"; # g
    m_fg[3] = "\033[0;33m"; # y
    m_fg[4] = "\033[0;34m"; # b
    m_fg[5] = "\033[0;35m"; # m
    m_fg[6] = "\033[0;36m"; # c
    m_fg[7] = "\033[0;37m"; # wht
    m_fg_def = "\033[0;39m"; # default


    m_colormap[0] = m_fg_def
    m_colormap[1] = m_fg[2]
    m_colormap[2] = m_fg[3]
    m_colormap[3] = m_fg[5]
    m_colormap[4] = m_fg[6]
    m_colormap[5] = m_fg[4]
    m_colormap[6] = m_fg[7]
    m_colormap[7] = m_bg[4]m_fg[3]
    m_colormap[8] = m_bg[4]m_fg[7]
    maxcolors = 9;

    m_threadid[0] = 0;
    m_last_thread = 0;

    m_scope_level = 0;
    m_indenttab[0]=0;


    FS="|";
	printf("%s%slog_filter.awk%s\n",m_bg_def, m_colormap[1], m_colormap[0]);
}

function indentText(thread_index, tmp)
{
    tmp = "";
    for(i = 0; i < m_indenttab[thread_index]; ++i)
    {
        tmp = tmp" ";
    }
    return tmp;
}

function incrIndent(thread_index)
{
    return ++m_indenttab[thread_index];
}

function decrIndent(thread_index)
{
    return --m_indenttab[thread_index];
}

function replaceText(text, tmp)
{
    tmp = "";
    gsub("ue::sys::threads::", "UE::", text)
    gsub("ue::ai::pathfind", "pf", text)
    gsub("virtual ", "V ", text)
	t = m_bg[2]"_"m_bg_def
    gsub("error ", t, text)
    gsub("###", "\033[0;1m###\033[0;22m", text)
    gsub("ERR", "\033[0;9m!!!\033[0;29m", text)
    return text;
}


{
# |date|pid|tid|file:line|fn|{pfn
#   1    2   3      4     5   6

	if (match($3, "[[:digit:]]+"))
	{
		found = -1;
		for (i=0; i<m_last_thread; ++i)
		{
			#printf("cmp%d [[ %s with %s ]]\n", i, $3, m_threadid[i]);
			if (m_threadid[i] == $3)
				found = i;
		}

		if (found == -1)
		{
			found = m_last_thread;
			m_threadid[m_last_thread] = $3;
			m_indenttab[m_last_thread] = 0;
			++m_last_thread;
		}

		#printf("colormap = %d for threadid=%d \n", m_colormap[found], m_threadid[found]);
		scope_detected = 0

		if (index($6,"}"))
		{
			decrIndent(found);
			scope_detected = 1;
		}
		if (index($6,"{"))
		{
			scope_detected = 1;
		}

		strout = "";

		fill = indentText(found);

		msg = replaceText($6);

		date = $1;
		gsub("[0-9]*-[a-zA-Z]*-[0-9]* ", "", date)

		if (scope_detected)
		{
			if (m_scope_print)
			{
				printf("%s%s|%02i|%s%s\n", m_colormap[found], date, found, fill, msg);
			}
		}
		else
			printf("%s%s|%02i|%s%s\n", m_colormap[found], date, found, fill, msg);

		if (index($6,"{"))
			incrIndent(found);
	}
	else
	{
		printf("* %s%s\n", m_colormap[0], $0);
	}

	fflush();
}

END {
	for (i=0; i<m_last_thread; ++i)
	{
		printf("thread map: %i -> %d (%x)\n", i, m_threadid[i], m_threadid[i]);
	}
}
