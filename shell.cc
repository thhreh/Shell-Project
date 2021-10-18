#include <cstdio>

#include "shell.hh"


void yyrestart(FILE *file);
int yyparse(void);

void Shell::prompt() {
  //printf("myshell>");
  
  fflush(stdout);
}

extern "C" void signalHandle(int sig) {
  Shell::_currentCommand.clear();
  printf("\n");
  Shell::prompt();
  
}

int main() {
  struct sigaction sig;
  sig.sa_handler = signalHandle;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = SA_RESTART;



  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
