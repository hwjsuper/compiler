#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* 保存当前记号 */

/*进行递归调用的函数*/
static TreeNode * declaration_list(void);
static TreeNode * declaration(void);
static TreeNode * params(void);
static TreeNode * param_list(TreeNode * k);
static TreeNode * param(TreeNode * k);
static TreeNode * compound_stmt(void);
static TreeNode * local_declaration(void);
static TreeNode * statement_list(void);
static TreeNode * statement(void);
static TreeNode * selection_stmt(void);
static TreeNode * expression_stmt(void);
static TreeNode * iteration_stmt(void);
static TreeNode * return_stmt(void);
static TreeNode * expression(void);
static TreeNode * simple_expression(TreeNode * k);
static TreeNode * additive_expression(TreeNode * k);
static TreeNode * term(TreeNode * k);
static TreeNode * factor(TreeNode * k);
static TreeNode * var(void);
static TreeNode * call(TreeNode * k);
static TreeNode * args(void);

static void syntaxError(char * message)
{
  fprintf(listing,"\n>>> ");
  fprintf(listing,"Syntax error at line %d: %s",lineno,message);
  Error = TRUE;
  exit(1);
}

static void match(TokenType expected)
{ if (token == expected) token = getToken();
  else {
    syntaxError("unexpected token -> ");
    printToken(token,tokenString);
    fprintf(listing,"      ");
  }
}

/*declaration_list(void)函数使用递归向下分析方法直接调用declaration()函数，并返回树节点*/ 
TreeNode * declaration_list(void)
{ 
  TreeNode * t = declaration();
  TreeNode * p = t;
  //在开始语法分析出错的情况下找到int和void型，过滤掉int和void之前的所有Token，防止在开始时出错后面一错百错
  while((token != INT)&&(token != VOID)&&(token != ENDFILE))
  {   
	syntaxError("");
	getToken();
	if(token == ENDFILE)
	break;
  }
  //寻找语法分析的入口，即找到int和void
  while ((token == INT) || (token == VOID))
  { TreeNode * q;
    q = declaration();
    if (q != NULL) {
      if (t == NULL) t = p = q;
      else /* now p cannot be NULL either */
      { p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}

/*declaration(void)函数并不是直接调用var-declaration或fun-declaration文法所对应的函数，令其返回节点
而是在declaration(void)函数中，通过判断是"ID("、"ID["、还是"ID;"的方式先确定是变量定义还是函数定义，
然后分别根据判断的结果生成变量声明节点或函数声明节点*/ 
TreeNode * declaration(void)
{ TreeNode * t = NULL;
	TreeNode * p = NULL;
	TreeNode * q = NULL;
	TreeNode * s = NULL;
	if(token == INT)
	{ 
		p = newNode(IntK);
		match(INT);
	}
	else if(token == VOID)
	{  
		p = newNode(VoidK);
		match(VOID);
	}
	else
	{
		syntaxError("unexpected token -> ");
	}

	if((p != NULL)&&(token == ID))
	{ 
		q = newNode(IdK);     
		q->attr.name = copyString(tokenString);
		match(ID);
		if(token == LSMPAREN)//'('：函数情况
		{ 
			t = newNode(FunDeclK);
			t->child[0] = p;
			t->child[1] = q;
			match(LSMPAREN);
			t->child[2] = params();
			match(RSMPAREN);
			t->child[3] = compound_stmt();
		}
		else if(token == LMDPAREN)//'['：数组声明
		{ 
			t = newNode(VarArryDeclK);
			TreeNode * m = newNode(ArrayK);
			match(LMDPAREN);
			s = newNode(ConstK);
			s->attr.val = atoi(tokenString);
			m->child[0] = q;
			m->child[1] = s;
			t->child[0] = p;
			t->child[1] = m;
			match(NUM);
			match(RMDPAREN);
			match(SEMI);
		}
		else if(token == SEMI)//';'结尾：普通变量声明
		{
			t = newNode(VarDeclK);
			t->child[0] = p;
			t->child[1] = q;
			match(SEMI);
		}
		else
		{  
			syntaxError("");
		}
	} 
	else
	{  
		syntaxError("");
	}	
	return t;
}

TreeNode * params(void)
{  
	TreeNode * t = newNode(ParamsK);
	TreeNode * p = NULL;
	if(token == VOID)//开头为void，参数列表可能是(void)和(void id,[……])两种情况
	{  
		p = newNode(VoidK);
		match(VOID);
		if(token == RSMPAREN)//参数列表为(void)
		{
			if(t != NULL)
				t->child[0] = p;
		}
		else//参数列表为(void id,[……])  ->void类型的变量
		{
			t->child[0] = param_list(p);
		}
	}
	else if(token == INT)//参数列表为(int id,[……])
	{ 
		t->child[0] = param_list(p);
	}
	else
	{ 
		syntaxError("");
	}	
	return t;
}

TreeNode * param_list(TreeNode * k)//k可能是已经被取出来的VoidK，但又不是(void)类型的参数列表，所以一直传到param中去，作为其一个子节点
{  
	TreeNode * t = param(k);
	TreeNode * p = t; 
	k = NULL;//没有要传给param的VoidK，所以将k设为NULL
	while(token == COMMA)
	{ 
		TreeNode * q = NULL;
		match(COMMA);
		q = param(k);
		if(q != NULL)
		{
			if(t == NULL) 
			{ 
				t = p = q;
			}
			else
			{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode * param(TreeNode * k)
{  
	TreeNode * t = newNode(ParamK);
	TreeNode * p = NULL;//ParamK的第一个子节点
	TreeNode * q = NULL;//ParamK的第二个子节点

	if(k == NULL && token == INT)
	{
		p = newNode(IntK);
		match(INT);
	}
	else if(k != NULL)
	{
		p = k;
	}
	if(p != NULL)
	{
		t->child[0] = p;		
		if(token == ID)
		{ 			
			q = newNode(IdK);
			q->attr.name = copyString(tokenString); 
			match(ID);
			t->child[1] = q;
			
		}
		else
		{ 
			syntaxError("unexpected token -> ");
		}

		if((token == LMDPAREN)&&(t->child[1] != NULL))
		{
			match(LMDPAREN);
			TreeNode * m = newNode(ArrayK);
			m->child[0] = q;
			t->child[1] = m;
			match(RMDPAREN);
		}
		else 
		{  
			return t; 
		}		
	}
	else
	{ 
		syntaxError("");
	}
	return t;
}

TreeNode * compound_stmt(void)
{  
	TreeNode * t = newNode(CompStmtK);
	match(LLGPAREN);
	t->child[0] = local_declaration();
	t->child[1] = statement_list(); 
	match(RLGPAREN);
	return t;
}

TreeNode * local_declaration(void)
{  
	TreeNode * t = NULL;
	TreeNode * q = NULL;
	TreeNode * p = NULL;
	while(token == INT || token == VOID)
	{ 
		p = newNode(VarDeclK);
		if(token == INT)
		{
			TreeNode * q1 = newNode(IntK);
			p->child[0] = q1;
			match(INT);
		}
		else if(token == VOID)
		{
			TreeNode * q1 = newNode(VoidK);
			p->child[0] = q1;
			match(VOID);
		}
		if((p != NULL)&&(token == ID))
		{ 
			TreeNode * q2 = newNode(IdK);     
			q2->attr.name = copyString(tokenString);
			p->child[1] = q2;
			match(ID);

			if(token == LMDPAREN)
			{ 
				TreeNode * q3 = newNode(VarDeclK);
				p->child[3] = q3;
				match(LMDPAREN);
				match(RMDPAREN);
				match(SEMI);
			}
			else if(token == SEMI)
			{
				match(SEMI);
			}
			else
			{
				match(SEMI);
			}
		} 
		else
		{ 
			syntaxError("");
		}
		if(p != NULL)
		{
			if(t == NULL)
				t = q = p;
			else
			{
				q->sibling = p;
				q = p;
			}
		}
	}
	return t;
}

TreeNode * statement_list(void)
{  
	TreeNode * t = statement();   
	TreeNode * p = t;
	while (token == IF || token == LLGPAREN || token == ID || token == WHILE || token == RETURN || token == SEMI || token == LSMPAREN || token == NUM)
	{  
		TreeNode * q;
		q = statement();
		if(q != NULL)
		{
			if(t == NULL)  
			{ 
				t = p = q;
			}
			else
			{  
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode * statement(void)
{  
	TreeNode * t = NULL;
	switch(token)
	{
	case IF:
		t = selection_stmt(); 
		break;
	case WHILE:
		t = iteration_stmt(); 
		break;
	case RETURN:
		t = return_stmt(); 
		break;
	case LLGPAREN:
		t = compound_stmt(); 
		break;
	case ID: case SEMI: case LSMPAREN: case NUM: 
		t = expression_stmt(); 
		break;
	default:
		syntaxError("");
		token = getToken();
		break;
	}
	return t;
}

TreeNode * selection_stmt(void)
{  
	TreeNode * t = newNode(SeleStmtK);
	match(IF);
	match(LSMPAREN);
	if(t != NULL) 
	{
		t->child[0] = expression();
	}
	match(RSMPAREN);
	t->child[1] = statement();
	if(token == ELSE)
	{ 
		match(ELSE);
		if(t != NULL) 
		{ 
			t->child[2] = statement();
		}
	}
	return t;
}

TreeNode * expression_stmt(void)
{  
	TreeNode * t = NULL;
	if(token == SEMI)  
	{ 
		match(SEMI);
		return t;
	}
	else 
	{
		t = expression();
		match(SEMI);
	}
	return t;
}

TreeNode * iteration_stmt(void)
{  
	TreeNode * t = newNode(IterStmtK);
	match(WHILE);
	match(LSMPAREN);
	if(t != NULL) 
	{ 
		t->child[0] = expression();
	}
	match(RSMPAREN);
	if(t != NULL) 
	{ 
		t->child[1] = statement();
	}
	return t;
}

TreeNode * return_stmt(void)
{  
	TreeNode * t = newNode(RetnStmtK);
	match(RETURN);
	if (token == SEMI)
	{  
		match(SEMI);
		return t;
	}
	else
	{ 
		if(t != NULL) 
		{ 
			t->child[0] = expression();
		} 
	}
	match(SEMI);
	return t;
}

TreeNode * expression(void)
{  
	TreeNode * t = var();
	if(t == NULL)//不是以ID开头，只能是simple_expression情况
	{ 
		t = simple_expression(t); 
	}
	else//以ID开头，可能是赋值语句，或simple_expression中的var和call类型的情况
	{    
		TreeNode * p = NULL;
		if(token == ASSIGN)//赋值语句
		{ 
			p = newNode(AssignK);
			p->attr.name = copyString(tokenString);
			match(ASSIGN);
			p->child[0] = t;
			p->child[1] = expression();
			return p;
		}
		else //simple_expression中的var和call类型的情况
		{ 
			t = simple_expression(t);
		}
	}	   
	return t;
}

TreeNode * simple_expression(TreeNode * k)
{   
	TreeNode * t = additive_expression(k);
	k = NULL;
	if(EQ==token || LG==token || LGEQ==token || SM==token || SMEQ==token || UNEQ==token)
	{  
		TreeNode * q = newNode(OpK);
		q->attr.op = token; 
		q->child[0] = t;
		t = q;
		match(token);
		t->child[1] = additive_expression(k);		 
		return t;
	}
	return t;
}

TreeNode * additive_expression(TreeNode * k)
{  
	TreeNode * t = term(k);
	k = NULL;
	while((token == PLUS)||(token == MINUS))
	{ 
		TreeNode * q = newNode(OpK);
		q->attr.op = token; 
		q->child[0] = t; 
		match(token);
		q->child[1] = term(k);
		t = q;		
	}
	return t;
}

TreeNode * term(TreeNode * k)
{  
	TreeNode * t = factor(k);
	k = NULL;
	while((token == TIMES)||(token == OVER))
	{ 
		TreeNode * q = newNode(OpK);
		q->attr.op = token; 
		q->child[0] = t;
		match(token);
		q->child[1] = factor(k);
		t = q; 
	}
	return t;
}

TreeNode * factor(TreeNode * k)
{  
	TreeNode * t = NULL;
	if(k != NULL)//k为上面传下来的已经解析出来的以ID开头的var,可能为call或var
	{
		if(token == LSMPAREN && k->nodekind != ArrayK) //call
		{ 
			t = call(k);
		}
		else 
		{ 
			t = k; 
		}
	}
	else//没有从上面传下来的var
	{ 
		switch(token)
		{
		case LSMPAREN:
			match(LSMPAREN);
			t = expression();
			match(RSMPAREN);
			break;
		case ID:
			k = var();
			if(LSMPAREN == token && k->nodekind != ArrayK) 
			{ 
				t = call(k);
			}
			else{
				t = k; 
			} 
			break;
		case NUM:
			t = newNode(ConstK);
			if((t != NULL)&&(token == NUM))
			{ 
				t->attr.val = atoi(tokenString); 
			}
			match(NUM);
			break;
		default:
			syntaxError("");
			token = getToken();
			break;
		} 
	}
	return t;
}

TreeNode * var(void)
{  
	TreeNode * t = NULL;
	TreeNode * p = NULL;
	TreeNode * q = NULL;
	if(token == ID)
	{
		p = newNode(IdK);
		p->attr.name = copyString(tokenString); 
		match(ID);
		if(token == LMDPAREN) 
		{ 
			match(LMDPAREN);
			q = expression();
			match(RMDPAREN);

			t = newNode(ArrayK);
			t->child[0] = p;
			t->child[1] = q;
		}
		else
		{
			t = p;
		}
	}
	return t;
}

TreeNode * call(TreeNode * k)
{  
	TreeNode * t = newNode(CallK);
	if(k != NULL)
		t->child[0] = k;
	match(LSMPAREN);
	if(token == RSMPAREN)  
	{  
		match(RSMPAREN);
		return t;
	}
	else if(k != NULL)
	{ 
		t->child[1] = args();
		match(RSMPAREN);
	}
	return t;
}

TreeNode * args(void)
{  
	TreeNode * t = newNode(ArgsK);
	TreeNode * s = NULL;
	TreeNode * p = NULL;
	s = expression();
	p = s;
	while(token == COMMA)
	{   
		TreeNode * q;
		match(COMMA);
		q = expression();
		if(q != NULL)
		{
			if(s == NULL)  
			{ 
				s = p = q;
			}
			else
			{  
				p->sibling = q;
				p = q;
			}
		}
	}
	if(s != NULL)
	{
		t->child[0] = s;
	}
	return t;
}

/* 说明C-语言编写的程序由一组声名序列组成。
使用递归向下分析方法直接调用declaration_list()函数，并返回树节点
 */
TreeNode * parse(void)
{ TreeNode * t;
  token = getToken();
  t = declaration_list();
  if (token!=ENDFILE)
    syntaxError("Code ends before file\n");
  return t;
}
