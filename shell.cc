#include <cstdio>

#include "shell.hh"


void yyrestart(FILE *file);
int yyparse(void);

void Shell::prompt() {
  //if (isatty(0)) {
    //printf("myshell>");
  //}
  fflush(stdout);
}
//when ctrl C is processing 

extern "C" void signalHandle(int sig){
    printf("\n");
    Shell::prompt();
} 
// one message a time
extern "C" void zombie(int sig) {
  pid_t pid = waitpid(-1, NULL, WNOHANG);
  for (unsigned i=0; i<Shell::_PIDs.size(); i++) {
    if (pid == Shell::_PIDs[i]) {
      printf("[%d] exited\n", pid);
      Shell::_PIDs.erase(Shell::_PIDs.begin()+i);
      break;
    }
  }

}

int main() {
  //if(isatty(STDIN_FILENO)) {
    //system("/homes/tbagwel/cs252/lab3-src/.shellrc");
  //}
  //ctrl C handle
  struct sigaction sig;
  sig.sa_handler = signalHandle;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sig, NULL)){
    perror("sigaction");
    exit(2);
  }

  //when background is true, handle zombie
  struct sigaction Zombie;
  Zombie.sa_handler = zombie;
  sigemptyset(&Zombie.sa_mask);
  Zombie.sa_flags = SA_RESTART;
  


  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
std::vector<int> Shell::_PIDs;
