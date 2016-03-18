
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

#define JUS_RL_BUFSIZE 1024
#define JUS_TOK_BUFSIZE 64
#define JUS_TOK_DELIM " \t\r\n\a"

int jus_chdir(char **args);
int jus_help(char **args);
int jus_exit(char **args);
int jus_getenv(char **args);
int jus_setenv(char**args);
int jus_echo(char **args);

//builtin command list
char *builtin_str[] = {
  "chdir",
  "help",
  "exit",
  "getenv",
  "setenv",
  "cd",
  "echo"
};

int (*builtin_func[]) (char **) = {
  &jus_chdir,
  &jus_help,
  &jus_exit,
  &jus_getenv,
  &jus_setenv,
  &jus_chdir,
  &jus_echo
};

int jus_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int jus_chdir(char **args)
{
  if (args[1] == NULL) {
    setenv("PWD", getenv("HOME"),1);
  } 
  else 
  {
    if (chdir(args[1]) != 0) {
      perror("chdir");
    }
  }
  return 1;
}

int jus_help(char **args)
{
  int i;
  printf("Built in functions: \n");

  for (i = 0; i < jus_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int jus_exit(char **args)
{

  if (args[1] != NULL)
  {
    printf("ERROR: exit should only have one integer argument or none\n");
    return 1;
  }
  
    return 0;
}

int jus_getenv(char **args)
{
  
  if(getenv(args[1]) == NULL)
  {
    printf("\n");
  }

  else if(args[2] != NULL)
  {
    printf("ERROR: more than one operand specified\n");
  }

  else
  {
    printf("%s\n", getenv(args[1]));
  }
  return 1;
}

int jus_setenv(char **args)
{


printf("remember must be in this format: setenv <key> = <value>\n");
printf("all other arguments will be ignored\n");

setenv(args[1], args[3],1); 

  

  return 1;

}

int jus_echo(char **args)
{

  int count = 1;
  while (args[count] != NULL )
  {
    printf("%s\n", args[count]);
    count = count + 1;
  }
  return 1;

}

int jus_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) 
  {
    //there is an error if exec returns -1 or nothing
      //this is for the child process
    if (execvp(args[0], args) == -1) 
    {
      perror("launh");
    }
    exit(EXIT_FAILURE);
  } 
    //this is checking if fork had an error
  //it is up to the user to decide to quit
  else if (pid < 0) 
  {
  
    perror("launch");
  } 

  //this means that fork executed successfully.   
  else
   {
  
    do 
    {
      //waitpid is used to wait for the process state
    //to change
      waitpid(pid, &status, WUNTRACED);

    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int jus_execute(char **args)
{
  int i;

  if (args[0] == NULL) 
  {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < jus_num_builtins(); i++) 
  {
    if (strcmp(args[0], builtin_str[i]) == 0) 
    {
      return (*builtin_func[i])(args);
    }
  }

  return jus_launch(args);
}

char *jus_Readline(void)
{
  int bufsize = JUS_RL_BUFSIZE;
  int pos = 0;
  char *buf = malloc(sizeof(char) * bufsize);
  int c; //c is an int because EOF is an int

  if (!buf) 
  {
    //we use fprintf because it writes to the output stream we specify
      //ie stderr
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) 
  {
    // taking in a character
    c = getchar();

    // checking to see if c hit the end of line
    if (c == EOF || c == '\n') 
    {
      buf[pos] = '\0';
      return buf;
    } else 
    {
      buf[pos] = c;
    }
    pos++;

    // check to see if we have exceeded the buffer
    if (pos >= bufsize) 
    {
      bufsize += JUS_RL_BUFSIZE;
      buf = realloc(buf, bufsize);
      if (!buf) 
      {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
      }
    }
}

char **jus_Parseline(char *line)
{
  int bufsize = JUS_TOK_BUFSIZE, pos = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }
  //strtok will return pointers to the string we are providing
  //in addition it places \0 bytes at the end of each token
  token = strtok(line, JUS_TOK_DELIM);
  while (token != NULL) 
  {
    tokens[pos] = token;
    pos++;

    if (pos >= bufsize) 
    {
      bufsize += JUS_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) 
      {
  free(tokens_backup);
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    //null terminate our list of tokens
    token = strtok(NULL, JUS_TOK_DELIM);
      }
  tokens[pos] = NULL;
  return tokens;
}

void jusLoop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("Hello, please enter a command> ");
    line = jus_Readline();
    args = jus_Parseline(line);
    status = jus_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  //config file and run command loop.
  jusLoop();

  return EXIT_SUCCESS;
}

