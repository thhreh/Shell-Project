/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "command.hh"
#include "shell.hh"


Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
    _append = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }

    // Print contents of Command data structure

    //print();

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    int defaultin = dup(0);
    int defaultout = dup(1);
    int defaulterr = dup(2);

    //initialize input
    int fdin;
    int fdout;
    int fderr;

    //initialize inputs

    if (_inFile) {
      const char* myinfile = _inFile->c_str();
      fdin = open(myinfile, O_RDONLY);
    }
    else{
      fdin = dup(defaultin);
    }

    if(_errFile) {
      const char* errfile = _errFile->c_str();
      if(_append) {
        fderr = open(errfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
      }
      else {
        fderr = open(errfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
      }
    } else {
      fderr = dup(defaulterr);
    }

    //redirect err

    dup2(fderr, 2);
    close(fderr);

    //treverse trough simple commands
    int ret;
    for (size_t i = 0; i < _simpleCommands.size(); i++) {
      dup2(fdin, 0);
      close(fdin);
      if (i == _simpleCommands.size() - 1) {
        //when last simple
        if(_outFile ){
          const char* myoutfile = _outFile->c_str();
          //check for the way to write file
          if(_append){
            fdout = open(myoutfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
            }
            else {
              fdout = open(myoutfile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
            }
        } else if(!_outFile){
          fdout = dup(defaultout);
        }
        if(_errFile){
          const char* errfile = _errFile->c_str();
          if(_append){
            fderr = open(errfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
          }
          else {
            fderr = open(errfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
          }
        } else if(!_errFile){
          fderr = dup(defaulterr);
        }
        dup2(fderr, 2);
        close(fderr);
      }
      else {
      //if it is not the last command, pipe
        int fdpipe[2];
        pipe(fdpipe);
        fdout = fdpipe[1];
        fdin = fdpipe[0];
      }

      dup2(fdout, 1);
      close(fdout);


      //fork child program
      ret = fork();
      size_t argsize = _simpleCommands[i]->_arguments.size();
      if (ret == -1) {
         perror("fork\n");
         exit(2);
      }
      else if(ret == 0) {
            //Convert std::vector<std::string *> _arguments into char**
        char ** x = new char*[argsize+1];
        for (size_t j = 0; j<argsize;j++){
          x[j] = (char *)_simpleCommands[i]->_arguments[j]->c_str();
          x[j][strlen(_simpleCommands[i]->_arguments[j]->c_str())] = '\0';

        }
        x[argsize] = NULL;
        execvp(_simpleCommands[i]->_arguments[0]->c_str(), x);
        exit(1);
      }
    }
    //redirect stdout
    dup2(defaultin,0);
    dup2(defaultout,1);
    dup2(defaulterr,2);
    close(defaultin);
    close(defaultout);
    close(defaulterr);
    //check for &
    if (!_background) {
      waitpid(ret, NULL, 0);
    }
    // Clear to prepare for next command
    clear();

    // Print new prompt
    Shell::prompt();

}

SimpleCommand * Command::_currentSimpleCommand;

