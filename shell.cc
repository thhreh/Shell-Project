#include <cstdio>

#include "shell.hh"


void yyrestart(FILE *file);
int yyparse(void);

void Shell::prompt() {
  if (isatty(0)) {
    printf("myshell>");
  }
  fflush(stdout);
}

extern "C" void signalHandle(int sig) {
  printf("\n");
  Shell::prompt();
  
}

extern "C" void zombieHandle(int sig) {
  wait3(0, 0, NULL);
  while (waitpid(-1, NULL, WNOHANG) > 0);

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
  if (Shell::_currentCommand._background == true) {
    struct sigaction Zombie;
    Zombie.sa_handler = zombieHandle;
    sigemptyset(&Zombie.sa_mask);
    Zombie.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &Zombie, NULL)) {
      perror("sigaction");
      exit(2);
    }
  }

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
