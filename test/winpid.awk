# windows-specific version of pid.awk
# only "pid" is supported
BEGIN {
	pid = PROCINFO["pid"]
	printf "	ProcessId = %d;\n", pid > "winpid.ok"
	printf "done" > "winpid.done"
}
