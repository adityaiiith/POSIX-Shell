#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h>
#include<string>
#include<cstring>
#include<fcntl.h>
#include<iostream>
#include<map>
#include<vector>
#include<fstream>

#define clear() printf("\033[H\033[J") 
#define max_length_of_command 1000 
#define max_commands 200 

using namespace std;

map<string,string> aliasmap;
vector<string> history_buffer;
string prompt = "$ ";
extern char** environ;

void shell_start();
void check_redir(char**);
int detect_PS1(char*);
void make_amgrc();
int take_input(char *);
int detect_cd(char *);
void execute_cd(char *);
int detect_root();
void display_prompt(char *);
int detect_PS1(char *);
int detect_alias(char *);
void save_alias(char *);
void find_alias(char *);
void execute_simple_command(char **);
void create_file_for_single(char *);
void create_file_for_double(char *);
void check_redir(char **);
int detect_history(char *);
void show_history();
void parse(char *, char **, const char * );
int detect_delimiter(char *, char );
void execute_piped_command(char **);
int processString(char *, char **, char **);

void shell_start()
{
	clear();
}

void make_amgrc()//making .bashrc file for my amgshell
{
    char  *home,*user,*path,*display,*dbus;
    char* temp_environ[100]; 
    char hostname[1024];
    home = getenv("HOME");
    user = getenv("USER");
    path = getenv("PATH");
    dbus = getenv("DBUS_SESSION_BUS_ADDRESS");
    display = getenv("DISPLAY");
    gethostname(hostname,sizeof(hostname));
    ofstream file;
    file.open("amgrc");
    file<<"HOME="<<home<<endl;
    file<<"USER="<<user<<endl;
    file<<"PATH="<<path<<endl;
    file<<"HOSTNAME="<<hostname;
    //amgrc file created with the required environment variables
    string temp(home);
    temp = "HOME=" + temp;
    temp_environ[0]= (char*)temp.c_str();
    string temp1(user);
    temp1 = "USER=" + temp1;
    temp_environ[1]=(char*)temp1.c_str();
    string temp2(path);
    temp2 = "PATH=" + temp2;
    temp_environ[2]=(char*)temp2.c_str();
    string temp3(hostname);
    temp3 = "HOSTNAME=" + temp3;
    temp_environ[3]=(char*)temp3.c_str();
    string temp4(dbus);
    temp4 = "DBUS_SESSION_BUS_ADDRESS=" + temp4;
    temp_environ[4]=(char*)temp4.c_str();
    string temp5(display);
    temp5 = "DISPLAY=" + temp5;
    temp_environ[5]=(char*)temp5.c_str();

    memcpy(environ,temp_environ,sizeof(temp_environ));
}

int take_input(char* s)//take input command
{ 
	  string temp;
	  char* str;
    cout<<endl<<prompt;
    getline(cin,temp);
    history_buffer.push_back(temp);
  	char temp_array[temp.length() + 1];
  	strcpy(temp_array, temp.c_str());
  	temp_array[temp.length()]='\0';
	  str = temp_array;
	  if(strlen(str) != 0) 
    {    
        strcpy(s,str); 
        return 0; 
    }    
    else 
    {    
        return 1; 
    } 
}      
int detect_cd(char* s)
{ 
    string str1(s),temp = str1.substr(0,2);
    if(temp.compare("cd") == 0)
        return 1;
    else 
      return 0;
} 

void execute_cd(char* s)
{
    string str1(s);
    int size = str1.length();
    string dir =str1.substr(3,size-3);
    if(dir.compare("~") == 0)
      chdir(getenv("HOME"));
    else
      chdir(dir.c_str());

}

int detect_root()
{
    if(!geteuid())
      return 1;
    else
      return 0;
}

void display_prompt(char* s)
{
    string str1(s);
    int size = str1.length();
    int pos = str1.find("=");
    prompt = str1.substr(pos + 2,size-pos-3) + " ";
}

int detect_PS1(char* s)
{
   string st;
   st=(string)s;
   string temp = st.substr(0,3);
   if(temp.compare("PS1") == 0)
   {
       return 1;
   } 
   else
    return 0;
}
int detect_alias(char* s)
{
   string st;
   st=(string)s;
   string temp = st.substr(0,5);
   if(temp.compare("alias") == 0)
   {
       return 1;
   } 
   else
    return 0;
}
void save_alias(char* s)
{
    string str1(s);
    string key,value,temp = str1.substr(0,5);
    int size = str1.length();
    if(temp.compare("alias") == 0)
    { 
        int pos = str1.find("=");
        value = str1.substr(pos + 2,size-pos-3);
        key = str1.substr(6,pos-6);
        aliasmap[key] = value;
    } 
    //return key;
//char* k = (char*)str.c_str();
}

void find_alias(char* s)
{
    string str1;
    str1 = (string)s;
    if(aliasmap.find(str1) != aliasmap.end())//finding any matching entry in map
    {
        strcpy(s, (char*)aliasmap[str1].c_str() );//convert string ---> const char* --->  char*
    }
}

void execute_simple_command(char** parsed) // execute simple command without pipes
{ 
	//cout<<"\nINSIDE SIMPLE FUNCTION";
    pid_t pid = fork();  
   
  	if (pid == -1) 
  	{ 
        cout<<"\nCould not Fork"; 
        return; 
    } 
    else if(!pid) 
    {      	
    	check_redir(parsed);
        if(execvp(parsed[0], parsed) < 0) 
        {  
            cout<<"\nCould not execute command.."; 
        }    
        exit(0); 
    } 
    else 
    {   
        wait(NULL);  
        return; 
    } 
} 
void create_file_for_single(char* str)//creating file for single redirection
{	
  	int fd;
  	fd = open (str, O_WRONLY | O_TRUNC | O_CREAT,0644);
  	dup2 (fd,1);
    close(fd);
}

void create_file_for_double(char* str)//creating file for double redirection
{	
  	int fd;
  	fd = open (str, O_WRONLY | O_APPEND | O_CREAT,0644);
  	dup2 (fd,1);
    close(fd);
}
void check_redir(char** parsed)
{
	string temp;
	int i,flag1=0,flag2=0;
	for(i = 0 ; parsed[i] ; i++)
     	{
     		temp = parsed[i];
     		if(temp.compare(">")==0)
     		{
     			flag1 = 1;
     			break;
     		}
     		if(temp.compare(">>")==0)
     		{
     			flag2 = 1;
     			break;
     		}
		}
		if(flag1)
		{
			parsed[i]=NULL;
			create_file_for_single(parsed[i+1]);
		}
		if(flag2)
		{
			parsed[i]=NULL;
			create_file_for_double(parsed[i+1]);
		}

}

int detect_history(char *s)
{
    string str(s);
    if(str.compare("history") == 0)
      return 1;
    else
      return 0;
}

void show_history()
{
    for(int i = 0 ; i < history_buffer.size() ; i++)
    {
        cout<<"\n"<<i+1<<" "<<history_buffer[i];
    }
}

void parse(char* s, char** parsed_command, const char* delimiter) // delimit the command using space or |
{ 
	
    int i=0,command_count = 0;
  	while(i < max_commands)
  	{
        parsed_command[i] = strsep(&s, delimiter);
        //printf("\n%s",parsed_command[i]); 
  		if(parsed_command[i] == NULL) 
            break; 
        if(strlen(parsed_command[i]) == 0) 
            i--; 
        
        i++;
    }
        
}
int detect_delimiter(char* s,char delimiter)//detect if there is any delimiter in the command
{
	int flag  = 0 ;
	for(int i = 0 ; s[i] ; i++)
	{
		if(s[i]==delimiter)
	    {
	    	flag = 1;
			break;
		}
 	}
 	if(flag)
 		return flag;//delim present
 	else
 		return flag;//delim not present
}

void execute_piped_command(char** pipedarg)// execute command with pipes
{
	//cout<<"\nINSIDE PIPE FUNCTION";
	int pipefd[2];
	pid_t pid;
	//int pid=0;
	int fdd = 0,flag;

	while(*pipedarg != NULL)
	{
		pipe(pipefd);
		pid = fork();

		if(pid < 0)
		perror("\nCould not Fork");
		
		else if(pid == 0)
		{
			char *parsed_command[1000];
			dup2(fdd, 0);
			if(*(pipedarg + 1) != NULL)
			dup2(pipefd[1],1);
			close(pipefd[0]);
			close(pipefd[1]);
			
			parse(pipedarg[0], parsed_command," ");
			
			check_redir(parsed_command);//check for any redirection operator between pipes
			//execute_misc_command
            
			if(execvp(parsed_command[0], parsed_command) == -1)
			cout << "\nCould not execute command..."<<endl;
			exit(0);
		}
		else
		{
			wait(NULL);
			close(pipefd[1]);
			fdd = pipefd[0];
			pipedarg++;
		}
	}
}

int processString(char* s, char** parsed_command, char** parsedpipe)//determine command execution based on the presence of pipe
{ 

  	   //cout<<"INSIDE PROCESSSTRING";
  	   int flag1,flag2,flag3,flag4,flag5;
  	   char* temp;
  	   //strcpy(temp,s);
  	   //temp = s;
  	   //strncpy(temp, s, sizeof(s));
  	   //write(0, s, sizeof(s)); 
       flag5 = detect_history(s);
       flag4 = detect_cd(s);
       flag3 = detect_PS1(s);
       flag2 = detect_alias(s);
  	   flag1 = detect_delimiter(s,'|');
       if(flag5 == 1)
       {
          show_history();
          return 5;
       }
       if(flag4 == 1)
       {
          execute_cd(s);
          return 4;
       }
       if(flag3 ==1 )
       {
          display_prompt(s);
          return 3; 
       }
       if(flag2 == 1) 
       {
          save_alias(s);
          return 2;
       }
       if(flag1 == 1)
       {
       	 	parse(s, parsedpipe,"|");
       	 	return 1;
       }
       else
       {
          parse(s, parsed_command," ");
          return 0;         
       } 
       
} 

int main() 
{ 

    make_amgrc();
    char input[max_length_of_command], *simplearg[max_commands]; 
    char* pipedarg[max_commands]; 
    int mainflag; 
    shell_start(); 
    //cout <<endl<<getenv("PATH");
  	while(1)
  	{ 
        if(take_input(input)) 
            continue;

        find_alias(input);
        mainflag = processString(input, simplearg, pipedarg); 
        
        if (mainflag == 0) 
        {
        	  //cout<<"\nSIMPLECOMMANDENCOUNTERED";
            execute_simple_command(simplearg); 
        }
  		
    		if (mainflag == 1) 
    		{
    			    //cout<<"\nPIPEDCOMMANDENCOUNTERED";
              execute_piped_command(pipedarg);
        }
    } 
    return 0; 
} 