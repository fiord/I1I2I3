#include "die.hpp"

void die(char *s) {
  perror(s);
  std::exit(1);
}
