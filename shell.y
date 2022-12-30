
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

%token 	NOTOKEN GREAT NEWLINE GREATGREAT AMPERSAND PIPE LESS AMPERSANDGREATGREAT EXIT

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
#include <cstring>
#include <signal.h>
#include "command.h"
%}

%%
ggoal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command 
        ;

simple_command:	
	commands_list iomodifier_opt_list NEWLINE{
		printf("   Yacc: Execute commands \n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	| EXIT {	printf(" \n  	  Goodbye!!	\n");
			return 0;
		}
	;
iomodifier_opt_list :
	iomodifier_opt_list iomodifier_opt
	|
	;
commands_list :
	commands_list PIPE command_and_args
	|command_and_args
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

	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
               
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt: 
	GREATGREAT WORD{
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command:: _currentCommand.flag=1;
		Command::_currentCommand._outFile = $2;
	}
	|
	GREAT WORD LESS WORD{
		printf("   Yacc: insert output \"%s\"\n", $2);

		Command::_currentCommand._outFile = $2;	
		printf("   Yacc: insert input \"%s\"\n", $4);
		Command::_currentCommand._inputFile = $4;
	}
	|
	LESS WORD GREAT WORD{
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile = $4;	
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| 
	GREAT WORD{
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command:: _currentCommand.flag=0;
		Command::_currentCommand._outFile = $2;	
	}
	|
	LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	|
	AMPERSAND{
		printf("   Yacc: background \"\"\n");
		Command::_currentCommand._background = 1;
	}
	|
	AMPERSANDGREATGREAT WORD{
		printf("   Yacc: insert Output \"%s\"\n", $2);
//		Command::_currentCommand._outFile = $2;
//		if(Command:: _currentCommand.flag == 1 ){
		Command::_currentCommand._errFile = $2;
//		}
//		else if(Command:: _currentCommand.flag ==0){
//		Command::_currentCommand._outFile = $2;
//		}
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
