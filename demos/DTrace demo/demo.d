#! /usr/sbin/dtrace -C -s
#pragma D option quiet

#source: http://www.brendangregg.com/DTrace/dtrace_oneliners.txt


# New processes with arguments
proc:::exec-success { trace(curpsinfo->pr_psargs); }

# Files opened by process
syscall::open*:entry { printf("%s %s",execname,copyinstr(arg0)); }

# Syscall count by program
syscall:::entry { @num[execname] = count(); }

# Successful signal details,
proc:::signal-send /pid/ { printf("%s -%d %d",execname,args[2],args[1]->pr_pid); }
