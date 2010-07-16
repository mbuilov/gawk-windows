# Bug reported by Ulrich Drepper.
BEGIN {
  printf("%'d %d\n", 1000000, 1000000)
}
