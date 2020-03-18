BEGIN {
  if (PROCINFO["platform"] == "windows") {
    "find /v \"\"" | getline; print; close("find /v \"\"")
  } else {
    "cat" | getline; print; close("cat")
  }
}
