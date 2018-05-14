#ifndef LIBTAG_HPP
#define LIBTAG_HPP

#include <istream>
#include <ostream>
#include <string>

enum struct lib {
  lf,
  boost
};

inline
std::string to_str(lib tag) {
  return tag == lib::lf ? "lf" : "boost";
}

inline
std::ostream& operator<<(std::ostream& os, lib tag) {
  return os << to_str(tag);
}

inline
std::istream& operator>>(std::istream& is, lib& tag) {
  std::string s;
  if (is >> s) {
    if (s == "lf") tag = lib::lf;
    else if (s == "boost") tag = lib::boost;
    else is.setstate(is.failbit);
  }
  return is;
}

#endif // LIBTAG_HPP
