/****************************************************************************
							Phg 2.0 
							本文件源自于13年的一个个人小项目
							内涵群论与对称性，极简主义等理念
							运算式编程
语法示例:

#function
$blend(a, b, alpha)
{
	$a*(1-alpha) + b*alpha;
}

#call function
ab = blend(1, 8, 0.8);
>ab;

#if - else
t = 0;
?(i = 1){
t = t + 1;
}:{
t = t - 1;
}
>t;

#calc
yy = 8*3 + 1*8;
> yy;

#loop
@(yy < 8){
yy = yy + 1;
}
> yy;

****************************************************************************/
//namespace PHG{
#define PHG_DEBUG
//#define SYNTAXERR(msg)	ERRORMSG("At: " << cd.ptr - cd.start + 1 << ", ERR: " << msg)
#define SYNTAXERR(msg)	ERRORMSG("ERR: " << msg << "\nAt: \n" << cd.ptr)
#ifdef WIN
#define PHG_ASSERT(x)	{if(!(x)){std::stringstream ss; ss << "PHG ASSERT FAILED! " << __FILE__ << "(" << __LINE__ << ")"; ::MessageBoxA(0, ss.str().c_str(), "PHG ASSERT", 0); errorcode = 1;} }
#else
#define PHG_ASSERT(x)	{if(!(x)){std::stringstream ss; ss << "PHG ASSERT FAILED! " << __FILE__ << "(" << __LINE__ << ")"; PRINT(ss.str()); errorcode = 1;} }
#endif
#define PHG_PRINT(x)	{if(becho) PRINT(x)}
#define INVALIDFUN	cd.funcnamemap.end()

#define ADD_VAR			GROUP::gvarmapstack.addvar
#define ADD_SVAR(s,v)	GROUP::gvarmapstack.addvar(s.c_str(),v)
#define SET_VAR			ADD_VAR
#define SET_SVAR		ADD_SVAR
#define GET_VAR			GROUP::gvarmapstack.getvar

#define varname		std::string
#define to_int(v)	(int)(v)

#define opr			unsigned short
#define fnname	std::string
#define functionptr	const char*

#define NAME		0x01FF
#define NUMBER		0x02FF
#define OPR			0x03FF
#define LGOPR		0x04FF

// -----------------------------------------------------
//#ifndef code	
struct code;
//#endif
#ifndef callfunc
var callfunc(code& cd);
#endif
#ifndef parser_fun
typedef void (*parser_fun)(code& cd);
#endif
parser_fun parser = 0;
#ifndef statement_fun
typedef int(*statement_fun)(code& cd);
#endif
statement_fun statement = 0;

static char rank[256];				// 是运算符等级设定数组
unsigned char rank_opr0(opr c) { return rank[(char)c]; }
typedef unsigned char(*rank_opr_fun)(opr);
rank_opr_fun rank_opr = rank_opr0;

typedef bool(*funparam_fun)(var&, const char*, const char*);
funparam_fun get_funparam = 0;

typedef var(*get_var_fun)(const char*);
get_var_fun get_var = 0;

typedef void(*add_var_fun)(const char*, var&);
add_var_fun add_var = 0;

typedef void(*add_var_fun2)(code& cd, const char*, const char*, var&);
add_var_fun2 add_var2 = 0;

typedef void(*subtree_fun)(code& cd, const char*, var&);
subtree_fun subtree = 0;

typedef void(*tableindex_fun)(const char*, var&);
tableindex_fun tableindex = 0;

#ifndef gvarmapstack
struct varmapstack_t;
extern varmapstack_t	gvarmapstack;
#endif
bool	becho = true;

// API
#ifndef fun_t
typedef var(*api_fun)(code& cd, int stackpos);
#endif 
struct api_fun_t
{
	api_fun fun;
};
std::map<std::string, api_fun_t> api_list;

// 表格数据 [1,2,3]
void(*table)(code& cd);

std::vector<std::string> gstable;

// 计算过程
void(*process)(code& cd);

// 树节点
//void(*tree)(code& cd);
#ifndef tree_fun
void(*tree)(code& cd);// 树节点 {A,B,C}
#else
tree_fun tree = 0;
#endif 

// 运算
var(*act)(code& cd, int args);

// 错误处理
int errorcode = 0;

// -----------------------------------------------------
static inline bool checkline(char c) {
	return (c == '\n' || c == '\r');
}
static inline bool checkspace(char c) {
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}
static inline bool checkspace2(char c) {
	return (c == ' ' || c == '\t');
}
static bool iscalc0(opr o) {
	char c = (char)o;
#ifdef CHECK_CALC
	return CHECK_CALC;
#else
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '.';
#endif
}
typedef bool (*iscalc_fun)(opr o);
iscalc_fun iscalc = iscalc0;

static bool islogic0(opr o) {
	char c = (char)o;
#ifdef CHECK_LOGIC
	return CHECK_LOGIC;
#else
	return c == '>' || c == '<' || c == '=' || c == '&' || c == '|' || c == '!';
#endif
}
iscalc_fun islogic = islogic0;

static inline bool isname(char c) {
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_';
}
static inline bool isnum(char c) {
	return c >= '0' && c <= '9';
}
static inline bool isbracket(char c) {
	return c == '(';
}

// -----------------------------------------------------
// 堆栈定义
// -----------------------------------------------------
// stacks define
struct codestack_t
{
	std::vector<const char*> stack;
	string cach;
	void push(const char* c) {
		stack.push_back(c);
	}
	const char* pop() {
		cach = stack.back();
		stack.pop_back();
		return cach.c_str();
	}
	const char* cur() {
		PHG_ASSERT(!stack.empty());
		return stack[top()];
	}
	int top() {
		return stack.size() - 1;
	}
	bool empty() {
		return stack.empty();
	}
	codestack_t() {}
};
struct valstack_t
{
	std::vector<var> stack;
	void push(const var& v) {
		//PRINT("PUSH")
		stack.emplace_back(v);
	}
	var pop() {
		//PRINT("POP")
		if (stack.empty())
		{
			ERRORMSG("pop value error!");
			return INVALIDVAR;
		}
		var ret = stack.back();
		stack.pop_back();
		return ret;
	}
	bool pop(var& v) {
		//PRINT("POP2")
		if (stack.empty())
		{
			ERRORMSG("pop value error!");
			return false;
		}
		v = stack.back();
		stack.pop_back();
		return true;
	}
	var& back() {
		return stack.back();
	}
	void pop_back() {
		if (stack.empty())
		{
			ERRORMSG("pop value error!");
			return;
		}
		stack.pop_back();
	}
	var& cur() {
		//PRINT("cur")
		PHG_ASSERT(!stack.empty());
		return stack[top()];
	}
	var& get(int pos) {
		if (stack.empty() || top() - pos < 0)
		{
			ERRORMSG("get value error!");
		}
		return stack[top() - pos];
	}
	int top() {
		return stack.size() - 1;
	}
	void reset()
	{
		stack.clear();
	}
	valstack_t() {}
};

struct oprstack_t
{
	std::vector<opr> stack;
	void push(opr c) {
		stack.push_back(c);
	}
	opr pop() {
		opr ret = stack.back();
		stack.pop_back();
		return ret;
	}
	opr cur() {
		PHG_ASSERT(!stack.empty());
		return stack[top()];
	}
	void setcur(opr o) {
		PHG_ASSERT(!stack.empty());
		stack[top()] = o;
	}
	int top() {
		return stack.size() - 1;
	}
	bool empty() {
		return stack.empty();
	}
	oprstack_t() {}
};

struct varmapstack_t
{
	using varmap_t = std::map<varname, var>;
	std::vector<varmap_t> stack;

	void push()
	{
		//PRINT("varmapstack PUSH");
		stack.push_back(varmap_t());
	}
	void pop()
	{
		//PRINT("varmapstack POP");
		stack.pop_back();
	}
	void addvar(const char* name, const var& v)
	{
#ifdef USE_STRING
		//PRINT("addvar: " << name << "=" << float(v));
#endif
		//if(!gtable.empty())
		//	add2table(v);

		if (stack.empty())
			push();

		stack.back()[name] = v;
	}
	var getvar(const char* name)
	{
		//PRINT("getvar = " << name);
		if (stack.empty())
		{
			ERRORMSG("var: " << name << " undefined!");
			return INVALIDVAR;
		}
		for (int i = stack.size() - 1; i >= 0; i--)
		{
			auto it = stack[i].find(name);
			if (it != stack[i].end())
			{
				return it->second;
			}
		}
		ERRORMSG("var: " << name << " undefined!");
		return INVALIDVAR;
	}
	bool getvar(var& vout, const char* name)
	{
		if (stack.empty())
		{
			return false;
		}
		for (int i = stack.size() - 1; i >= 0; i--)
		{
			auto it = stack[i].find(name);
			if (it != stack[i].end())
			{
				vout = it->second;
				return true;
			}
		}
		return false;
	}
	void clear()
	{
		stack.clear();
	}
} gvarmapstack;

// -----------------------------------------------------
// 代码结构体
// -----------------------------------------------------
struct code
{
	const char* ptr;				// code pointer
	const char* start;
	codestack_t			codestack;	// 代码栈
	oprstack_t			oprstack;	// 操作栈
	valstack_t			valstack;	// 值栈
	std::vector<std::string>		strstack;		// 字符串栈(存变量名）
	std::map<fnname, functionptr>	funcnamemap;	// 函数map
	std::vector<int>	iter;		// iterator

	code() {}
	code(const char* buf) {
		start = buf;
		ptr = buf;
	}
	char next0() {
		return (*(++ptr));
	}
	char next() {
		while (!eoc(++ptr) && checkspace(*(ptr)));
		return (*ptr);
	}
	char next_() {
		while (!eoc(++ptr) && checkspace2(*(ptr)));
		return (*ptr);
	}
	char next2() {
		while (!eoc(++ptr)) {
			if (!checkspace(*(ptr)) && !isname(*(ptr))) // 跳过空格,名字
				break;
		}
		return (*ptr);
	}
	char next3() {
		while (!eoc(++ptr)) {
			if (!checkspace(*(ptr)) && !isname(*(ptr)) && !isnum(*(ptr)))
				break;
		}
		return (*ptr);
	}
	char next4() {
		while (!eoc(++ptr) && checkspace2(*(ptr)));
		return (*ptr);
	}
	char nextline() {
		while (!eoc(++ptr) && !checkline(*(ptr)));
		return (*ptr) == '\0' ? '\0' : *(++ptr);
	}
	char getnext0() {
		const char* p = ptr+1;
		if(!eoc(p))
			return (*p);
		return 0;
	}
	char getnext() {
		const char* p = ptr;
		while (!eoc(++p) && checkspace(*(p)));
		return (*p);
	}
	char getnext2() {
		const char* p = ptr;
		while (!eoc(++p)) {
			if (!checkspace(*(p)) && (!isname(*(p))))
				break;
		}
		return (*p);
	}
	char getnext3() {
		const char* p = ptr;
		while (!eoc(++p)) {
			if (!checkspace(*(p)) && !isnum(*(p)))
				break;
		}
		return (*p);
	}
	char getnext4(const char* start = 0) {
		const char* p = start != 0 ? start : ptr;
		while (!eoc(++p)) {
			if (!checkspace(*(p)) && (!isname(*(p)) && !isnum(*(p))))
				break;
		}
		return (*p);
	}
	const char* getnext5() {
		const char* p = ptr;
		while (!eoc(++p)) {
			if (!checkspace(*(p)) && (!isname(*(p)) && !isnum(*(p)) && (*p) != '.'))
				break;
		}
		return p;
	}
	bool eoc(const char* p = 0) {
		if(errorcode != 0) 
			return true;
		p == 0 ? p = ptr : 0;
		return (p == 0 || (*p) == '\0');
	}
	inline char pre() {
		return ptr == start ? 0 : *(ptr-1);
	}
	inline char cur() {
		return *ptr;
	}
	const char* getname() {
		static char buf[32];// 暂不支持多线程！
		char* pbuf = buf;
		const char* p = ptr;
		if (isname(*p))
		{
			while (!eoc(p) && !checkspace(*p) && (isname(*p) || isnum(*p)))
				*(pbuf++) = *(p++);
		}
		(*pbuf) = '\0';
		return buf;
	}
	short gettype()
	{
		for (; !eoc(); next()) {
			char c = cur();
			if (checkspace(c))
				continue;
			else if (isdigit(c)) {
				return NUMBER;
			}
			else if (iscalc(c)) {
				return OPR;
			}
			else if (islogic(c)) {
				return LGOPR;
			}
			else if (isname(c)) {
				return NAME;
			}
			else
				return c;
		}
		return 0;
	}
};
/*
// 运算
var act_default(code& cd, int args)
{
	opr o = cd.oprstack.pop();

	PHGPRINT("act:" << o << " args = " << args)

	switch (o) {
	case '+': {
		if (args > 1) {
			var b = cd.valstack.pop();
			var a = cd.valstack.pop();
			return a + b;
		}
		else {
			return cd.valstack.pop();
		}
	}
	case '-': {
		if (args > 1) {
			var b = cd.valstack.pop();
			var a = cd.valstack.pop();
			return a - b;
		}
		else {
			return -cd.valstack.pop();
		}
	}
	case '*': {
		var b = cd.valstack.pop();
		var a = cd.valstack.pop();
		return a * b;
	}
	case '/': {
		var b = cd.valstack.pop();
		var a = cd.valstack.pop();
		return a / b;
	}
	case '>': {
		var b = cd.valstack.pop();
		var a = cd.valstack.pop();
		return a > b;
	}
	case '<': {
		var b = cd.valstack.pop();
		var a = cd.valstack.pop();
		return a < b;
	}
	case '=': {
		var b = cd.valstack.pop();
		var a = cd.valstack.pop();
		return a == b;
	}
	case '!': {
		var a = cd.valstack.pop();
		return !a;
	}
	default: {return 0; }
	}
}
*/

inline var chars2var(code& cd) {
	char buff[64];
	bool isreal = false;
	int i = 0;
	for (; i < 64; i++) {
		char c = cd.cur();
		if (c == '.')
			isreal = true;
		if (!isdigit(c) && c != '.')
			break;
		buff[i] = c;
		cd.next();
	}
	buff[i] = '\0';
	//PRINTV(buff);
	cd.strstack.push_back(buff);
	//if (!isreal && !gtable.empty())
	//{
	//	int number = atoi(buff);
	//	if (number < 0 || number >= gtable.size())
	//	{
	//		ERRORMSG("chars2var error! number=" << number);
	//		return INVALIDVAR;
	//	}
	//	//PRINTV(number);
	//	return gtable[number];
	//}
	return isreal ? var((real)atof(buff)) : var(atoi(buff));
}

// Get Value (number/function/var)
bool getval(code& cd, short type) {

	if (type == NUMBER) {
		cd.valstack.push(chars2var(cd));
		/*if (cd.oprstack.empty() || !(iscalc(cd.oprstack.cur()) || islogic(cd.oprstack.cur()))) {
			cd.oprstack.push('.');
		}*/
		return true;
	}
	else if (type == NAME) {
		const char* name = cd.getname();

		// 函数
		if (api_list.find(name) != api_list.end() ||
			cd.funcnamemap.find(name) != INVALIDFUN) {

			cd.valstack.push(callfunc(cd));
			//PRINT("func:" << name << " c:" << cd.cur());
		}
		else
		{// 变量
			if (get_var)
				cd.valstack.push(get_var(name));
			else
			{
				var v;
				if (gvarmapstack.getvar(v, name))
					cd.valstack.push(std::move(v));
				else
				{
					//PRINT("var: " << name << " not found!");
#ifdef USE_STRING	
					cd.valstack.push(var(name)); // 变量找不到 按照字符串处理
#endif
				}
			}
			cd.next3();

			cd.strstack.push_back(name);

			//PRINT("var:" << name << " c:" << cd.cur());
		}
		//if (cd.oprstack.empty() || !(iscalc(cd.oprstack.cur()) || islogic(cd.oprstack.cur()))) {
		//	cd.oprstack.push('.');
		//}
		return true;
	}
	return false;
}
// finished trunk
void finishtrunk(code& cd, int trunkcnt = 0)
{
	const char sk = '{', ek = '}';

	int sk_cnt = 0;
	while (!cd.eoc()) {
		char c = cd.cur();
		if (c == sk) {
			trunkcnt++;
			sk_cnt++;
		}
		else if (c == ek) {
			trunkcnt--;

			if (trunkcnt == 0) {
				cd.next();
				break;
			}
		}
		else if (c == ';') // 单行 trunk
		{
			if (sk_cnt == 0)
			{
				cd.next();
				break;
			}
		}
		cd.next();
	}
}

// get string
inline std::string getstring(code& cd, char s1 = '\'', char s2 = '\"', char ed = '\"')
{
	//PRINT("getstring...")
	std::string content;
	while (!cd.eoc()) {
		char c = cd.cur();
		if (c == '\\')
		{
			cd.ptr++;
			content += cd.cur();
			cd.ptr++;
			continue;
		}
		if (c != s1 && c != s2 && c != ed)
		{
			content += c;
			cd.ptr++;
			continue;
		}
		cd.next0();
		break;
	}
	//PRINTV(content)
	return content;
}
// -----------------------------------------------------
// 表达式 for example: x=a+b, v = fun(x), x > 2 || x < 5
// -----------------------------------------------------
var expr(code& cd, int args0 = 0, unsigned char rank0 = 0)
{
	//PRINT("expr( ");
	int args = args0;
	int oprs = 0;
	bool brace = true; // 避免大括号歧义
	while (!cd.eoc()) {
		short type = cd.gettype();
		//PRINTV(cd.cur())
		if (type == '\"' || type == '\'')
		{
			cd.next0();
			string str = getstring(cd, type, type, type);
			cd.strstack.push_back(str);
#ifdef USE_STRING			
			cd.valstack.push(std::move(var(str.c_str())));
			args++;
			//	PRINTV(cd.cur());
#else
			return INVALIDVAR;
#endif
		}
		else if (type == '[')
		{
			string body;
			while (true) {
				char c = *(++cd.ptr);
				ASSERT(c != '[')// 不支持嵌套！
				if (c == 0 || c == ']')
				{
					cd.next();
					break;
				}
				body += c;
			}
			PRINT("tableindex: " << body);
			var ret;
			if (tableindex)
			{
				tableindex(body.c_str(), ret);
			}
			return ret;
		}
		else if (type == '{' && brace)
		{// 运算中使用大括号，临时匿名节点
			string stbody;
			while (true) {
				char c = *(++cd.ptr);
				ASSERT(c != '{');// 不支持嵌套！
				if (c == 0 || c == '}')
				{
					cd.next();
					break;
				}
				stbody += c;
			}
			PRINT("{" << stbody << "}");
			var ret;
			ASSERT(subtree != 0)
			subtree(cd, stbody.c_str(), ret);
			cd.valstack.push(ret);
			args++;
			//PRINT("cd.cur(): " << cd.cur());
			continue;
			//return ret;
		}
		else if (type == NAME || type == NUMBER) {
			getval(cd, type);
			args++;
			brace = false;
			//PRINTV(args);
			/*if (';' == cd.cur())
			{
				return cd.valstack.pop();
			}*/
		}
		else if (type == OPR || type == LGOPR) {
			opr o = cd.cur();
			if (rank_opr(o) == rank_opr('.') - 1) // 注意：指数位置运算符优先级永远紧跟在(.)之后！
			{// 指数位置一元运算符
				cd.oprstack.push(o);
				oprs++;
				cd.next();
				cd.valstack.push(act(cd, 1));
				args++;
				continue;
			}
			{
				opr lo = (o << 8) | cd.getnext0();
				if (iscalc(lo)) {
					o = lo;
					//PRINT("long oper: " << o);
				}
			}
			if (rank_opr(o) <= rank0)
			{
				return cd.valstack.pop();
			}
			/*if (!cd.oprstack.empty() && cd.oprstack.cur() == '.')
				cd.oprstack.setcur(o);
			else*/
			{
				cd.oprstack.push(o);
				oprs++;
			}
			cd.next();
			if (o & 0xFF00)
				cd.next();

			if (cd.cur() == '{')
			{
				brace = true;
				//cd.valstack.push(expr(cd));
				//args++;
				continue;
			}
			//PRINT(cd.cur());

			if (iscalc(cd.cur()) || islogic(cd.cur())) {
				cd.valstack.push(expr(cd));
				args++;
			}
			else {
				char no;
				if (cd.cur() == '(')
				{
					cd.next();
					cd.valstack.push(expr(cd));
					no = cd.getnext4();
					cd.next();
					args++;
				}
				else
				{
					if (isnum(cd.cur()))
						no = (*cd.getnext5());
					else if (isname(cd.cur()))
					{
						no = cd.getnext4();
						//PRINTV(no)
						if(no == '(') // 函数？
						{
							const char* p = cd.ptr;
							while (!cd.eoc(++p)) {
								if ((*p) == ')' || (*p) == ';')
									break;
							}
							no = cd.getnext4(p);
							//PRINTV(no)
						}
					}
				}
				//PRINTV(no);
				// 四则运算
				if (cd.cur() != '(' &&
					(iscalc(no) || islogic(no))) // 这里暂时不考虑long oper!
				{
					if (cd.cur() == ')')
						cd.next();

					type = cd.gettype();
					if (rank_opr(o) >= rank_opr(no)) { // A*B+...
						if(getval(cd, type))
							args++;

						cd.valstack.push(act(cd, args));
						args = 1;
					}
					else { // A+B*...
						getval(cd, type);
						args = 1;
						cd.valstack.push(expr(cd, 1, rank_opr(o)));
						args++;
						cd.valstack.push(act(cd, args));
						char nc = cd.cur();
						if (iscalc(nc) || islogic(nc)) {
							args = 1;
							continue;
						}
						else
							return cd.valstack.pop();
					}
				}
			}
		}
		else {
			char c = cd.cur();
			if (c == '(') {
				cd.next();
				cd.valstack.push(expr(cd));
				cd.next();

				args++;
			}
			else if (c == ')' || c == ']' || c == ';' || c == ',' || c == '{' || c == '}') {

				if (!cd.oprstack.empty() &&
					(iscalc(cd.oprstack.cur()) || islogic(cd.oprstack.cur())) &&
					oprs > 0)
				{
					return act(cd, args);//PRINT(")");
					//PRINTV(oprs);
					//return act(cd, args);
				}
				else {
					return cd.valstack.pop();//PRINT(")");
					//return cd.valstack.pop();
				}
			}
		}
	}
	SYNTAXERR("';' is missing?");
	errorcode = 1;
	return INVALIDVAR;
}
// single var
void singvar(code& cd) {
	std::string name = cd.getname();
	cd.next3();
	if (cd.cur() == '=')
	{
		//PRINT("singvar: " << name);
		cd.next();

		var v = expr(cd);
		cd.next();
		if (add_var)
			add_var(name.c_str(), v);
		else
			gvarmapstack.addvar(name.c_str(), v);
	}
	else if (cd.cur() == '.')
	{
		cd.next();
		std::string prop = cd.getname();
#ifdef USE_STRING
		var va;
		if (gvarmapstack.getvar(va, prop.c_str()))
			prop = va.tostr();
#endif
		//PRINTV(prop);
		cd.next3();
		PHG_ASSERT(cd.cur() == '=');
		cd.next();
		var v = expr(cd);
		cd.next();
		if (add_var2)
			add_var2(cd, name.c_str(), prop.c_str(), v);
	}
}

// statement
int statement_default(code& cd) {

	short type = cd.gettype();
	if (type == NAME) {
		const char* p_nc = cd.getnext5();
		if (*p_nc == '=') {
			singvar(cd);
		}
		else if (*p_nc == '(') {
			callfunc(cd);
			cd.next();
		}
		else
		{
			//PRINT(*(cd.ptr));
			opr lo = ((*p_nc) << 8) | *(p_nc + 1);
			if (iscalc(lo)) {
				expr(cd);
			}
			else
			{
				SYNTAXERR("statement error : '" << *p_nc << "'");
				errorcode = 1;
			}
		}
	}
	else if (cd.cur() == '>') {
		cd.next();
		PHGPRINT("> ", expr(cd));
		cd.next();
	}
	else
	{
		cd.next();
	}
	return 0;
}

// subtrunk
int subtrunk(code& cd, var& ret, int depth, bool bfunc, bool bsingleline = false)
{
	while (!cd.eoc()) {
		short type = cd.gettype();
		//PRINTV(cd.cur());
		switch (type) {
			case '~': // break
			{
				//PRINT("break");
				return 3; // 跳出
			}
			case ';':
			{
				//PRINT(";");
				cd.nextline();
				break;
			}
			case '}':
			{
				//PRINT("}")
				cd.next();
				if (bfunc && depth == 0)
				{
					PRINT("}");
					return 2; // 函数返回
				}
				break;
			}
			case '\'':
			case '#':
			{
				cd.nextline();
				break;
			}
			case '?':  // if else
			{
				PHG_ASSERT(cd.next() == '(');
			IF_STATEMENT:
				cd.next();
				bool e = bool(expr(cd));
				cd.next();
				if (e == false) {// else
					finishtrunk(cd, 0);

					if (cd.cur() == '}')
					{
						cd.next();
						break;
					}

					if (cd.cur() == ':')
					{
						cd.next();
						if (cd.cur() == '(')
						{
							goto IF_STATEMENT;
						}
					}
					else
						continue;
				}
				bool tk = false;
				if (cd.cur() == '{')
				{
					tk = true;
					cd.next();
				}

				int rettype = subtrunk(cd, ret, depth + 1, 0, !tk);
				if (rettype == 2) {
					return rettype;
				}
				if (rettype == 3)
				{
					//if (tk)
					finishtrunk(cd, 1);
					return rettype;
				}
				break;
			}
			case ':':
			{
				cd.next();
				finishtrunk(cd, 0);

				//cd.next();

				continue;
			}
			case '@': // loop
			{
				if (cd.next() == '(') {
					cd.next();

					const char* cp = cd.ptr;
					cd.iter.push_back(0);
				codepos1:
					{ // iter
						cd.iter.back()++;
						std::string name = "i";
						for (auto it : cd.iter)
							name = "_" + name;
						gvarmapstack.addvar(name.c_str(), var(cd.iter.back()));
					}
					var e = expr(cd);
					cd.next();

					//PRINT("iter ");
					if (e != 0) {
						bool tk = false;
						if (cd.cur() == '{')
						{
							tk = true;
							cd.next();
						}
						int rettype = subtrunk(cd, ret, depth + 1, 0, !tk);
						//PRINTV(rettype);

						if (rettype == 2) {
							return rettype;
						}
						else if (rettype == 3) {
							finishtrunk(cd, 1);
							//if (cd.cur() == '}') 
							//	cd.next();
							return rettype;
						}

						cd.ptr = cp;
						goto codepos1;
					}
					else {
						finishtrunk(cd, 0);
					}
					cd.iter.pop_back();
				}
				else
				{
					cd.iter.push_back(0);
					int loopcnt = int(expr(cd));
					PRINT("@" << loopcnt);
					//PRINTV(cd.cur())
					bool tk = false;
					if (cd.cur() == '{')
					{
						tk = true;
						cd.next();
					}
					const char* cp = cd.ptr;
					//while(expr(cd) != 0){cd._i ++;
					for (int i = 1; i <= loopcnt; i++) {
						{ // iter
							cd.iter.back() = i;
							std::string name = "i";
							for (auto it : cd.iter)
								name = "_" + name;
							//PRINT(name << "=" << i << " in " << loopcnt)
							gvarmapstack.addvar(name.c_str(), var(cd.iter.back()));
						}
						cd.ptr = cp;
						int rettype = subtrunk(cd, ret, depth + 1, 0, !tk);

						if (rettype == 2) {
							return rettype;
						}
						if (rettype == 3) {// break;
							finishtrunk(cd, 1);
							//if (cd.cur() == '}') 
							//	cd.next();
							//PRINTV(i << " in " << loopcnt);
							break;
						}
					}
					cd.iter.pop_back();
				}
				break;
			}
			case '$': // return
			{
				if (!bfunc)
					return 0;

				cd.next();
				//PRINTV(cd.cur());
				if (cd.cur() != ';')
					ret = expr(cd);

				return 2; // 函数返回
			}
			default:
			{
				if (cd.cur() == '{' ||	// tree define
					cd.cur() == '$')	// function define
					return 0;
				statement(cd);

				if (bsingleline)
					return 0;
			}
		}
	}
	return 0;
}

// 函数
var callfunc_phg(code& cd) {
	fnname fnm = cd.getname();
	PRINT("callfunc: " << fnm << "(){");
	//PRINT("{");
	PHG_ASSERT(cd.next3() == '(');

	cd.next();
	while (!cd.eoc()) {
		char c = cd.cur();
		if (c == ')') {
			cd.next();
			break;
		}
		else if (c == ',') {
			cd.next();
			continue;
		}
		else {
			cd.valstack.push(expr(cd));
		}
	}

	cd.codestack.push(cd.ptr);

	if (api_list.find(cd.getname()) != api_list.end() ||
		cd.funcnamemap.find(fnm) == INVALIDFUN)
	{
		ERRORMSG("no function named: '" << fnm << "'");
		return INVALIDVAR;
	}
	cd.ptr = cd.funcnamemap[fnm];

	cd.next3();
	PHG_ASSERT(cd.cur() == '(');

	gvarmapstack.push();
	cd.next();

	std::vector<std::string> paramnamelist;
	std::vector<var> paramvallist;
	while (!cd.eoc()) {
		char c = cd.cur();
		if (c == ')') {
			cd.next();
			break;
		}
		else if (c == ',') {
			cd.next();
		}
		else {
			paramnamelist.push_back(cd.getname());
			paramvallist.push_back(cd.valstack.back());
			cd.valstack.pop_back();
			cd.next2();
		}
	}
	for (int i = 0; i < paramnamelist.size(); i++)
		gvarmapstack.addvar(paramnamelist[i].c_str(), paramvallist[paramvallist.size() - 1 - i]);

	var ret(0);
	PHG_ASSERT(subtrunk(cd, ret, 0, true) == 2);
	gvarmapstack.pop();

	cd.ptr = cd.codestack.pop();
	//PRINT("}");
	return ret;
}

var callfunc(code& cd) {
	fnname fnm = cd.getname();
	if (api_list.find(fnm) != api_list.end())
	{
		PHG_PRINT("API:" << fnm);
		api_fun_t& apifun = api_list[fnm];
		int args = 0;

		PHG_ASSERT(cd.next3() == '(');

		cd.next();
		while (!cd.eoc()) {
			char c = cd.cur();

			if (c == ')') {
				cd.next();
				break;
			}
			else if (c == ',') {
				cd.next();
				continue;
			}
			else {
				if (c == '{' && get_funparam)
				{// 读到',' or ')'
					string param;
					while (true) {
						c = *(++cd.ptr);
						if (c == 0 || c == '}')
							break;
						param += c;
					}
					var v;
					if (get_funparam(v, fnm.c_str(), param.c_str()))
					{
						cd.valstack.push(v);
						args++;
					}
					cd.next();
				}
				else
				{
					cd.valstack.push(expr(cd));
					args++;
				}
			}
		}
		var ret = apifun.fun(cd, args);
		for (int i = 0; i < args; i++)
			cd.valstack.pop_back();
		//PRINTV(cd.cur());
		return ret;
	}
	else
		return callfunc_phg(cd);
}

// func
void func(code& cd) {
	fnname fnm = cd.getname();
	PHG_PRINT("$define func: " << fnm);
	if (cd.funcnamemap.find(fnm) != cd.funcnamemap.end())
	{
		ERRORMSG("function named: '" << fnm << " already exists!");
		return;
	}
	if (api_list.find(fnm) != api_list.end())
	{
		ERRORMSG("function named: '" << fnm << " already exists!");
		return;
	}

	cd.funcnamemap[fnm] = cd.ptr;
	cd.next();
	finishtrunk(cd, 0);
}

// ------------------------------------------
// 默认解析器
// ------------------------------------------
void parser_default(code& cd) {
	PHG_PRINT("=========PHG========");
	PHG_PRINT(cd.ptr);
	PHG_PRINT("--------------------");
#ifdef RANK_INIT
	RANK_INIT
#else
	rank['|'] = 1;
	rank['&'] = 2;
	rank['>'] = 3;
	rank['<'] = 3;
	rank['='] = 3;
	rank['+'] = 4;
	rank['-'] = 4;
	rank['*'] = 5;
	rank['/'] = 5;
	rank['!'] = 6;
	rank['^'] = 7;
	rank['.'] = 9;
#endif
	//getchar();

	if(gvarmapstack.stack.empty())
		gvarmapstack.push();

	while (!cd.eoc()) {
		short type = cd.gettype();

		if (type == ';') {
			cd.nextline();
		}
		else if (type == '\'' || type == '#') {
			cd.nextline();
		}
		else if (type == '$') {
			cd.next();
			func(cd);
		}
		else if (type == '(') {
			if(process)
				process(cd);
		}
		else if (type == '[') {
			table(cd);
		}
		else if (type == '{') {
			tree(cd);
		}
		else {
			var ret(0);
			subtrunk(cd, ret, 0, 0);
		}
	}
}

// ------------------------------------------
// 初始化 与 外部调用
// ------------------------------------------
void init()
{
	errorcode = 0;

	if (!parser)
		parser = parser_default;
	if (!statement)
		statement = statement_default;
}

bool checkChinese(const char* str)
{
	char c;
	while (true)
	{
		c = *str++;
		//PRINT(c);
		if (c == 0) break;
		if (c & 0x80)
			if (*str & 0x80)
			{
				//PRINTV(str);
				return true;
			}
	}
	return false;
}
void fixedstring(string& out, const char* str)
{
	out.clear();
	char c;
	while (true)
	{
		c = *str++;
		if (c == '#') do c = (*str++); while ((*str) != '\n' && c != '\0');
		if (c == 0) break;
		out += c;
	}
}
bool checkcode(const char* str)
{
	string codestr;
	fixedstring(codestr, str);
	if (count(codestr.begin(), codestr.end(), '{') != count(codestr.begin(), codestr.end(), '}'))
	{
		ERRORMSG("number of \'{\' != \'}\'!");
		return false;
	}
	return true;
}
// doexpr
inline var doexpr(const char* str)
{
	//PRINT("doexpr " << str)
	code cd(str);

	return expr(cd);
}

// dostring
void dostring(const char* str)
{
	//PRINT("dostring ...")
	init();

#ifdef PHG_DEBUG
	{// check format
		if (!checkcode(str))
			return;
	}
#endif
	code c(str);
	parser(c);
	//PRINT("dostring done!\n");
}

void dofile(const char* filename)
{
	PRINT("dofile:" << filename);
	init();

	FILE* f;
	if (!(f = fopen(filename, "rb")))
	{
		MSGBOX("dofile:" << filename << " file not found!");
		return;
	}

	int sp = ftell(f);
	fseek(f, 0, SEEK_END);
	int sz = ftell(f) - sp;
	char* buf = new char[sz + 1];
	fseek(f, 0, SEEK_SET);
	fread(buf, 1, sz, f);
	buf[sz] = '\0';
	fclose(f);

	dostring(buf);

	delete[]buf;
	PRINT("\n");
}
// API
inline void register_api(crstr name, api_fun fun)
{
	//PRINT("regAPI: " << name);
	api_list[name].fun = fun;
}
// dump
inline void dump_strstack(code& cd)
{
	PRINT("--- dump_strstack:")
	for (const auto& s : cd.strstack)
	{
		PRINT(s);
	}
}
//}
