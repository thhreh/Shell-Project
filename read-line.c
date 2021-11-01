/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];
char right_buffer[MAX_BUFFER_LINE];
int right_length;

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [128];
//  "ls -al | grep x", 
//  "ps -e",
//  "cat read-line-example.c",
//  "vi hello.c",
//  "make",
//  "ls -al | grep xxx | grep yyy"
//};
int history_length = 0;

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n"
    " down arrow     See next command in the history\n"
    " left arrow     move cru to the left\n"
    " right arrow     move cru to the right\n"
    " ctrl D        delete this character\n"
    "ctrl E         go to end of the line\n"
    "ctrl A         go to start of the line\n";

  write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  right_length = line_length;

  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch != 127) {

      // It is a printable character. 

      // Do echo
      write(1,&ch,1);

      // If max number of character reached return.
      if (line_length + right_length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      if(right_length == 0){
        line_buffer[line_length] = ch;
        line_length++;
      }else{
        line_buffer[line_length] = ch;
        line_length++;
        for (int i=right_length-1; i>=0; i--) {
          char c = right_buffer[i];
          write(1,&c,1);
        }
        for (int i=0; i<right_length; i++) {
          char c = 8;
          write(1,&c,1);
        }
      }


    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      if (right_length != 0) {
        for (int i=right_length-1; i>=0; i--) {
          char c = right_buffer[i];
          right_buffer[i] = NULL;
          line_buffer[line_length]=c;
          line_length++;
        }
      }
      history[history_length] = (char *)malloc(128);
      strcpy(history[history_length++], line_buffer);
      //history[history_length-1][strlen(line_buffer)-1] = '\0';
      history_index = history_length-1;


      //if (line_length != 0) {
        //if (history[history_index]==NULL){
         // history[history_index] = (char *)malloc(MAX_BUFFER_LINE);

       // }
       // strcpy(history[history_index], line_buffer);
       // history_prev = history_index;
       // history_index++;
       // if (history_index>=history_length) {
       //   history_index = 0;
       //   history_full = 1;
       // }
      //}

      right_length=0;
      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 4){
      //ctrld
      if (right_length == 0) continue;
      for(int i=right_length-2; i>=0; i--) {
        char c = right_buffer[i];
        write(1,&c,1);
      }
      ch = ' ';
      write(1,&ch,1);
      for (int i=0; i<right_length; i++) {
        char c = 8;
        write(1,&c,1);
      }

      right_length--;
    }
    else if(ch == 5){
      //end of the line ctrl E
      while(right_length != 0){
        write(1,"\033[1C",5);
        line_buffer[line_length]=right_buffer[right_length-1];
        right_length--;
        line_length++;
      }
    }
    else if(ch == 1){
      //go to begining
      while(line_length != 0){
        char c = 8;
        write(1,&c,1);
        right_buffer[right_length] = line_buffer[line_length-1];
        right_length++;
        line_length--;
      }
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if (ch == 8 || ch == 127) {
      // <backspace> was typed. Remove previous character read.
      if(line_length == 0) continue;
      
      if(right_length == 0){
      // Go back one character
        ch = 8;
        write(1,&ch,1);

      // Write a space to erase the last character read
        ch = ' ';
        write(1,&ch,1);

      // Go back one character
        ch = 8;
        write(1,&ch,1);

      // Remove one character from buffer
        line_length--;
      }
      else{
        ch = 8;
        write(1,&ch,1);

        for(int i=right_length-1; i>=0; i--) {
          char c = right_buffer[i];
          write(1,&c,1);
        }
        // Write a space to erase the last character read
        ch = ' ';
        write(1,&ch,1);
        for (int i=0; i<right_length+1; i++) {
          char c = 8;
          write(1,&c,1);
        }
        // Remove one character from buffer
        line_length--;
      }

    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1;
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && (ch2==65 || ch2 == 66)) {

      right_length = 0;
	// Up arrow. Print next line in history.

	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}	


	// Copy line from history
  if(ch2 == 65){
    if(history_length > 0 && history_index >= 0){
      strcpy(line_buffer, history[history_index--]);
      history_index=(history_index)%history_length;
      if(history_index == -1){
        history_index = history_length - 1;
      }
      line_length = strlen(line_buffer);
      right_length = 0;

    }
  }
  else if(ch2 == 66){
    //down arrow
    if(history_length > 0 && history_index <= history_length-1){
      strcpy(line_buffer, history[history_index++]);
      history_index=(history_index)%history_length;
    }
    else if(history_index == history_length){
      history_index = history_length - 1;
      strcpy(line_buffer,"");
    }
    line_length = strlen(line_buffer);
    right_length = 0;
   }
  


	// echo line
	write(1, line_buffer, line_length);
      }
      else if(ch1==91 && ch2==68){
        //left arrow
        if (line_length == 0) continue;
        ch = 8;
        write(1,&ch,1);
        //insert
        right_buffer[right_length] = line_buffer[line_length-1];
        right_length++;
        line_length--;

      }
      else if (ch1==91 && ch2==67) {
      //right arrow
        if (right_length == 0) continue;
        write(1,"\033[1C",5);
        line_buffer[line_length]=right_buffer[right_length-1];
        line_length++;
        right_length--;
      }
      
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;
  
  //history update

  //if (right_length != 0) {
  //   for (int i=right_length-1; i>=0; i--) {
  //      char c = right_buffer[i];
  //           line_buffer[line_length]=c;
  //           line_length++;
   //     }
  //}

  //history[history_length] = (char *)malloc(strlen(line_buffer)*sizeof(char)+1);
  //strcpy(history[history_length++], line_buffer);
  //history[history_length-1][strlen(line_buffer)-1] = '\0';
  //history_index = history_length-1;
  //tty_term_mode();


  return line_buffer;
}

