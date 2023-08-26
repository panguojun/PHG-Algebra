/**************************************************************************
*				运算元素
*			本页代码定义一种代理式元素
*		通过类型控制以及资源ID来索引实际元素，从而达到优化效率的目的
**************************************************************************/
#ifdef ELEMENT
#undef ELEMENT
#endif
#define ELEMENT			GROUP::var_t
#define OBJ			ELEMENT
#define ENT			ELEMENT

#ifdef STRING2VAR
#undef STRING2VAR
#endif
#define STRING2VAR(str)		GROUP::var_t(str)
// ------------------------------------------
// 元素定义
// ------------------------------------------
typedef struct var_t
{
	using fun_set_t = std::function<void(const var_t& v)>;
	string sval;
	union {
		int ival = 0;
		real fval;
	};
	short resid = -1;
	enum { TYPE_DEFAULT = 1, TYPE_INT = 1,TYPE_REAL = 2, TYPE_S = 3, TYPE_UNKOWN = 0};
	short type = TYPE_DEFAULT; // 1 -int, 2 -real, 3 -string, 0 -unkown, other -custom
	fun_set_t fun_set = 0;

	var_t() { }
	var_t(int _val) {
		type = 1; ival = _val;
	}
	var_t(real _val) {
		type = 2; fval = _val;
	}
	var_t(bool _val) {
		type = 1; ival = (int)_val;
	}
	var_t(const char* _val) {
		type = 3; sval = _val;
		//PRINTV(sval);
	}
	var_t(const var_t& v)
	{
		//PRINT("var_t copy " << v.type);
		(*this) = v;
	}
	var_t(int _type, const char* _val) {
		type = _type; sval = _val;
	}
	void operator = (int v)
	{
		type = 1; ival = v; resid = -1;
	}
	void operator = (real v)
	{
		type = 2; fval = v; resid = -1;
	}
	void operator = (const var_t& v)
	{
		//PRINT("type ="  << v.type << " v.sval = " << v.sval);
		type = v.type; 
		if (type == 3)
			sval = v.sval;
		else if(type == 2)
			fval = v.fval;
		else if(type == 1)
			ival = v.ival;
		else if(fun_set)
			fun_set(v);
		else
			sval = v.sval;
		resid = v.resid;
	}
	bool operator == (int v) const
	{
		return type == 1 && ival == v;
	}
	bool operator != (int v) const
	{
		return type != 1 || ival != v;
	}
	bool operator == (const var_t& v) const
	{
		return  
			((type == 0 || v.type == 0 || !sval.empty()) && sval == v.sval) ||
			 (type == v.type && ((type == 1 && ival == v.ival) ||
			 (type == 2 && fval == v.fval))) ||
			 (type != v.type && float(*this) == float(v));
	}
	bool operator != (const var_t& v) const
	{
		return !(*this == v);
	}
	operator bool() const
	{
		//PRINT("var_t::int " << ival)
		if (type == 1)
			return ival != 0;
		if (type == 2)
			return (int)fval != 0;
		return !sval.empty() && sval != "";
	}
	operator int() const
	{
		//PRINT("var_t::int " << ival)
		if (type == 1)
			return ival;
		if (type == 2)
			return (int)fval;
		return atoi(sval.c_str());
	}
	operator float() const
	{
		//PRINT("var_t::float " << ival)
		if (type == 1)
			return float(ival);
		if (type == 2)
			return fval;
		return atof(sval.c_str());	
		//return 0.0f;
	}
	inline string tostr() const
	{
		//PRINT(type << ":" << sval);
		if (type == 1)
			return to_string(ival);
		if (type == 2)
			return to_string(fval);
		return sval;
	}
	var_t operator + (var_t& v) const
	{
		var_t ret;
		
		if (type == 1 && v.type == 1) 
			ret.ival = ival + v.ival; 
		else if (type == 3 || v.type == 3)
		{
			ret.type = 3;
			ret.sval = tostr() + v.tostr();
			//PRINTV(ret.sval)
		}
		else{
			ret.type = 2;
			ret.fval = float(*this) + float(v);
		}
		return ret;
	}
	void operator += (var_t& v)
	{
		ASSERT(type != 3 && v.type != 3);
		if (type == 1 && v.type == 1)
			ival += v.ival;
		else if(type == 2){
			fval += float(v);
		}
	}
	var_t operator - (var_t& v) const
	{
		ASSERT(type != 3 && v.type != 3);
		var_t ret;
		
		if (type == 1 && v.type == 1)
			ret.ival = ival - v.ival;
		else{
			ret.type = 2;
			ret.fval = float(*this) - float(v);
		}
		return ret;
	}
	void operator -= (var_t& v)
	{
		ASSERT(type != 3 && v.type != 3);
		if (type == 1 && v.type == 1) {
			ival -= v.ival;
		}
		else if (type == 2) {
			fval -= v.fval;
		}
	}
	var_t operator - () const
	{
		ASSERT(type != 3);
		var_t ret;
		ret.type = type;
		if (type == 1) {
			ret.ival = -ival;
		}
		else if (type == 2) {
			ret.fval = -fval;
		}
		return ret;
	}
	var_t operator * (var_t& v) const
	{
		ASSERT(type != 3 && v.type != 3);
		var_t ret;
		if (type == 1 && v.type == 1){
			ret.ival = ival * v.ival;
		}
		else{
			ret.type = 2;
			ret.fval = float(*this) * float(v);
		}
		return ret;
	}
	var_t operator / (var_t& v) const
	{
		ASSERT(type != 3 && v.type != 3);
		var_t ret;
		if (type == 2 || v.type == 2) {
			ret.type = 2;
			ret.fval = float(*this) / float(v);
		}
		else
		{
			ret.ival = ival / v.ival;
		}
		return ret;
	}
	bool operator > (const var_t& v) const
	{
		ASSERT(type != 3 && v.type != 3);
		if (type == 2 || v.type == 2) {
			return float(*this) > float(v);
		}
		else
		{
			return ival > v.ival;
		}
	}
	bool operator < (const var_t& v) const
	{
		ASSERT(type != 3 && v.type != 3);
		if (type == 2 || v.type == 2) {
			return float(*this) < float(v);
		}
		else
		{
			return ival < v.ival;
		}
	}
};

// ------------------------------------------
// 打印
// ------------------------------------------
void(*print_fun)(const var&) = 0;
inline void _PHGPRINT(const std::string& pre, const var& v)
{
	if (v.type == 1)
		PRINT(pre << v.ival)
	else if (v.type == 2)
		PRINT(pre << v.fval)
	else if (v.type == 3)
		PRINT(pre << v.sval)
	else
	{
		if (!v.sval.empty())
			PRINT(pre << v.sval)
		else if (!strlist.empty())
			PRINT(pre << strlist.back())

		if (print_fun)
			print_fun(v);
	}
}

// ------------------------------------------
// 运算符号定义 
// ------------------------------------------
#define CHECK_CALC	(c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '.' || c == '`')
#define CHECK_LOGIC	(c == '>' || c == '<' || c == '=' || c == '&' || c == '|' || c == '!')

// CALC RANK
#define RANK_INIT	\
	rank['|'] = 1;	\
	rank['&'] = 2;	\
	rank['>'] = 3;	\
	rank['<'] = 3;	\
	rank['='] = 3;	\
	rank['+'] = 4;	\
	rank['-'] = 4;	\
	rank['*'] = 5;	\
	rank['/'] = 5;	\
	rank['!'] = 6;	\
	rank['^'] = 7;	\
	rank['`'] = 8;	\
	rank['~'] = 8;	\
	rank['.'] = 9;

// ------------------------------------------
#define USE_STRING
#include "phg.hpp"
#undef USE_STRING
// ------------------------------------------
// 运算实现
// ------------------------------------------
using fun_calc_t = std::function<var(code& cd, opr o, int args)>;
fun_calc_t fun_calc = 0;
static var _act(code& cd, int args)
{
	opr o = cd.oprstack.pop();
	//PRINT("calc:" << (char)o << "(" << args << ")");

	if (fun_calc)
	{
		var ret = fun_calc(cd, o, args);
		if (ret != 0)
			return ret;
	}
	var ret = 0;
	switch (o) {
		case '+': {
			if (args > 1) {
				var& b = PHG_VALSTACK(1);
				var& a = PHG_VALSTACK(2);
				ret = a + b;
			}
			else {
				return cd.valstack.pop();
			}
		}break;
		case '+=': {
			if (args > 1) {
				var& b = PHG_VALSTACK(1);
				var& a = PHG_VALSTACK(2);
				ret = a + b;
				crstr a_name = GET_SPARAM(1);
				SET_SVAR(a_name, ret);
			}
			else {
				return cd.valstack.pop();
			}
		}break;
		case '-': {
			if (args > 1) {
				var& b = PHG_VALSTACK(1);
				var& a = PHG_VALSTACK(2);
				ret = a - b;
			}
			else {
				return -cd.valstack.pop();
			}
		}break;
		case '*': {
			if (args > 1) {
				var& b = PHG_VALSTACK(1);
				var& a = PHG_VALSTACK(2);
				ret = a * b;
			}
			else {
				return cd.valstack.pop();
			}
		}break;
		case '/': {
			if (args > 1) {
				var& b = PHG_VALSTACK(1);
				var& a = PHG_VALSTACK(2);
				ret = a / b;
			}
			else {
				return cd.valstack.pop();
			}
		}break;
		case '^': {
			var& b = PHG_VALSTACK(1);
			var& a = PHG_VALSTACK(2);
			ret = pow((real)a, (real)b);
		}break;
		case '=': {
			var& b = PHG_VALSTACK(1);
			var& a = PHG_VALSTACK(2);
			ret = var(int(a == b));
		}break;
		case '>': {
			var& b = PHG_VALSTACK(1);
			var& a = PHG_VALSTACK(2);
			ret = a > b;
		}break;
		case '<': {
			var& b = PHG_VALSTACK(1);
			var& a = PHG_VALSTACK(2);
			ret = a < b;
		}break;
		case '&': {
			var& b = PHG_VALSTACK(1);
			var& a = PHG_VALSTACK(2);
			ret = int(a) && int(b);
		}break;
		case '|': {
			var& b = PHG_VALSTACK(1);
			var& a = PHG_VALSTACK(2);
			ret = int(a) || int(b);
		}break;
		case '!': {
			if (args > 1) {
				var& b = PHG_VALSTACK(1);
				var& a = PHG_VALSTACK(2);
				ret = !(a == b);
			}
			else {
				var a = cd.valstack.pop();
				return !int(a);
			}
		}break;
		default: {}
	}
	PHG_VALPOP(args);
	return ret;
}
