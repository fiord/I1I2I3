#ifndef INCLUDE_GUARD_DIE_HPP
#define INCLUDE_GUARD_DIE_HPP
#include <stdio.h>
#include <stdlib.h>

void die(char *s) {
  std::perror(s);
  std::exit(-1);
}
#endif // INCLUDE_GUARD_DIE_HPP
