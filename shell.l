
 /*
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"
#include "shell.hh"
//extern “C” char * read_line();
//int mygetc(FILE * f) {
//  static char *p;
  //char ch;
  //if (!isatty(0)) {
   // return getc(f);
  //}

  //if (p==NULL || *p == 0) {
   // char * s = read_line();
   // p = s;

  //}
  //ch = *p;
  //p++;
  //return ch;
//}

#undef getc
#define getc(f) mygetc(f)

static  void yyunput (int c,char *buf_ptr  );

extern int return_code;
extern int pid_last;
extern std::string arg_last;
extern "C" char * read_line();

void myunputc(int c) {
  unput(c);
}

int mygetc(FILE * f) {
  static char *p;
  char ch;

  if (!isatty(0)) {
    return getc(f);

  }
  if (p==NULL || *p == 0) {
  char * s = read_line();
  p = s;

  }
  ch = *p;

  p++;
  return ch;


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


~[^ \n\t]* {
  //tidle
  std::string str = std::string(yytext);
  std::string home_dir = getenv("HOME");
  if (str.size() != 1) {
    if (str[1] == '/') {
      str = home_dir + str.substr(1, -1);
    }
    else {
      str = std::string("/homes/") + str.substr(1,-1);
    }
    yylval.cpp_string = new std::string(str);
  }
  else{
    yylval.cpp_string = new std::string(home_dir);
  }
  return WORD;
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
  close(fdpipeout[0]);

}

$\{[^ \}\n]*\}([^ \n]*|$\{[^ \}\n]*\})* {
  
  std::string variable = yytext;
  //variable expansion
  std::string result = "";
  if (variable.find("${") <= variable.size()){
    while(variable.find("${") <= variable.size()){
      std::string left_substring= variable.substr(0,variable.find("${"));
      result += left_substring;
      std::string right_substring = variable.substr(variable.find("}")+strlen("}"));
      std::string main = variable.substr(left_substring.size()+strlen("${"),variable.find("}")-variable.find("${")-2);
      //check for other cases
      if(main.compare("$") == 0){
        result += std::to_string(getpid());
      }
      else if(main.compare("?") == 0){
        result += std::to_string(return_code);
      }
      else if(main.compare("!") == 0){
        result += std::to_string(pid_last);
      }
      else if(main.compare("_") == 0){
        result += arg_last;
      }
      else if(main.compare("SHELL") == 0){
        char real_path[2048];
        result += realpath("/proc/self/exe",real_path);

      }
      else{
        char * temp = getenv(main.c_str());
        if(!temp) {
          result += main.c_str();
        }else{
          result += temp;
        }
      }
      variable = right_substring;
    }
    result += variable;
  }
  else{
    result += variable;
  }
  yylval.cpp_string = new std::string(result);
  return WORD;
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


