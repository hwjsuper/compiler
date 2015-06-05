/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the C- compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
   { START,INNUM,INID,INEQ,INUNEQ,INSMEQ,INLGEQ,INOVER,STARTCOMMENT,ENDCOMMENT,DONE }
   StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{ if (!(linepos < bufsize))
  { lineno++;
    if (fgets(lineBuf,BUFLEN-1,source))
    { if (EchoSource) fprintf(listing,"%4d: %s",lineno,lineBuf);
      bufsize = strlen(lineBuf);
      linepos = 0;
      return lineBuf[linepos++];
    }
    else
    { EOF_flag = TRUE;
      return EOF;
    }
  }
  else return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{ if (!EOF_flag) linepos-- ;}

/* lookup table of reserved words */
static struct
    { char* str;
      TokenType tok;
    } reservedWords[MAXRESERVED]
   = {{"if",IF},{"else",ELSE},{"int",INT},{"return",RETURN},{"void",VOID},{"while",WHILE}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{ int i;
  for (i=0;i<MAXRESERVED;i++)
    if (!strcmp(s,reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the 
 * next token in source file
 */
TokenType getToken(void)
{  /* index for storing into tokenString */
   int tokenStringIndex = 0;
   /* holds current token to be returned */
   TokenType currentToken;
   /* current state - always begins at START */
   StateType state = START;
   /* flag to indicate save to tokenString */
   int save;
   while (state != DONE)
   { int c = getNextChar();
     save = TRUE;
     switch (state)
     { 
       case START:
         if (isdigit(c))
           state = INNUM;
         else if (isalpha(c))
           state = INID;
         else if (c == '=')
           state = INEQ;
         else if (c == '!')
           state = INUNEQ;
         else if (c == '<')
            state = INSMEQ;
         else if (c == '>')
            state = INLGEQ;
         else if ((c == ' ') || (c == '\t') || (c == '\n'))
           save = FALSE;
         else if (c == '/')
         { 
           state = INOVER;
         }
         else
         { state = DONE;
           switch (c)
           { case EOF:
               save = FALSE;
               currentToken = ENDFILE;
               break;
             case '+':
               currentToken = PLUS;
               break;
             case '-':
               currentToken = MINUS;
               break;
             case '*':
               currentToken = TIMES;
               break;
             case ';':
               currentToken = SEMI;
               break;
             case ',':
               currentToken = COMMA;
               break;
	         case '(':
               currentToken = LSMPAREN;
               break;
             case ')':
               currentToken = RSMPAREN;
               break;
             case '[':
               currentToken = LMDPAREN;
               break;
             case ']':
               currentToken = RMDPAREN;
               break;
             case '{':
               currentToken = LLGPAREN;
               break;
             case '}':
               currentToken = RLGPAREN;
               break;
             default:
               currentToken = ERROR;
               break;
           }
         }
         break;

       case INNUM:
         if (!isdigit(c))
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = NUM;
         }
         break;

       case INID:
         if (!isalpha(c))
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = ID;
         }
         break;

       case INEQ:
         state = DONE;
         if (c == '=')
           currentToken = EQ;
         else
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           currentToken = ASSIGN;
         }
         break; 

       case INUNEQ:
         state = DONE;
         if (c == '=')
           currentToken = UNEQ;
         else
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           currentToken = ERROR;
         }
         break;

       case INSMEQ:
         state = DONE;
         if (c == '=')
           currentToken = SMEQ;
         else
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           currentToken = SM;
         }
         break;

       case INLGEQ:
         state = DONE;
         if (c == '=')
           currentToken = LGEQ;
         else
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           currentToken = LG;
         }
         break;

       case INOVER:
          if (c =='*')
           {
           	 save = FALSE;
           	 tokenStringIndex--;
             state = STARTCOMMENT;
           } 
           else
           {
              state = DONE;
              ungetNextChar();
              save = FALSE;
              currentToken = OVER;
           }
           break;
           
       case STARTCOMMENT:
         save = FALSE;
         if (c == '*')
         { 
          state = ENDCOMMENT;
         }
         else{
           state = STARTCOMMENT;
         } 
         break;
         
       case ENDCOMMENT:
         save = FALSE;
         if (c == '/')
         { 
          state = START;
         }
         else{
           state = STARTCOMMENT;
         } 
         break;
         
       case DONE:
       default: /* should never happen */
         fprintf(listing,"Scanner Bug: state= %d\n",state);
         state = DONE;
         currentToken = ERROR;
         break;
     }
     if ((save) && (tokenStringIndex <= MAXTOKENLEN))
       tokenString[tokenStringIndex++] = (char) c;
     if (state == DONE)
     { tokenString[tokenStringIndex] = '\0';
       if (currentToken == ID)
         currentToken = reservedLookup(tokenString);
     }
   }
   if (TraceScan) {
     fprintf(listing,"\t%d: ",lineno);
     printToken(currentToken,tokenString);
   }
   return currentToken;
} /* end getToken */

