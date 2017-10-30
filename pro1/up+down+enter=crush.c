#include <unistd.h>
#include <sys/dir.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <memory.h>
#include <termios.h>
#include <ctype.h>


void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}



typedef struct command
{
  char com[512];
  char number_show[2];
  char number_count[2];
  struct command* preCommand;
  struct command* nextCommand; 
}Com;

void current_dir_at_first(char* current_path)
{
  write(1, "JERRY@ashell:~", 14);
  write(1, current_path, strlen(current_path) );
  write(1, " $$>  ", 6);
}

int main()
{
  char* cd = "cd";
  char* ls = "ls";
  char* pwd = "pwd";
  char* exit = "exit";
  char* history = "history";
  static char current_path[512];
  getcwd(current_path, sizeof(current_path));
  
  int one(const struct dirent *unused)
  {
    return 1;
  }
    
  current_dir_at_first(current_path); 
  struct command *head, *tail, *p;
  static char temp_command[512];
//  size = sizeof(struct command);
  head = tail = NULL;
//  int count = 0;
//  count= read(0, temp_command, 512);
//  temp_command[count-1] = '\0';
  
  

  
  
  while(strcmp(temp_command,"exit") != 0)
  {

    struct termios SavedTermAttributes;
    char RXChar;
    static char read_com[512];
    memset(read_com, '\0', strlen(read_com));
    int t = 0, flag = 0;
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes); 
    while(1)
    {
//        printf(" inner \n");
      read(STDIN_FILENO, &RXChar, 1);
      if(RXChar == 0x0A ) // C-d or Enter
      {
        break;
      }
        
      else
      {
        if(isprint(RXChar) && flag == 0)
        {
          read_com[t] = RXChar;
          write(1, &RXChar,1);
//          printf("%s \n", read_com);
          t++;
        }
        else
        {
          if (RXChar == 0x7F)
          {
            write(1, "\b \b", 3);
            read_com[t-1] = '\0';
            t--;
//            printf("%s \n", read_com);
          }
          
          if (RXChar == 0x1B && flag == 0)
            flag++;
          if (RXChar == 0x5B && flag == 1)
            flag++;
          if (RXChar == 0x41 && flag == 2)
            write(1,"up\n", 3);
          if (RXChar == 0x42 && flag == 2)
            write(1,"down\n", 5);            
        }
      }        
    
    }

//    write(1, read_com, strlen(read_com));
//    write(1, "\n", 1);
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);

    strcpy(temp_command, read_com);
    p=(struct command *)malloc(sizeof(struct command));   //construct dl-list
    strcpy(p->com, temp_command);
    p->number_count[0] = '?';
    p->number_count[1] = '\0';
    p->number_show[0] = '?';
    p->number_show[1] = '\0';
    p->nextCommand = NULL;
    p->preCommand = NULL;
    if(head == NULL)
    {
      head = p;
    }
    else
    {
      tail->nextCommand = p;
      p->preCommand = tail;
    }
    tail = p;
//    current_dir_at_first(current_path); 
//    write(1, temp_command, strlen(temp_command));
//    write(1, "\n", 1);
    
    char* token = strtok(temp_command, " ");           // token to sperate the string
    char* com_argument[20] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
                              , NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
                              , NULL, NULL, NULL, NULL};
    int num_com_arg = 0;
    while( token != NULL )           
    {
      com_argument[num_com_arg] = token;
//      current_dir_at_first(current_path); 
//      printf( "%s\n", com_argument[num_com_arg] );
      token = strtok( NULL, " ");
      num_com_arg++;
    }    
    
    if (com_argument[0] != NULL  && strcmp( com_argument[0], cd) == 0)     // cdcdcdcd
    {
         
      if (com_argument[1] != NULL)
      {
        chdir(com_argument[1]);
        static char now_path[512];
        getcwd(now_path, sizeof(now_path));
        memset(current_path, '\0', strlen(current_path));
        strcpy(current_path, now_path);
        current_dir_at_first(now_path);
          
      }
         
      else
      {
        char* home_dir = getenv("HOME");
        chdir(home_dir);
        static char home_path[512];
        getcwd(home_path, sizeof(home_path));
        memset(current_path, '\0', strlen(current_path));
        strcpy(current_path, home_path);
        current_dir_at_first(home_path);
      }      
    }  
    else if (com_argument[0] != NULL && strcmp( com_argument[0], ls) == 0)    // lslslslslslsls
    {
      if (com_argument[1] != NULL)
      {
        int n, i = 0;
        char ls_path[512];
        strcpy(ls_path, com_argument[1]);
        chdir(ls_path);
        
        struct dirent **namelist;      
        n = scandir(ls_path, &namelist, 0 , alphasort); 
        while(i < n)
        {
          struct stat type_perm;
          stat(namelist[i]->d_name, &type_perm);    
          char buff[] = {"----------"};
          if (type_perm.st_mode & S_IRUSR) 
             buff[1] = 'r'; 
          if (type_perm.st_mode & S_IWUSR) 
             buff[2] = 'w'; 
          if (type_perm.st_mode & S_IXUSR) 
             buff[3] = 'x'; 
          if (type_perm.st_mode & S_IRGRP) 
             buff[4] = 'r'; 
          if (type_perm.st_mode & S_IWGRP) 
             buff[5] = 'w'; 
          if (type_perm.st_mode & S_IXGRP) 
             buff[6] = 'x'; 
          if (type_perm.st_mode & S_IROTH) 
             buff[7] = 'r'; 
          if (type_perm.st_mode & S_IWOTH) 
             buff[8] = 'w'; 
          if (type_perm.st_mode & S_IXOTH) 
             buff[9] = 'x';             	
         	if (S_ISDIR(type_perm.st_mode))
     	      buff[0] = 'd';     	
      	  else   
      	    buff[0] = '-';   
        
          int k;
          for (k = 0; k < 10; k++)
            printf("%c", buff[k]);
          printf(" %s\n", namelist[i]->d_name);
          free(namelist[i]);       
          i++;
        }
        free(namelist);
        chdir(current_path);
        current_dir_at_first(current_path);      
      }
      
      else
      {
        int n, i = 0;
        struct dirent **namelist;      
        n = scandir(current_path, &namelist, 0 , alphasort); 
        while(i < n)
        {
          struct stat type_perm;
          stat(namelist[i]->d_name, &type_perm);    
          char buff[] = {"----------"};
          if (type_perm.st_mode & S_IRUSR) 
             buff[1] = 'r'; 
          if (type_perm.st_mode & S_IWUSR) 
             buff[2] = 'w'; 
          if (type_perm.st_mode & S_IXUSR) 
             buff[3] = 'x'; 
          if (type_perm.st_mode & S_IRGRP) 
             buff[4] = 'r'; 
          if (type_perm.st_mode & S_IWGRP) 
             buff[5] = 'w'; 
          if (type_perm.st_mode & S_IXGRP) 
             buff[6] = 'x'; 
          if (type_perm.st_mode & S_IROTH) 
             buff[7] = 'r'; 
          if (type_perm.st_mode & S_IWOTH) 
             buff[8] = 'w'; 
          if (type_perm.st_mode & S_IXOTH) 
             buff[9] = 'x';             	
         	if (S_ISDIR(type_perm.st_mode))
       	    buff[0] = 'd';     	
        	else   
        	  buff[0] = '-';   
        
          int k;
          for (k = 0; k < 10; k++)
            printf("%c", buff[k]);
          printf(" %s\n", namelist[i]->d_name);
          free(namelist[i]);       
          i++;
        }
        free(namelist);
        current_dir_at_first(current_path);      
      }
      
    }
    
    else if (com_argument[0] != NULL && strcmp(pwd, com_argument[0]) == 0)       // pwdpwd pwd pwd
    {

      static char absolute_path[512];
      memset(absolute_path, '\0', strlen(absolute_path));
      getcwd(absolute_path, sizeof(absolute_path)); 
      write(1, absolute_path, sizeof(absolute_path));
      write(1, "\n", 1);
      current_dir_at_first(current_path);
    }

    else if (com_argument[0] != NULL && strcmp(history, com_argument[0]) == 0) //history history history
    {
      struct command* his_p = tail;
      struct command* his_find = tail;
      int j = 9;

      for(;his_p->preCommand != NULL; his_p = his_p->preCommand)
      {
        his_p->number_count[0] = j + '0';
        j--;
      }
      
      his_p->number_count[0] = j + '0';
           
      for (;his_find->preCommand != NULL && 
            his_find->number_count[0] <= '9' && 
            his_find->number_count[0] > '0'; 
            his_find = his_find->preCommand);
   
      int r = 0;
      for (;his_find->nextCommand != NULL; his_find = his_find->nextCommand)
      {
        his_find->number_show[0] = r + '0';
        write(1, his_find->number_show, 2);
        write(1, "  ", 2);
        write(1, his_find->com, strlen(his_find->com));
        write(1, "\n", 1);
        r++;
      }
      his_find->number_show[0] = r + '0';
      write(1, his_find->number_show, 2);
      write(1, "  ", 2);
      write(1, his_find->com, strlen(his_find->com));
      write(1, "\n", 1);
      current_dir_at_first(current_path);
    }
    
    
       
    else if (com_argument[0] != NULL && strcmp(exit, com_argument[0]) == 0)
    {
    }
    
    else
    {
      current_dir_at_first(current_path);
//      char * argv[ ] ={ "ls","-al","/etc/passwd",0};
//     execvp("ls",argv);    
    }
//    current_dir_at_first(current_path); 
//    int count = read(0, temp_command, sizeof(temp_command));
//    temp_command[count-1] = '\0';
    memset(temp_command, '\0', strlen(temp_command));
  }
    

}		




