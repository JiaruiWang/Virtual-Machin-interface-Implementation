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
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

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
  int count_com_num;
  struct command* preCommand;
  struct command* nextCommand; 
}Com;

int count_space_or_pipe(char temp[512], char sign)
{
      int len = strlen(temp);
      int count= 0, i;
      for (i = 0; i < len; i++)
      { 
        if (temp[i] == sign)
        {
          count++;
        }
      }
      return count;
}

void assign_null_to_string_array(char* temp[], int index)
{
      int g, len= index;
      for (g = 0; g < len; g++)
      {
        temp[g] = NULL;
      }
}

void sperate_command_by_space_pipe_to_string_array(char* temp_array[], char temp[512], char* sign)
{
    char* token = strtok(temp, sign);     // token to sperate the string
    int num_com_arg = 0;
    while( token != NULL )           
    {
      temp_array[num_com_arg] = token;
//      write(1, temp_array[num_com_arg],strlen(temp_array[num_com_arg]));
      token = strtok( NULL, sign);
      num_com_arg++;
    }    
}

void current_dir_at_first(char* current_path)
{
  if (strlen(current_path) > 16)
  {
    char dir_buffer[512];
    memset(dir_buffer, '\0', strlen(dir_buffer));
    strcpy(dir_buffer, current_path);
    char* token_dir = strtok(dir_buffer, "/");     // token to sperate the string
    char* dir_argument[20] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
                              , NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
                              , NULL, NULL, NULL, NULL};
    int num_dir_arg = 0;
    while( token_dir != NULL )           
    {
      dir_argument[num_dir_arg] = token_dir;
//      current_dir_at_first(current_path); 
//      printf( "%s\n", com_argument[num_com_arg] );
      token_dir = strtok( NULL, "/");
      num_dir_arg++;
    }
    write(1, "/.../", 5);
    write(1, dir_argument[num_dir_arg-1], strlen(dir_argument[num_dir_arg-1])); 
    write(1, ">", 1);
  }
  else
  {
    write(1, current_path, strlen(current_path) );
    write(1, ">", 1);
  }
}

int cd(char* com_argument[20], char* cd_string, char current_path[512])
{
    if (com_argument[0] != NULL && strcmp( com_argument[0], cd_string) == 0)  // cdcd
    {
         
      if (com_argument[1] != NULL)
      {
        chdir(com_argument[1]);
        static char now_path[512];
        getcwd(now_path, sizeof(now_path));
        memset(current_path, '\0', strlen(current_path));
        strcpy(current_path, now_path);
//        current_dir_at_first(current_path);   
      }
      else
      {
        char* home_dir = getenv("HOME");
        chdir(home_dir);
        static char home_path[512];
        getcwd(home_path, sizeof(home_path));
        memset(current_path, '\0', strlen(current_path));
        strcpy(current_path, home_path);
//        current_dir_at_first(current_path);
      } 
      return 0;
    }
    return 1;
}
    
int ls(char* com_argument[20], char* ls_string, char current_path[512])
{
    if (com_argument[0] != NULL && strcmp( com_argument[0], ls_string) == 0)    
    {                                                      // lslslslslslsls
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
          char buff[10] = {"----------"};
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
                   
          write(1, buff, 10);
          write(1, " ", 1);
          write(1, namelist[i]->d_name, strlen(namelist[i]->d_name));
          write(1, "\n", 1);
          free(namelist[i]);       
          i++;
        }
        free(namelist);
        chdir(current_path);
//        current_dir_at_first(current_path);      
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
        
          write(1, buff, 10);
          write(1, " ", 1);
          write(1, namelist[i]->d_name, strlen(namelist[i]->d_name));
          write(1, "\n", 1);
          free(namelist[i]);       
          i++;
        }
        free(namelist);
//        current_dir_at_first(current_path);      
      }
      return 0;
    }
    return 1;
}

int pwd(char* com_argument[20], char* pwd_string, char current_path[512])
{
    if (com_argument[0] != NULL && strcmp(pwd_string, com_argument[0]) == 0)   
    {                                                        // pwdpwd pwd pwd

      static char absolute_path[512];
      memset(absolute_path, '\0', strlen(absolute_path));
      getcwd(absolute_path, sizeof(absolute_path)); 
      write(1, absolute_path, sizeof(absolute_path));
      write(1, "\n", 1);
//      current_dir_at_first(current_path);
      return 0;
    }
    return 1;
}

int history(char* com_argument[20], char* history_string, char current_path[512], struct command* tail)
{   
    if (com_argument[0] != NULL && strcmp(history_string, com_argument[0]) == 0) 
    {                                                 //history history history
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
//      current_dir_at_first(current_path);
      return 0; //run
    }
    return 1; //do not run
}

int exit_fun(char* com_argument[20], char* exit_string)
{     
    if (com_argument[0] != NULL && strcmp(exit_string, com_argument[0]) == 0)
    {
      exit(0);
      return 0; 
    }
    return 1;
}

int my_function(char* com_argument[20], char* cd_string, char* ls_string, char* pwd_string, char* history_string, char* exit_string, char current_path[512], struct command* tail)
{   
    int c, l, p, h, e;
    c = cd(com_argument, cd_string, current_path);
    l = ls(com_argument, ls_string, current_path);   
    p = pwd(com_argument, pwd_string, current_path);
    h = history(com_argument, history_string, current_path, tail);
    e = exit_fun(com_argument, exit_string);
    return c*l*p*h*e; // 1 all do not runn; 0 run
    
}

int main()
{
  char* cd_string = "cd";
  char* ls_string = "ls";
  char* pwd_string = "pwd";
  char* exit_string = "exit";
  char* history_string = "history";
  static char current_path[512];
  int length = 0;
  int count_com_number = 0;
  
  
  
  getcwd(current_path, sizeof(current_path));
  
  int one(const struct dirent *unused)
  {
    return 1;
  }
    
  current_dir_at_first(current_path); 
  struct command *head, *tail, *p, *history_scroll;
  static char temp_command[512];
//  size = sizeof(struct command);
  head = tail = NULL;
//  int count = 0;
//  count= read(0, temp_command, 512);
//  temp_command[count-1] = '\0';
  
   
  
  while(1)
  {

    struct termios SavedTermAttributes;
    char RXChar;
    static char read_com[512];

    memset(read_com, '\0', strlen(read_com));
    int t = 0, flag = 0;
    int hit_head = 0;
    int hit_tail = 0;
    int bottom = 0;
    int first_up_after_down = 0;
    int first_down_after_up = 0;
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes); 
    while(1)
    {
//        printf(" inner \n");
      read(STDIN_FILENO, &RXChar, 1);
      if(RXChar == 0x0A ) // C-d or Enter
        break;
        
      else
      {
        if(isprint(RXChar) && flag == 0)
        {
        
          read_com[t] = RXChar;
          write(1, &RXChar,1);
//          printf("%x \n", read_com[t]);
          t++;
        }
        else
        {
          if (RXChar == 0x7F && t > 0)
          {
            write(1, "\b \b", 3);
            read_com[t-1] = '\0';
            t--;
//            printf("%s \n", read_com);
          }
          else
            write(1, "\a", 1);
          
          if (RXChar == 0x1B && flag == 0)
            flag++;
          if (RXChar == 0x5B && flag == 1)
            flag++;
          if (RXChar == 0x41 && flag == 2)
          {
            if (history_scroll )
            { 

              if ( history_scroll->count_com_num >= 0)
              { 
                first_down_after_up = 1;
                if (!(hit_tail == 1) && first_up_after_down ==1 && (history_scroll->preCommand) && (history_scroll->preCommand))
                {
                  history_scroll = history_scroll->preCommand;
                  history_scroll = history_scroll->preCommand;
//                  first_up_after_down = 0;
                }
                first_up_after_down = 0;
                if (hit_tail == 1 && bottom == 1)
                {
                
                  hit_tail = 0;
                }
                 
//                
                if (hit_head == 0)
                {  

                  for (; t > 0; t--)
                    write(1, "\b \b", 3); 
                  if (history_scroll->nextCommand && bottom == 0)
                  {
    
                    for ( ; t > 0; t--)
                      write(1, "\b \b", 3);
                  }                    
                  memset(read_com, '\0', strlen(read_com));
                  strcpy(read_com, history_scroll->com);       
                  write(1,read_com, strlen(read_com));
                  t = strlen(read_com);
//                  printf("%d\n", t);

                  if ( history_scroll->preCommand)
                    history_scroll = history_scroll->preCommand;
                  else
                    hit_head = 1;
                }
                if (bottom == 1)
                  bottom = 0;
                if (history_scroll->preCommand == NULL)
                {
                  history_scroll = head;
                  write(1, "\a", 1);
//                  hit_head = 1; 
                }
              } 
              
            }
            else
              write(1, "\a", 1);
            flag = 0;
          }
          if (RXChar == 0x42 && flag == 2)
          {  
//            write(1 ,"1", 1);
            if (history_scroll )
            { 
              if (history_scroll->count_com_num >= 0)
              { 
                
                first_up_after_down = 1;
                if (!(hit_head == 1) && first_down_after_up == 1 && (history_scroll->nextCommand) && (history_scroll->nextCommand->nextCommand))
                  {
                    history_scroll = history_scroll->nextCommand;
                    history_scroll = history_scroll->nextCommand;
 //                   first_down_after_up = 0;
                  }
                first_down_after_up = 0;
                if (hit_head == 1 && (history_scroll->nextCommand))
                {
                  history_scroll = history_scroll->nextCommand;
                  hit_head = 0;
                }
              
                if (hit_tail == 1 && bottom == 0)
                {
    
                  for(; t > 0; t--)
                  {
                    write(1, "\b \b", 3);
                    memset(read_com, '\0', strlen(read_com));
                  }
                    
                  bottom = 1; 
                } 
//                write(1 ,"2", 1);
//                printf("%d\n", t);
                for(; t > 0; t--)
                {
                  write(1, "\b \b", 3); 
//                  printf("%d\n", t);
                }
                
//                printf("%d\n", t);
                if (hit_tail == 0)
                {
                 
                  if (history_scroll->preCommand)
                  {
                    for(; t > 0 ; t--)
                      write(1, "\b \b", 3); 
                  }                 
                  memset(read_com, '\0', strlen(read_com));
                  strcpy(read_com, history_scroll->com);
                  write(1,read_com, strlen(read_com));
                  t = strlen(read_com);
//                  printf("%d\n", t);

                  if (history_scroll->nextCommand)
                    history_scroll = history_scroll->nextCommand;
                  else
                    hit_tail = 1; 
                }
//                write(1 ,"3", 1);
                if (history_scroll->nextCommand == NULL)
                {
                  history_scroll = tail;
                }
                
              }
              
            } 
            flag = 0; 
          } 
                      
        }
      }        
    
    }

//    write(1, read_com, strlen(read_com));
//    write(1, "\n", 1);
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);

    write(1, "\n", 1);
    strcpy(temp_command, read_com);
    if (strlen(temp_command))
    {
    p=(struct command *)malloc(sizeof(struct command));   //construct dl-list
    strcpy(p->com, temp_command);
//    printf("%x :%s.\n" , p->com[0], p->com);
    p->number_count[0] = '?';
    p->number_count[1] = '\0';
    p->number_show[0] = '?';
    p->number_show[1] = '\0';
    p->count_com_num = count_com_number;
    count_com_number++;
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
    }
    int l = 0;
    struct command* count_list;   
    for (count_list = tail; count_list && count_list->preCommand; 
         count_list = count_list-> preCommand)
    {  
      l++;
//      printf("%d\n", l);
    }
    if (l==10)
    {
      head = count_list->nextCommand;
      head->preCommand = NULL;
      free(count_list);
    }
    history_scroll = tail;
    
//    current_dir_at_first(current_path); 
//    write(1, temp_command, strlen(temp_command));
//    write(1, "\n", 1);
    
    char temp_pipe[512];
    memset(temp_pipe, '\0', strlen(temp_pipe));
    strcpy(temp_pipe, read_com);
    int count_pipe =count_space_or_pipe(temp_pipe, '|');
    
    if (count_pipe > 0)
    { int p_itr, q_itr;
      int run1 = 1;
      char* com_argument_between_pipe[count_pipe + 1];      
      assign_null_to_string_array(com_argument_between_pipe, count_pipe + 1 );
      sperate_command_by_space_pipe_to_string_array(com_argument_between_pipe, temp_pipe, "|");
      
      int count_space_in_one_command[count_pipe + 1];
      for (q_itr = 0; q_itr < count_pipe + 1; q_itr++)
        count_space_in_one_command[q_itr] = count_space_or_pipe(com_argument_between_pipe[q_itr], ' ');
      
      char* com_argument_smallest[count_pipe + 1][10]; 
      
      for (q_itr = 0; q_itr < count_pipe + 1; q_itr++)
        for (p_itr = 0; p_itr < 10; p_itr++)
          assign_null_to_string_array(com_argument_smallest[q_itr], 10);
      
      for (q_itr = 0; q_itr < count_pipe + 1; q_itr++)
//        char temp_com_argument_between_pipe[512]=
        sperate_command_by_space_pipe_to_string_array(com_argument_smallest[q_itr], com_argument_between_pipe[q_itr], " ");      
      
      int pipe_fd[count_pipe+1][2], status;
      pid_t pid[count_pipe+1];
      int t = 0;
//      char buf[100000];


	  while (t <= count_pipe)
	  {
	    pipe(pipe_fd[t]);
	    if ((pid[t] = fork()) == 0)
		{
//			printf("%d %d %d\n", t, pipe_fd[t][0], pipe_fd[t][1]); // why the fd number keep growing
			if (t!=0)
			{
				close(0);
				dup2(pipe_fd[t-1][0], 0);
				close(pipe_fd[t-1][0]);
			}
			if (t!=count_pipe)
			{
				close(1);
				dup2(pipe_fd[t][1], 1);
				close(pipe_fd[t][1]);
				
			}
			
			run1 = my_function(com_argument_smallest[t], cd_string, ls_string, pwd_string, history_string, exit_string, current_path, tail);
			if (!run1)
			  exit(0);
      if (run1)
			  execlp(com_argument_smallest[t][0], com_argument_smallest[t][0], com_argument_smallest[t][1], (char *)NULL ); 

		}
		else
		{
			wait(&status);
			close(pipe_fd[t][1]);
		}
		t++;
	  }
    
    current_dir_at_first(current_path);
        
    }
      
    
  
    else
    {
      int status, count_space = 0, count_gt = 0, count_lt = 0;
      count_gt = count_space_or_pipe(temp_command, '>');
      count_lt = count_space_or_pipe(temp_command, '<');
      
      if (count_gt + count_lt)
      {
        char temp_redirect[512];
        int count_space0 = 0, count_space1 = 0;
        memset(temp_redirect, '\0', strlen(temp_redirect));
        strcpy(temp_redirect,temp_command);
        char *two_side_of_redirect[3];
        assign_null_to_string_array(two_side_of_redirect, 3);
        
        if (count_gt)
          sperate_command_by_space_pipe_to_string_array(two_side_of_redirect, temp_redirect, ">");
        if (count_lt)
          sperate_command_by_space_pipe_to_string_array(two_side_of_redirect, temp_redirect, "<");
                
        count_space0 = count_space_or_pipe(two_side_of_redirect[0], ' ');
        char* com_argument0[count_space0 + 2];
        assign_null_to_string_array(com_argument0, count_space0 + 2);
        sperate_command_by_space_pipe_to_string_array(com_argument0, two_side_of_redirect[0], " ");
        
        count_space1 = count_space_or_pipe(two_side_of_redirect[1], ' ');
        char* com_argument1[count_space1 + 2];
        assign_null_to_string_array(com_argument1, count_space1 + 2);
        sperate_command_by_space_pipe_to_string_array(com_argument1, two_side_of_redirect[1], " ");
        
        char file_path[512];
        memset(file_path, '\0', strlen(file_path));
        strcpy(file_path, current_path);
        strcat(file_path, "/");
        int f = 0, fd_out, fd_in;
        int pipe_re_fd[2][2];
        int run2 = 1, run3;
        if (count_gt)
        {
          
          strcat(file_path, com_argument1[0]);
          
          while(f <= 1)
          {
            pipe(pipe_re_fd[f]);
            if (fork() == 0)
            {
              if (f != 0)
              { 
                close(1);
                close(0);
                dup2(pipe_re_fd[f-1][0], 0);
                close(pipe_re_fd[f-1][0]);
                close(pipe_re_fd[f-1][1]);
                fd_out = open(file_path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
                
                dup2(fd_out, 1);
                close(fd_out);
                exit(0);
                
              }
              if (f != 1)
              {
                close(1);
                dup2(pipe_re_fd[f][1], 1);
                close(pipe_re_fd[f][1]);
                close(pipe_re_fd[f][0]);
			          run2 = my_function(com_argument0, cd_string, ls_string, pwd_string, history_string, exit_string, current_path, tail);
			          if (!run2)
			            exit(0);
                if (run2)
			            execlp(com_argument0[0], com_argument0[0], com_argument0[1], (char *)NULL );                 
              }
            }
            else
            {
            wait(&status);
            close(pipe_re_fd[f][1]);
            
            }
            f++;
          }
        }
        
        if (count_lt)
        {
          strcat(file_path, com_argument1[0]);
          
          while(f <= 1)
          {
            pipe(pipe_re_fd[f]);
            if (fork() == 0)
            {
              if (f != 0)
              { 
                close(1);
                close(0);
                dup2(pipe_re_fd[f-1][1], 1);
                close(pipe_re_fd[f-1][1]);
                close(pipe_re_fd[f-1][0]);
                fd_in = open(file_path, O_RDONLY, S_IRUSR|S_IWUSR);
                
                dup2(fd_in, 0);
                close(fd_in);
                exit(0);
                
              }
              if (f != 1)
              {
                close(0);
                dup2(pipe_re_fd[f][0], 0);
                close(pipe_re_fd[f][0]);
                close(pipe_re_fd[f][1]);
			          run3 = my_function(com_argument0, cd_string, ls_string, pwd_string, history_string, exit_string, current_path, tail);
			          if (!run3)
			            exit(0);
                if (run3)
			            execlp(com_argument0[0], com_argument0[0], com_argument0[1], (char *)NULL );                 
              }
            }
            else
            {
            wait(&status);
            close(pipe_re_fd[f][1]);
            
            }
            f++;
          }
        }
        current_dir_at_first(current_path);
      }
      
/*	  while (t <= count_pipe)
	  {      
      	    pipe(pipe_fd[t]);
	    if ((pid[t] = fork()) == 0)
		{
//			printf("%d %d %d\n", t, pipe_fd[t][0], pipe_fd[t][1]); // why the fd number keep growing
			if (t!=0)
			{
				close(0);
				dup2(pipe_fd[t-1][0], 0);
				close(pipe_fd[t-1][0]);
			}
			if (t!=count_pipe)
			{
				close(1);
				dup2(pipe_fd[t][1], 1);
				close(pipe_fd[t][1]);
				
			}
			
			run1 = my_function(com_argument_smallest[t], cd_string, ls_string, pwd_string, history_string, exit_string, current_path, tail);
			if (!run1)
			  exit(0);
      if (run1)
			  execlp(com_argument_smallest[t][0], com_argument_smallest[t][0], com_argument_smallest[t][1], (char *)NULL ); 

		}
		else
		{
			wait(&status);
			close(pipe_fd[t][1]);
		}
		t++;
	  }
 */    
      
      else
      {
      count_space = count_space_or_pipe(temp_command, ' ');
      char* com_argument[count_space + 2];    
      assign_null_to_string_array(com_argument, count_space + 2);   
      sperate_command_by_space_pipe_to_string_array(com_argument, temp_command, " ");
      
      int run = my_function(com_argument, cd_string, ls_string, pwd_string, history_string, exit_string, current_path, tail);
      if(!run)
        current_dir_at_first(current_path);
      if (run)
      {
        if(fork() == 0)
        {
          execlp(com_argument[0], com_argument[0], com_argument[1], NULL );
        }
        else
        {
          wait(&status);
        }  
        current_dir_at_first(current_path);
      }
      }
      
      memset(temp_command, '\0', strlen(temp_command));
    }
  }
    

}		


      
      



