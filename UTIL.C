/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{ 
  switch (token)
  { case IF:
    case ELSE:
    case INT:
    case RETURN:
    case VOID:
    case WHILE:
      fprintf(listing,
         "reserved word: %s\n",tokenString);
      break;
    
    case PLUS: fprintf(listing,"+\n"); break;
    case MINUS: fprintf(listing,"-\n"); break;
    case TIMES: fprintf(listing,"*\n"); break;
    case OVER: fprintf(listing,"/\n"); break;
    case SM: fprintf(listing,"<\n"); break;
    case SMEQ: fprintf(listing,"<=\n"); break;
    case LG: fprintf(listing,">\n"); break;
    case LGEQ: fprintf(listing,">=\n"); break;
	case EQ: fprintf(listing,"==\n"); break;
    case UNEQ: fprintf(listing,"!=\n"); break;
    case ASSIGN: fprintf(listing,"=\n"); break;
    case SEMI: fprintf(listing,";\n"); break;
    case COMMA: fprintf(listing,",\n"); break;
    case LSMPAREN: fprintf(listing,"(\n"); break;
    case RSMPAREN: fprintf(listing,")\n"); break;
    case LMDPAREN: fprintf(listing,"[\n"); break;
    case RMDPAREN: fprintf(listing,"]\n"); break;
    case LLGPAREN: fprintf(listing,"{\n"); break;
    case RLGPAREN: fprintf(listing,"}\n"); break;
    case ENDFILE: fprintf(listing,"EOF\n"); break;
    case NUM:
      fprintf(listing,
          "NUM, val= %s\n",tokenString);
      break;
    case ID:
      fprintf(listing,
          "ID, name= %s\n",tokenString);
      break;
    case ERROR:
      fprintf(listing,
          "ERROR: %s\n",tokenString);
      break;
    default: /* should never happen */
      fprintf(listing,"Unknown token: %d\n",token);
  }
}

/* 创造语法树的新节点*/
TreeNode * newNode(NodeKind kind){
  TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = kind;
    t->lineno = lineno;
    if(kind==OpK || kind==IntK || kind==IdK){
    	t->type = Int;
    }
	if(kind==IdK)
		t->attr.name = "";
	if(kind==ConstK)
		t->attr.val = 0;
  }
  return t;
} 

/* 给String分配空间并拷贝当前的tokenString */
char * copyString(char * s)
{
  int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = (char*)malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* 通过打印空格进行缩进 */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* printTree打印语法树，使用缩进来表示子树 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    switch (tree->nodekind) {
      case IdK:
        fprintf(listing,"Id: %s\n",tree->attr.name);
        break;
      case ConstK:
        fprintf(listing,"Const: %d\n",tree->attr.val);
        break;
      case ArrayK:
        fprintf(listing,"Array\n");
        break;
      case IntK:
        fprintf(listing,"Int\n");
        break;
      case VoidK:
        fprintf(listing,"Void\n");
        break;
      case VarDeclK:
        fprintf(listing,"Var-Declaration\n");
        break;
      case VarArryDeclK:
        fprintf(listing,"Array-Declaration\n");
        break;
      case FunDeclK:
        fprintf(listing,"Fun-Declaration\n");
        break;
      case ParamsK:
        fprintf(listing,"Params\n");
        break;
      case ParamK:
        fprintf(listing,"Param\n");
        break;
      case CompStmtK:
        fprintf(listing,"CompStmt\n");
        break;
      case SeleStmtK:
        fprintf(listing,"if\n");
        break;
      case IterStmtK:
        fprintf(listing,"while\n");
        break;
      case RetnStmtK:
        fprintf(listing,"return\n");
        break;
      case AssignK:
        fprintf(listing,"Assign\n");
        break;
      case OpK:
        fprintf(listing,"Op: ");
        printToken(tree->attr.op,"\0");
        break;
      case CallK:
        fprintf(listing,"Call\n");
        break;
      case ArgsK:
        fprintf(listing,"Args\n");
        break;
      default:
        fprintf(listing,"Unknown ExpNode kind\n");
        break;
    }
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
