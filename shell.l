
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


[^ \t\n|><][^ \t\n|><]*  {
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}

.  {
    return NOTOKEN;

}


