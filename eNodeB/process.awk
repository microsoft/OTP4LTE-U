/--TX-input=dummy/ {
    filename = substr($1, 3, length($1) - 6);
  }

/TX Time Elapsed:/ {time = $4;
    print "TX " filename " " time
  }

/RX Time Elapsed:/ {time = $4;
   print "RX " filename " " time
  }



