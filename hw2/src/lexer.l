%option noyywrap 
%{
#include <stdio.h>
#include "header.h" 
int linenumber;
symtab * lookup();
symtab * ptr;

/* You need to define for all tokens in C--, here are some examples */
#define INT		10
#define FLOAT 		11 
#define OP_ASSIGN 	12 
#define OP_OR 		13 
#define MK_LPAREN 	14 
#define MK_RPAREN 	15 
#define ERROR 		100 

%}

letter   [A-Za-z]
digit    [0-9]
ID	 {letter}({letter}|{digit}|"_")*
WS	 [ \t]+
Int_constant {digit}+

Float_constant [+-]?(({digit}+\.?{digit}*)|({digit}*\.{digit}+))([eE][+-]?{digit}+)?

String_constant \"[^"]*\"
comment \/\*([^*]|[ \n\t\r]|((\*)+([^\/*]|[ \n\t\r])))*(\*)*\*\/


RWInt "int"
RWFloat "float"
/* You need to define the following RE's 
Float_constant
String_constant
comment
*/

OP_assign        "="
OP_or   "||"

/* Other operators appear here */ 


newline  "\n"

DL_lparen "("
DL_rparen ")"
DL_lbrace "{"
DL_rbrace "}"
DL_comma  ","
DL_semicol ";"
DL_dot  "."

DL_lsqbracket "["
DL_rsqbracket "]"

OP_plus "+"
OP_minus "-"
OP_mult "*"
OP_div "/"
OP_lt "<"
OP_gt ">"
OP_lte "<="
OP_gte ">="
OP_eq "=="
OP_neq "!="
OP_and "&&"
OP_not "!"

/* Other separators appear here */

error    .

%%

{comment}       {printf("%s\n", yytext);}
{String_constant} {}
{Int_constant} {}
{Float_constant} {}
{WS}            {}
{RWInt}         {/* return INT; */ }
{RWFloat}       {/* return FLOAT; */}
{ID}            { 
			ptr = lookup(yytext);
			if (ptr == NULL)
			     insertID(yytext);	
			else 
			     ptr->counter++;
		}

{OP_assign}     /* return OP_ASSIGN; */ 
{OP_or}         /* return OP_OR; */
{DL_lparen}     /* return MK_LPAREN; */
{DL_rparen}     /* return MK_RPAREN; */
{DL_lbrace}     /* return MK_LBRACE; */
{DL_rbrace}     /* return MK_RBRACE; */
{DL_comma}      {}
{DL_semicol}    {}

{DL_dot}    {}
{DL_lsqbracket} {}
{DL_rsqbracket} {}

{OP_plus} {}
{OP_minus} {}
{OP_mult} {}
{OP_div} {}
{OP_lt} {}
{OP_gt} {}
{OP_lte} {}
{OP_gte} {}
{OP_eq} {}
{OP_neq} {}
{OP_and} {}
{OP_not} {}

{newline}       linenumber += 1;
{error}         printf("ERR: %s\n", yytext);/* return ERROR; */


%%

main(int argc, char **argv)
{
 argc--; ++argv;
   if (argc > 0)
      yyin = fopen(argv[0], "r");
   else
      yyin = stdin;
   yylex();
   printf("\n");
   printSymTab();
}

