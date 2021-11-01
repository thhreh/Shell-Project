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
//when ctrl C is processing 

extern "C" void signalHandle(int sig){
    printf("\n");
    Command::_currentCommand.prompt();
} 
// one message a time
extern "C" void zombie(int sig) {
  int pid = wait3(0, 0, NULL);
  while(waitpid(-1,NULL,WNOHANG)>0){};
}

int main() {
  
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
  //if(Shell::_currentCommand._background == true) {
    struct sigaction Zombie;
    Zombie.sa_handler = zombie;
    sigemptyset(&Zombie.sa_mask);
    Zombie.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &Zombie, NULL)) {
      perror("sigaction");
      exit(2);
    }
  //}

  FILE* fd = fopen(".shellrc", "r");
    if (fd) {
      yyrestart(fd);
      yyparse();
      yyrestart(stdin);
      fclose(fd);
    }
    else{
      Shell::_currentCommand.prompt();
    }


  yyparse();
}

//Command Shell::_currentCommand;
