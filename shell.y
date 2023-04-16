
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE
%token  GREATAPPEND LESS PIPE AMPERSAND AMPERSANDGREATAPPEND

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	pipes iomodifiers_opt background_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE {Command::_currentCommand.execute();}
	| error NEWLINE { yyerrok; }
	;

pipes:
	command_and_args PIPE pipes
	| command_and_args
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifiers_opt:
	iomodifiers_opt iomodifier_opt
	|
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| GREATAPPEND WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFileAppend = $2;
	}
	| AMPERSANDGREATAPPEND WORD {
		printf("   Yacc: insert output and error \"%s\"\n", $2);
		Command::_currentCommand._outFileAppend = $2;
		Command::_currentCommand._errFile = $2;
	}
	;

background_opt:
	AMPERSAND {
		Command::_currentCommand._background = 1;
	}
	|
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
