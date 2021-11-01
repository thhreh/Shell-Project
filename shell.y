
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires
{
#include <string>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE LESS AMPERSAND PIPE GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void yyerror(const char * s);
void expandWildcard(char * prefix, char * suffix);
void expandWildcardsIfNecessary(std::string * arg);
bool string_equality (char * a, char * b);

int yylex();
static std::vector<char *> _sortArgument = std::vector<char *>();


bool wildCard;
bool find=false;

%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;
//simple command contain three parts
simple_command:	
  pipe_list io_modifier_list background_optional NEWLINE {
   // printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE
  | error NEWLINE { yyerrok; }
  ;


pipe_list:
  command_and_args
  | pipe_list PIPE command_and_args
  ;

io_modifier_list:
  io_modifier_list iomodifier_opt
  | iomodifier_opt
  |
  ;

background_optional:
  AMPERSAND {
    Shell::_currentCommand._background = true;
  }
  |/*can be empty"*/
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    wildCard = false;
    char *p = (char *)"";
    expandWildcard(p, (char *)$1->c_str());
    std::sort(_sortArgument.begin(), _sortArgument.end(), string_equality);
    for (auto a: _sortArgument) {
      std::string * argToInsert = new std::string(a);
      Command::_currentSimpleCommand->insertArgument(argToInsert);
    }
    _sortArgument.clear();


  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());

    if ( strcmp($1->c_str(), "exit") == 0 ) {
      printf("Good Bye!!\n");
      exit(1);
    }
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

iomodifier_opt:
  GREAT WORD {
    
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    if (Shell::_currentCommand._outFile != NULL ){
      printf("Ambiguous output redirect.\n");
      exit(0);
    }
    Shell::_currentCommand._outFile = $2;
  }//greatergreater ampersand 
  | GREATGREATAMPERSAND WORD {
      //use different string for err and out to aviod doubel free
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      if (Shell::_currentCommand._outFile != NULL ){
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      std::string *errFile = new std::string($2->c_str());
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = errFile;
      Shell::_currentCommand._append = true;
  }
  | GREATAMPERSAND WORD {
      //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
      std::string *errFile = new std::string($2->c_str());
      if (Shell::_currentCommand._outFile != NULL ){
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = errFile;
  } //input
  | LESS WORD {
      //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
      if (Shell::_currentCommand._outFile != NULL ){
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      Shell::_currentCommand._inFile = $2;
  }
  | GREATGREAT WORD {
      //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
      if (Shell::_currentCommand._outFile != NULL ){
        printf("Ambiguous output redirect.\n");
        exit(0);
      }
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._append = true;
  }
  || TWOGREAT WORD{
    //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
    Shell::_currentCommand._errFile = $2;
  }
  ;



%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

bool string_equality (char * a, char * b) { return strcmp(a,b)<0; }

void expandWildcard(char * prefix, char * suffix) {
  //prefix are part dealt with, suffix are lefted outa
  //empty case
  if (suffix[0] == 0) {
    _sortArgument.push_back(strdup(prefix));
    return;
  }
  //setup and check for edges
  char Prefix[1024];
  char newPrefix[1024];
  if (prefix[0] == 0) {
    if (suffix[0] == '/') {
      suffix += 1;
      sprintf(Prefix, "%s/", prefix);
    }
    else{
      strcpy(Prefix, prefix);
    }
  }
  else{
    sprintf(Prefix, "%s/", prefix);
  }
  //dealing with next component of suffix
  char * s = strchr(suffix, '/');
  char component[1024];
  //break it to two
  if (s != NULL) {
    strncpy(component, suffix, s-suffix);
    component[s-suffix] = 0;
    suffix = s + 1;
  }
  else {
    strcpy(component, suffix);
    suffix = suffix + strlen(suffix);
  }

  //recurively traverse trough directories, if component contain no char wildcard
  if (strchr(component,'?')==NULL & strchr(component,'*')==NULL) {
    if (Prefix[0] == 0){
      strcpy(newPrefix, component);
    }
    else{
      sprintf(newPrefix, "%s/%s", prefix, component);
    }
    expandWildcard(newPrefix, suffix);
    return;
  }

  //if they have it

  char * reg = (char*)malloc(2*strlen(component)+10);
  char * r = reg;
  *r = '^'; r++;
  int i = 0;
  while (component[i]) {
    if (component[i] == '*') {
      *r='.'; r++; *r='*'; r++;
    }
    else if (component[i] == '?') {
      *r='.'; r++;
    }
    else if (component[i] == '.') {
      *r='\\'; r++; *r='.'; r++;
    }
    else {
      *r=component[i]; r++;
    }
    i++;
  }

  //reg holder
  *r='$'; r++; *r=0;
  regex_t re;
  int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);

  char * dir;
  if (Prefix[0] == 0){
    dir = (char*)".";
  }
  else{ 
    dir = Prefix;
  }
  //directory for recursion
  DIR * d = opendir(dir);
  if (d == NULL) {
    return;
  }

  struct dirent * ent;
  while ((ent = readdir(d)) != NULL) {
    if(regexec(&re, ent->d_name, 1, NULL, 0) == 0) {
      find = true;
      if (Prefix[0] == 0){
        strcpy(newPrefix, ent->d_name);
      }
      else {
        sprintf(newPrefix, "%s/%s", prefix, ent->d_name);
      }

      //check for hidden file
      if (reg[1] == '.') {
        if (ent->d_name[0] != '.') expandWildcard(newPrefix, suffix);
      } else{
        expandWildcard(newPrefix, suffix);
      }
    }
  }
  if (!find) {
    if (Prefix[0] == 0){
      strcpy(newPrefix, component);
    }
    else {
      sprintf(newPrefix, "%s/%s", prefix, component);
    }
    expandWildcard(newPrefix, suffix);
  }
  closedir(d);
  regfree(&re);
  free(reg);




}

void expandWildcardsIfNecessary(std::string * arg){
  char * arg_c = (char *)arg->c_str();
  if (strchr(arg_c,'?')==NULL & strchr(arg_c,'*')==NULL) {
    Command::_currentSimpleCommand->insertArgument(arg);
    return;
  }
  //converting reg
  char * a;
  std::string path;
  DIR * dir;
  if (arg_c[0] == '/') {
    std::size_t found = arg->find('/');
    while (arg->find('/',found+1) != -1) {
      found = arg->find('/', found+1);
    }
    path = arg->substr(0, found+1);
    a = (char *)arg->substr(found+1, -1).c_str();
    dir = opendir(path.c_str());


  }
  else{
    dir = opendir(".");
    a = arg_c;
  }
  if (dir == NULL) {
    perror("opendir");
    return;
  }
  //save reg
  char * reg = (char*)malloc(2*strlen(arg_c)+10);
  char * r = reg;
  *r = '^'; r++;
  while (*a) {
    if (*a == '*') {
      *r='.'; r++; *r='*'; r++;
    }
    else if (*a == '?') {
      *r='.'; r++;
    }
    else if (*a == '.') {
      *r='\\'; r++; *r='.'; r++;
    }
    else {
      *r=*a; r++;
    }
    a++;
  }
  *r='$'; r++; *r=0;
  regex_t re;
  //compile reg calculated
  int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
  if (expbuf != 0) {
    perror("regcomp");
    return;
  }
  //break down directory and  visit,sort directory entries
  std::vector<char *> sortArgument = std::vector<char *>();
  struct dirent * ent;
  while ( (ent=readdir(dir)) != NULL) {
    if (regexec(&re, ent->d_name, 1, NULL, 0) == 0) {
      if (reg[1] == '.') {
        if (ent->d_name[0] != '.') {
          std::string name(ent->d_name);
          name = path + name;
          sortArgument.push_back(strdup((char *)name.c_str()));
        }
      }
      else{
        std::string name(ent->d_name);
        name = path + name;
        sortArgument.push_back(strdup((char *)name.c_str()));
      }
    }
  }
  closedir(dir);
  regfree(&re);

  std::sort(sortArgument.begin(), sortArgument.end(), string_equality);

  for (auto a: sortArgument) {
    std::string * argToInsert = new std::string(a);
    Command::_currentSimpleCommand->insertArgument(argToInsert);
  }

  sortArgument.clear();
}


#if 0
main()
{
  yyparse();
}
#endif
