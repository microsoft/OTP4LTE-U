BEGIN {
    print "All results are normalized w.r.t. maximum run-time per sample (which is 1/30.72MHz)";
    print "The higher the number, the worse the performance, and above 1 is unacceptable";
    print "====================================================================================";
  }
/--input=dummy/ {
    filename = substr($1, 3, length($1) - 6);
    println filename;
  }
/--dummy-samples/ {
    x=index($3, "="); 
    nosamples=substr($3, x+1, length($3));
  }

/Time Elapsed:/ {time = $3;
    print filename " " 30.72 / (nosamples / time) #" " nosamples " " time
  }



