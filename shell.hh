#ifndef shell_hh
#define shell_hh

#include "command.hh"

struct Shell {

  static void prompt();

  static Command _currentCommand;

  std::vector<int> Shell::_bgPIDs;
};

#endif
