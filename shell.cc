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
  int ret = wait3(0, 0, NULL);
  printf("[%d] exited.\n", pid);
  while (waitpid(-1, NULL, WNOHANG) > 0) {};

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
    sigZombie.sa_handler = zombieHandle;
    sigemptyset(&sigZombie.sa_mask);
    sigZombie.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sigZombie, NULL)) {
      perror("sigaction");
      exit(2);
    }
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
