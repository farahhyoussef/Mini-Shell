
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
#include <time.h>

#include "command.h"

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
	_outFileAppend = 0;
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

	if ( _outFileAppend != _errFile) {
		free( _outFileAppend );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_outFileAppend = 0;
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
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:_outFileAppend?_outFileAppend:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}
void handler(int signum){
	FILE* log=fopen("LogFile.log","a");
	time_t now;
	time(&now);
	fprintf(log,"Child Process Terminated. Time:%s\n",ctime(&now));
	fclose(log);
}
void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	if(!strcasecmp(_simpleCommands[0]->_arguments[0],"exit"))
	{
		printf("     Good Bye!!       \n");
		clear();
		exit(EXIT_SUCCESS);
	}
	if(!strcmp(_simpleCommands[0]->_arguments[0],"cd"))
	{
		if(chdir(_simpleCommands[0]->_arguments[1]) != 0)
			perror("cd");
		clear();
		prompt();
		return;
	}
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);
	int inputfile;
	inputfile=_inputFile?open(_inputFile,O_RDONLY | O_CREAT,0666):dup(defaultin);
	if ( inputfile < 0 ) {
		perror( "inputfile" );
		exit( 2 );
	}
	int outputfile;
	int errFile;
	
	int j;
    int** fdpipes = (int**)malloc(_numberOfSimpleCommands * sizeof(int*));
    for (j = 0; j < _numberOfSimpleCommands; j++)
    {
        fdpipes[j] = (int*)malloc(2 * sizeof(int));
    }
    for (j = 0; j < _numberOfSimpleCommands; j++)
    {
        if ( pipe(fdpipes[j]) == -1)
        {
            perror( "pipe");
            exit( 2 );
        }
    }
	pid_t pid;
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		dup2(inputfile, 0);
		close(inputfile);
        if(i == _numberOfSimpleCommands-1){
			if (_outFile)
			{
				outputfile=open(_outFile,O_WRONLY | O_TRUNC | O_CREAT,0666);
			}else if(_outFileAppend){
				outputfile=open(_outFileAppend,O_WRONLY | O_CREAT | O_APPEND,0666);
			}else{
				outputfile=dup(defaultout);
			}
			if ( outputfile < 0 ) {
				perror( "outfile" );
				exit( 2 );
			}
			errFile=_errFile?open(_outFileAppend,O_WRONLY | O_CREAT | O_APPEND,0666):dup(defaulterr);
			if ( errFile < 0 ) {
				perror( "errfile" );
				exit( 2 );
			}
		}
		else{
			outputfile=dup(fdpipes[i][1]);
			inputfile=dup(fdpipes[i][0]);
		}
		

		dup2(outputfile,1);
		dup2(errFile,2);
		close(outputfile);
		close(errFile);
		pid = fork();
		if(pid == 0)
		{
			
			for (j = 0; j < _numberOfSimpleCommands; j++)
            {
			    close(fdpipes[j][0]);
				close(fdpipes[j][1]);
            }
			close(inputfile);
			close(outputfile);
			close(errFile);
			close(defaultin);
			close(defaultout);
			close(defaulterr);
			
			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
			perror("error");
			exit(2);
		}
		else if (pid < 0 ) {
			perror("failure");
			exit(2);
		}
		signal(SIGCHLD,handler);
	}
	dup2(defaultin,0);
	dup2(defaultout,1);
	dup2(defaulterr,2);
	
	for (int j = 0; j < _numberOfSimpleCommands; j++)
	{
		close(fdpipes[j][0]);
		close(fdpipes[j][1]);
	}
	close(inputfile);
	close(outputfile);
	close(errFile);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	
	if(!_background){
		waitpid(pid,0,0);
	}
	for (j = 0; j < _numberOfSimpleCommands; j++)
    {
        free(fdpipes[j]);
    }

    free(fdpipes);
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	char s[100];
	printf("myshell:%s>",getcwd(s,100));
	fflush(stdout);
	signal(SIGINT,SIG_IGN);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

