BEGIN {
      URL = ARGV[1]; ARGV[1] = ""
      if (Method    == "")  Method    = "GET"
      HttpService = "/inet/tcp/0/www.yahoo.com/80"
      ORS = RS = "\r\n\r\n"
      print Method " " URL "/index.html HTTP/1.0" |& HttpService
      HttpService                      |& getline Header
      print Header > "/dev/stderr"
      while ((HttpService |& getline) > 0)
	  printf "%s", $0
      close(HttpService)
  }
