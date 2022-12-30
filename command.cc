
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include "command.h"
#include<unistd.h> 
#include<time.h>
SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}
pid_t pid;
void Command::execute()
{
	//change directory
	if(!strcmp( _simpleCommands[0]->_arguments[0], "cd" )) {
	    int x;
	    if(_simpleCommands[0]->_numberOfArguments == 1)
	      x = chdir(getenv("HOME"));
	    else
	      x = chdir(_simpleCommands[0]->_arguments[1]);
	    if(x < 0)
	      perror("cd");
	    clear();
	    prompt();
	    return;
	}
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}
		// Print contents of Command data structure
		print();
	
	// Add execution here
	// and call exec
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr=dup(2);
	
	// Setup i/o redirection
	int fdin;
	//if there is an input redirection (ex command < infile) then open the file .
	if (_inputFile)
	{
		//create a file descriptor to read from it
		fdin = open(_inputFile , O_RDONLY);
	}
	//the input file is the default
	else{
		fdin = dup(defaultin);
	}
	
	int fderr;
	
		if (_errFile)
	{
		fderr = open(_errFile , O_WRONLY , O_TRUNC);
		 _currentCommand.flag = 1;
		dup2(fderr,2);
		close(fderr);
	}
	
	//the err file is the default
	else{
		fderr = dup(defaulterr);
	}

	
	int outfd;
	
	// For every simple command fork a new process
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
//in the piped system we need to read from the filedescriptor which was output in the previous command.
		dup2(fdin, 0);
		close(fdin);
	//if the command is the last command or the input is already only one command
		if (i == _numberOfSimpleCommands - 1)
		{
		//redirect the output to the created file descriptor
			if (_outFile){
				if(flag==0){
					outfd = open(_outFile, O_WRONLY|O_APPEND);
			}
				else
					outfd = open(_outFile, O_WRONLY|O_TRUNC);
			
			}
			else{
			outfd = dup(defaultout);
			}
		}
		else
		{
		//there are more than one command so we need to create a pipe.
			int fdpipe[3];
			pipe(fdpipe);
			outfd = fdpipe[1];
			fdin = fdpipe[0];
			fderr = fdpipe[2];
			if ( outfd < 0 ) {
				perror( "fail creat outfile" );
				exit( 2 );
			}
		}
 //the output of the fist command execution will be the input of the second command and the output of the second command will be redirected to the default output.
		dup2(outfd, 1);
//		dup2(fderr,1);
		close(outfd);
		
		pid = fork();
		
		if (pid == 0)
		{
		execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
		return;
		}

	}
	//restore input/output/error
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	
	// Wait for last process in the pipe line
	if (!_background)
	{
		waitpid(pid,NULL,0);
	}
	clear();
	// Print new prompt
	prompt();	
}



void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

void sigchild(int sig){
		time_t tm;
		time(&tm);
		FILE *log;
		log= fopen("log_file.log","a");
		fprintf(log," Date : %s \t",ctime(&tm));
		fprintf(log,"%d \n", pid);
		fclose(log);
}
int 
main()
{

	signal(SIGINT,SIG_IGN);
	signal(SIGCHLD,sigchild);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

