// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments

int main()
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
  char history[100][MAX_COMMAND_SIZE];
  int histCount = 0, arrayCount = 0, redo = -1;
  pid_t array[100];

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input

    if (redo == -1)   //if there isnt !, then take user input
    {
      while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
    }
    else
    {
      if (histCount > 15 && histCount >= redo)
      {
        strcpy(command_string, history[histCount - (16 - redo)]);
        command_string[strlen(command_string) + 1] = '\0';
        printf("%d is index after 15\n", histCount - (16 - redo));
      }
      else if (histCount <= 15 && histCount >= redo)      //re inserts old command into command_string
      {
        strcpy(command_string, history[redo]);
        command_string[strlen(command_string) + 1] = '\0';
        printf("%d is index\n", redo);
      }
      else
      {
        printf("Command does not exist in history\n");
      }
      redo = -1;
    } 

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this for loop and replace with your shell functionality

    if (token[0] == NULL || strlen(token[0]) == 0)    //if user enters a blank
    {
      continue;
    }
    else
    {
      if (histCount < 100)    //adds commands to history array
      {
        command_string[strlen(command_string) - 1] = '\0';
        strcpy(history[histCount], command_string);
        printf("added to %d\n", histCount);
        histCount++;
      }
      else
      {
        exit(0);
      }
    }

    if (strcmp (token[0], "quit") == 0) //quit command
    {
      exit(0);
    }
    else if (strcmp(token[0], "exit") == 0) //exit command
    {
      exit(0);
    }
    else if (strcmp(token[0], "cd") == 0) //cd command
    {
      chdir(token[1]);
    }
    else if (token[0][0] == '!')    //redo previous commands
    {
      if (isdigit(token[0][1])) 
      {
        char* num = &token[0][1];
        redo = atoi(num);
      }
    }
    else if (strcmp(token[0], "history") == 0)
    {
      array[arrayCount] = -1;
      arrayCount++;

      if(histCount > 15)
      {
        int index = 0;
        for (int i = histCount - 15; i < histCount; i++)
        {
          if(token[1] != NULL && strcmp(token[1], "-p") == 0)   //prints PID history
          {
            printf("%d: %s\n", array[i], history[i]);
          }
          else  //prints history after 15 entries
          {
            printf("%d: %s\n", index, history[i]);
            index++;
          }
        }
      }
      else
      {
        for (int i = 0; i < histCount; i++)
        {
          if (token[1] != NULL && strcmp(token[1], "-p") == 0)  //prints PID history
          {
            printf("%d: %s\n", array[i], history[i]);
          }
          else  //prints history before 15 entries
          {
            printf("%d: %s\n", i, history[i]);
          }
        }
      }

    }
    else
    {
      pid_t pid = fork();
      if (pid == 0)   //if child
      {
        //printf("child\n");
        int work = execvp(token[0], token); 
        if (work == -1)
        {
          printf("%s: Command not found.\n", token[0]);
          break;
        }
      }
      else  //parent, gets the child PID after child runs
      {
        int status;
        wait(&status);
        array[arrayCount] = pid;
        arrayCount++;
      }
    }

    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      if( token[i] != NULL )
      {
        free( token[i] );
      }
    }

    free( head_ptr );

  }

  free( command_string );

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}
