@load "time"

BEGIN {
   delta = 1.3
   printf "gettimeofday - systime = %d\n", (t0 = gettimeofday())-systime()
   printf "sleep(%s) = %s\n",delta,sleep(delta)
   printf "gettimeofday - systime = %d\n", (t1 = gettimeofday())-systime()
   slept = t1-t0
   if ((slept < 0.9*delta) || (slept > 1.3*delta))
      printf "Warning: tried to sleep %.2f secs, but slept for %.2f secs\n",
	     delta,slept
}
