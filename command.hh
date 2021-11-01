#ifndef command_hh
#define command_hh


#include "simpleCommand.hh"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <regex.h>
#include <pwd.h>
#include <algorithm>
#include <dirent.h>


// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  //decide if we will append to the following file
  bool _append;

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void prompt();
  void print();
  void execute();
  bool BuildinFunc(int i);
  
  static Command _currentCommand;
  static SimpleCommand *_currentSimpleCommand;
};

#endif
