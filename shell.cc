#include <cstdio>

#include "shell.hh"


void yyrestart(FILE *file);
int yyparse(void);

void Shell::prompt() {
  //printf("myshell>");
  
  fflush(stdout);
}

extern "C" void signalHandle(int sig) {
  printf("\n");
  Shell::prompt();
  
}

int main() {
  struct sigaction sig;
  sig.sa_handler = signalHandle;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = SA_RESTART;
  if(sigaction(SIGINT, &sig, NULL)){
    perror("sigaction");
    exit(2);
  }

  FILE*fd = fopen(".shellrc", "r");
  if (fd) {
    yyrestart(fd);
    yyparse();
    yyrestart(stdin);
    fclose(fd);
  }
  else{

    Shell::prompt();
  }
  yyparse();
}

Command Shell::_currentCommand;
