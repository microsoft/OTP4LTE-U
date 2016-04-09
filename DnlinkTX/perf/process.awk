BEGIN {
    print "All results are normalized w.r.t. maximum run-time, which is 10s (for 1000 LTE frames)";
    print "The higher the number, the worse the performance, and above 1 is unacceptable";
    print "====================================================================================";
  }
/--input=dummy/ {
    filename = substr($1, 3, length($1) - 6);
    println filename;
  }
/--dummy-samples/ {
    x=index($1, "="); 
    nosamples=substr($1, x+1, length($1));
  }

/Time Elapsed:/ {time = $3;
   print filename " " time / 10000000 " (" time ")"
  }



