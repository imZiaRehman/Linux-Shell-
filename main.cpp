#include <iostream>
#include <sys/types.h>
#include <cstring>
#include<string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;
#define clear() printf("\033[H\033[J")


void initialize()
{
    clear();
    cout << "         *Linux Shell*       \n";
    cout << "  1) Please enter a command to execute \n";
    cout << "  2) Enter exit to close Shell \n";
}
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    cout<<"\nDir:" <<cwd<<": ";
}
  
bool StringCompare(char* a, char b[])
{
    bool equal = false;
    for (int i = 0; a[i] != '\0'; i++)
    {
        if (a[i] == b[i])
        {
            if (a[i + 1] == '\0' || b[i + 1] == '\0')
            {
                return true;
            }
        }
        else
        {
            equal = false;
            break;
        }
    }
    return false;
}
int InputType(char Buffer[])
{
  //Cheking fot exit Condition.
    char e[] = { 'e','x','i','t'};
    if (StringCompare(Buffer, e))
     {
        return 0;         
     }
     //Checking for Pipes + Output redirection
   for(int i=0;Buffer[i] != '\0'; i++)
   {
      if(Buffer[i] == '|')
      {
         for(int j=i;Buffer[j] != '\0'; j++)
        {
           if(Buffer[j] == '>')
           {
             return 5;
           }
        }  
        break;
      }
   }
   //Checking Both input and output redirection.
   for(int i=0;Buffer[i] != '\0'; i++)
   {
      if(Buffer[i] == '<')
      {
         for(int j=i;Buffer[j] != '\0'; j++)
        {
           if(Buffer[j] == '>')
           {
             return 6;
           }
        }  
        break;
      }
   }

     //Cheking Pipes
   for(int i=0;Buffer[i] != '\0'; i++)
   {
      if(Buffer[i] == '>')
      {
        return 1;
      }
   }
    for(int i=0;Buffer[i] != '\0'; i++)
   {
      if(Buffer[i] == '<' )
      {
        return 3;
      }
   }
     for(int i=0;Buffer[i] != '\0'; i++)
   {
      if(Buffer[i] == '|' )
      {
        return 4;
      }
   }
   //Elese it is simple command
   return 2;
}
void ExecutingCommand(char *command, char **Arguments)
{
        //cout<<command;
        int id = fork();
        if (id == 0)
        {
            int status_code = execvp(command, Arguments);

            if(status_code == -1)
            {
                cout<<"Please Enter a Valid command.\n";
                exit(0);
            }
        }
        else
        {
          wait(NULL);
        }

}

void Sapearation(char Buffer[], char *command, char **&Arguments)
{

	int j = 0;        
        for (int i = 0; Buffer[i] != ' ' && Buffer[i] != '\0'; i++)
        {
            command[j] = Buffer[i];
            j++;
        }

        command[j] = '\0';
        j = 0;
        int index = 0;
        for (int i = 0; Buffer[i] != '\0'; i++)
        {
            if (Buffer[i] == ' ')
            {
                index++;
            }
        }
        index = index + 2;
        Arguments = new char* [index];
        char* dynamicArray;
        dynamicArray = strtok(Buffer, " ");
        int flag1 = 0, update = 0;
        while (dynamicArray != 0)
        {
            char* tempO;
            tempO = new char[1 + (strlen(dynamicArray))];
            strcpy(tempO, dynamicArray);
            tempO[strlen(dynamicArray)] = '\0';

            Arguments[update] = tempO;
            update++;

            dynamicArray = strtok(NULL, " ");
        }
        Arguments[update] = NULL;

}


void Separation2(char Buffer[], char *command, char **&Arguments)
{
 char Buffer2[100];
 char Filename[100];
 bool Append = false;
 int j=0;
 for(int i=0;Buffer[i] != '>' ;i++)
 {
   Buffer2[i] = Buffer[i];
   j++;
 }
 
 Buffer2[j] = '\0';
 if( Buffer[j] == '>')
 {
   if(Buffer[j+1] == '>')
   {
     //cout<<"Have to Append file \n"; 
     Append = true;
     j++;
   }
 } 
 int index = 0;
 j= j+2;
 for(int i = j; Buffer[i] != '\0';i++)
 {
  Filename[index] = Buffer[i];
  index++;
 }
 Filename[index] = '\0';
 Sapearation(Buffer2,command,Arguments);
   
 //Now changing File Descriptor of strdout and calling open system call
  int fd;
 if(Append ==  true)
 {
  fd=open(Filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
 }
  else
  {
    fd=open(Filename, O_CREAT | O_WRONLY , 0666);
  }
  int backup_fd=dup(1);
  dup2(fd,1);
  close(fd);
  ExecutingCommand(command, Arguments);
  close(1);
  dup2(backup_fd, 1);
  close(backup_fd);
}

void Separation3(char Buffer[], char *command, char **&Arguments)
{
 char Buffer2[100];
 char Filename[100];
 int j=0;
 for(int i=0;Buffer[i] != '<' ;i++)
 {
   Buffer2[i] = Buffer[i];
   j++;
 }
 
 Buffer2[j] = '\0';
 
 int index = 0;
 j= j+2;
 for(int i = j; Buffer[i] != '\0';i++)
 {
  Filename[index] = Buffer[i];
  index++;
 }
 Filename[index] = '\0';
  Sapearation(Buffer2,command,Arguments);
  
  int fd=open(Filename, O_RDONLY);
  int backup_fd=dup(0);
  dup2(fd,0);
  close(fd);
  ExecutingCommand(command, Arguments);
  close(0);
  dup2(backup_fd, 0);
  close(backup_fd);
}

int pipeCounter(char Buffer[])
{
 int counter =0;
  for(int i=0;Buffer[i] != '\0' ;i++)
  {
    if(Buffer[i] == '|')
    	counter++;
  }
 return counter;
}

void Pipe_Run(char *command, char **&Arguments, int in, int out)
{
	int pId = fork();
	
	if (pId < 0)	// If child process is not created.
	{
		cout << "ERROR: Fork Failed." << endl;
	}
	else if (pId == 0)
	{
		if (in != 0)
		{
			dup2(in, 0);
			close (in);
		}
		
		if (out != 1)
		{
			dup2(out, 1);
			close (out);
		}
	    int status_code = execvp(command, Arguments);

            if(status_code == -1)
            {
                cout<<"Please Enter a Valid command.\n";
                exit(0);
            }
		
	}
	else if (pId > 0)
	{
		wait(NULL);
	}
}

void Separation4(char Buffer[], char *command, char **&Arguments)
{
 char Buffer2[100];
 
 int Total_Pipes = pipeCounter(Buffer);
 //Now Total_Pipe contains the pipe present in the input

  int index = 0;
  int in;

  //The first process should get its input from the original file descriptor 0.  
  in = 0;
  int z = 0;
  // Below loop will run for first n-1 command.
  	int read = 0 ,fd[2];
  	for (int i=0; i<Total_Pipes; i++)
	{
	
	 for(z = 0; Buffer[index] != '|' && Buffer[index] != '\0'  ; index++, z++)
   	 {
   	   Buffer2[z] = Buffer[index];
   	 }
  	 Buffer2[z] = '\0';
  	// cout<<Buffer2<<endl;
  	 index = index +2;
	
	  if (pipe(fd) == -1)
	 {
	    cout << "ERROR: Pipe is not created." << endl;
	 }
          Sapearation(Buffer2,command,Arguments);	 
	  Pipe_Run (command, Arguments, read, fd[1]);
	   close(fd[1]);				
	   read = fd[0];
	}
	// This is the last command being run now.
	 for(z = 0; Buffer[index] != '|' && Buffer[index] != '\0'  ; index++, z++)
   	 {
   	   Buffer2[z] = Buffer[index];
   	 }
  	 Buffer2[z] = '\0';
  	 index = index +2;
	 Sapearation(Buffer2,command,Arguments);
	 Pipe_Run (command, Arguments, read, 1);
}
// Below function will execute the command witch contain pipe + Output redirection.
void PipeAndOutput(char Buffer[], char *command, char **&Arguments)
{
 char Buffer2[100], Filename[100];
 int index = 0, fd;
 bool Append = false;
 for(int i = 0 ; Buffer[i] != '>';i++)
 { 
 	Buffer2[i] = Buffer[i];
 	index++;	
 }
 if(Buffer[index+1] == '>')
 {
   Append = true;
 }
 index--;
 Buffer2[index] = '\0';
 index = index+3;
 int j=0;
 for(int i = index; Buffer[i] != '\0'; i++)
 {
  	Filename[j] = Buffer[i];
  	j++;
 } 
  Filename[j] = '\0';
  if(Append ==  true)
 {
  fd=open(Filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
 }
  else
  {
    fd=open(Filename, O_CREAT | O_WRONLY , 0666);
  }
  int backup_fd=dup(1);
  dup2(fd,1);
  close(fd);
  Separation4(Buffer2,command,Arguments);
  close(1);
  dup2(backup_fd, 1);
  close(backup_fd);
}
//Below function is for both input and output redirection.
void InputAndOutput(char Buffer[], char *command, char **&Arguments)
{
 char Buffer2[100], Filename[100];
 int index = 0, fd;
 bool Append = false;
 for(int i = 0 ; Buffer[i] != '>';i++)
 { 
 	Buffer2[i] = Buffer[i];
 	index++;	
 }
 if(Buffer[index+1] == '>')
 {
   Append = true;
   //cout<<"Append true\n";
   
 }
 index--;
 Buffer2[index] = '\0';

if(Append == true)
	index++;
 index = index+3;
 int j=0;
 for(int i = index; Buffer[i] != '\0'; i++)
 {
  	Filename[j] = Buffer[i];
  	j++;
 } 
  Filename[j] = '\0';
  //cout<<Buffer2<<endl;
  //cout<<Filename;
  if(Append ==  true)
 {
  fd=open(Filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
 }
  else
  {
    fd=open(Filename, O_CREAT | O_WRONLY , 0666);
  }
  int backup_fd=dup(1);
  dup2(fd,1);
  close(fd);
  Separation3(Buffer2,command,Arguments);
  close(1);
  dup2(backup_fd, 1);
  close(backup_fd);
  
}
int main()
{

    /* Shell initialization */
    initialize();
    //Taking Input
    
    while (1)
    {
        char Buffer[100], command[100], ** Arguments;
        printDir();
        cin.getline(Buffer, 100);
        int typeFlag = InputType(Buffer);

       //If Input value is exit
       if(typeFlag == 0)
       {
           cout << "Closing Shell \n";
           return 0;
       }
       
       if(typeFlag == 1)  //Input contatin Output Redirection
       {
          //cout << "Input contatin Output Redirection\n";
          Separation2(Buffer,command,Arguments);
       }
       if(typeFlag == 3) //Input contatin Input Redirection
       {
           //cout<<"Input contatin Input Redirection\n";
           Separation3(Buffer,command,Arguments);
       }
        if(typeFlag == 4) //Input contatin Pipes
       {
           //cout<<"Input contatin Pipes\n";
           Separation4(Buffer,command,Arguments);
       }
       if(typeFlag == 2) //Simple command
       {
         //cout<<"Simple command \n";
         Sapearation(Buffer,command,Arguments);
        
         ExecutingCommand(command, Arguments);
       }
        if(typeFlag == 5) //Input contatin Pipes + Output Redirection
       {
          //cout << "Input contatin Pipes + Output Redirection\n";
          PipeAndOutput(Buffer,command,Arguments);
       }
       
         if(typeFlag == 6) //Input contatin Input + Output Redirection
       {
          //cout << "Input contatin Input + Output Redirection\n";
          InputAndOutput(Buffer,command,Arguments);
       }
       
   
    }
    return 0;
}




