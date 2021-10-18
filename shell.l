
 /*
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

"2>" {
  return TWOGREAT;
}

"|" {
  return PIPE;
}

">" {
  return GREAT;
}

">>" {
  return GREATGREAT;
}

"<" {
  return LESS;
}

"&" {
  return AMPERSAND;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

\$\([^\n]*\) {
  //subshell

  std::string str = yytext;
  //remove $ 
  if (str.at(0) == '$') {
      str = str.substr(1,str.size()-1);
  }
  str = str.substr(1,str.size()-2);
  //remove paren
  std::size_t index = str.find(")");
  std::string temp_string = str.substr(0,index-2);

  char *subshell = const_cast<char*> (temp_string.c_str());
  int defaultin = dup(0);
  int defaultout = dup(1);
  //pipe
  int fdpipein[2];
  pipe(fdpipein);
  int fdpipeout[2];
  pipe(fdpipeout);

  write(fdpipein[1], subshell, strlen(subshell));
  write(fdpipein[1], "\n", 1);
  close(fdpipein[1]);

  dup2(fdpipein[0], 0);
  close(fdpipein[0]);
  dup2(fdpipeout[1], 1);
  close(fdpipeout[1]);
  
  int ret = fork();
  if (ret == 0) {
    char ** null_ptr = NULL;
    execvp("/proc/self/exe", null_ptr);
    _exit(1);
  } 
  else if (ret < 0) {
    perror("fork");
    exit(1);
  }
  dup2(defaultin, 0);
  dup2(defaultout, 1);
  close(defaultin);
  close(defaultout);

  char c;
  char * buffer = (char *) malloc (4096);
  int i = 0;
  //transfer data to buffer
  while (read(fdpipeout[0], &c, 1)) {
    if (c == '\n') {
      buffer[i++] = ' ';
    }
    else{
      buffer [i++] = c;
    }
  }
  buffer[i] = '\0';

  int j = i - 1;
  for (j = i - 1; j >= 0; j--) {
    myunputc(buffer[j]);
  }

}





["][^\n\"]*["] {
  //Quotes
  yylval.cpp_string = new std::string(yytext);
  //printf("before, string = %s\n",yylval.cpp_string);
  *yylval.cpp_string = yylval.cpp_string->substr(1, yylval.cpp_string->length()-2);
  //printf("after, string = %s\n",yylval.cpp_string);
  return WORD;
}

([^ \t\n<>\|\&\"\(\)]|\\.)+ {
  //escape
  //temp_string
  char * str = strdup(yytext);
  std::string temp_string = "";
  for(int index = 0; yytext[index] != '\0'; index++){
    if(!(yytext[index] =='\\' && yytext[index+1] != ' ')){
      temp_string += yytext[index];
    }
  }
  yylval.cpp_string = new std::string(temp_string);
  temp_string.clear();
  return WORD;
}

source([\ ][^ \n\t]*[^\ \n\t]) {

  //source call
  std::string _file = yytext;
  //printf("BUY!");
  _file = _file.substr(7, _file.size()-7);

  //open file for read
  FILE * file = fopen(_file.c_str(), "r");

  if(file != NULL){
    YY_BUFFER_STATE state = yy_create_buffer(file, YY_BUF_SIZE);
    yypush_buffer_state(state);
    yyparse();
    yypop_buffer_state();
    fclose(file);
    _file.clear();

  }
}


[^ \t\n|><][^ \t\n|><]*  {
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}

.  {
    return NOTOKEN;

}


