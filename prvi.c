#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
int lsh_cd(char **args);
int lsh_help(char **args);
int path(char **args);
int lsh_exit(char **args);
int lsh_echo(char **args);
int lst(char **args);
int echo(char **args);
int clr(char **args);
int touch(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "path",
  "lst",
  "echo",
  "clr",
  "touch"

};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &path,
  &lst,
  &echo,
  &clr,
  &touch
};


int lsh_num_builtins() 
{
  return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}
int lsh_help(char **args)
{
  int i;
  printf("MArija VUjcic's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");
  for (i = 0; i < lsh_num_builtins(); i++)
  {
    printf("  %s\n", builtin_str[i]);
  }
}
int touch(char **args)
{
	if (args[1] == NULL)
        {
		fprintf(stderr, "lsh_touch: expected argument to \"touch\"\n");
	}
	else {
		if (creat(args[1], 0755) != 0) {
			perror("lsh");
		}
	}
	return 1;
	
}
int lsh_exit(char **args)
{
        printf("\n\nRegards ,until next time ! :)\n\n");
  return 0;
}
int echo(char** args) 
{
  int i = 1;
  while(args[i] != NULL) 
  {
    printf("%s ", args[i]);
    i++;
  }
  printf("\n");
  return 1;
}

int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) 
  {
    // Child process
    if (execvp(args[0], args) == -1)
    {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0) 
  {
    // Error forking
    perror("lsh");
  }
  else
  {
    // Parent process
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL)
 {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++)
  {
    if (strcmp(args[0], builtin_str[i]) == 0)
    {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}
int path( char **args)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\n %s\n", cwd);
return 1;
}
//function for listing current dir or  dir with given name
int lst(char **args)
{
        DIR *dir;
        struct dirent *d;
        if(args[1]==NULL)
        {
                dir=opendir(".");
        }
        else
        {

               
               if( (dir=opendir(args[1]))== NULL)
			   {
				   printf("Direktorij ne postoji %s !",args[1]);
				   return 1;
			   }
        }
       
         while((d=readdir(dir))!=NULL)
                {
                printf("---> %s\n",d->d_name);
                }
        
        closedir(dir);
        return 1;

}

//function for clearing window
int clr(char **args)
{
		system("clear");
		return 1;
}	


void init_shell()
{
    printf("\n\t****Greetings, welcome to MV shell ****");
    char* username = getenv("USER");
    printf("\n\n\nUSER : @%s", username);
    printf("\n");


}


//reading the line ,checking is it 
#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;
  if (!buffer)
  {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    c = getchar();
    if (c == EOF)
    {
      exit(EXIT_SUCCESS);
    }
    else if (c == '\n')
    {
      buffer[position] = '\0';
      return buffer;
    }
    else
    {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize)
    {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer)
      {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

//splitting line in tokens
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens)
 {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  printf("%s",token);
  while (token != NULL)
 {
    tokens[position] = token;
    position++;

    if (position >= bufsize)
   {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
     if (!tokens) 
      {
                free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
   }

    token = strtok(NULL, LSH_TOK_DELIM);
 }
  tokens[position] = NULL;
  return tokens;
}


//loop for printng command line , calling lsh_read_line for reading, split for splitting and execute to do command
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;
  char cwd[100];
  getcwd(cwd,sizeof(cwd));
  char *username =getenv("USER");

  do
  {
    getcwd(cwd,sizeof(cwd));
    printf("%s shellu-@%s >>",cwd ,username);
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

//main function clearing screen, calling initalization of shell
int main(int argc, char **argv)
{
  system("clear");
  init_shell();
  lsh_loop();

  return EXIT_SUCCESS;
}

