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
  if (sig == SIGINT) {
    printf("\n");
    Shell::prompt();
  }
  if (sig == SIGCHLD) {
    if (Shell::_currentCommand._background == true) {
      int pid = wait3(0, 0, NULL);
      printf("[%d] exited.\n", pid);
      while (pid > 0) {
        pid = waitpid(-1, NULL, WNOHANG)
      }
    }
  }
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

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
std::vector<int> Shell::_bgPIDs;
