#undef var
#undef INVALIDVAR
#undef rank
#undef fun_t
#undef parser_fun
#undef statement_fun
#undef tree_fun
#undef callfunc
#undef gvarmapstack

#define var			ELEMENT
#define VAR			ELEMENT
#define INVALIDVAR	ELEMENT(0)
#ifndef STRING2VAR
#define STRING2VAR(str)	INVALIDVAR
#endif

#undef PHGPRINT
#define PHGPRINT	GROUP::_PHGPRINT
#define GROUPNAME	#GROUP

#define PHG_VAR(name, defaultval) (GROUP::gvarmapstack.stack.empty() || GROUP::gvarmapstack.stack.front().find(#name) == GROUP::gvarmapstack.stack.front().end() ? (var)defaultval : GROUP::gvarmapstack.stack.front()[#name])
#define PHG_PARAM(index)	cd.valstack.get(args - index)
#define PHG_POP_PARAMS(n) { for (int i = 0; i < n; i++) cd.valstack.pop(); }
#define PHG_VALSTACK(index)	cd.valstack.get(index - 1)
#define PHG_VALPARAM(index)	cd.valstack.get(index - 1)
#define PHG_VALPOP(n)	for(int i = 0; i < n; i ++) cd.valstack.pop_back();
#define PHG_RET_VAL(ret,args)	PHG_VALPOP(args) return ret;

// 注意拷贝构造，以及赋值函数的行为
#define DEFAULT_ELEMENT	\
	ELEMENT() {}\
	ELEMENT(int _val) {}\
	operator int (){return 1;}\
	bool operator == (const ELEMENT& v) const{return false;}

#define DEFAULT_ELEMENT_LT \
	ELEMENT() {}\
	ELEMENT(int _val) {}


// base
struct varbase_t
{
	union {
		int ival = 0;
		real fval;
	};
	int resid = -1;
	int type = 1; // 1 -int, 2 -real, others

	varbase_t() { }
	varbase_t(int _val) {
		type = 1; ival = _val; resid = -1;
	}
	varbase_t(real _val) {
		type = 2; fval = _val; resid = -1;
	}
	varbase_t(const varbase_t& v)
	{
		//PRINT("varbase_t copy " << v.type);
		(*this) = v;
	}

	void operator = (const varbase_t& v)
	{
		//PRINT("var_t ="  << v.type << "," << v.fval);
		type = v.type;
		if (type == 2)
			fval = v.fval;
		else
			ival = v.ival;
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
	bool operator == (real v) const
	{
		return type == 2 && fval == v;
	}
	bool operator != (real v) const
	{
		return type != 2 || fval != v;
	}
	bool operator == (const varbase_t& v) const
	{
		return type == v.type &&
			((type == 1 && ival == v.ival) || (type == 2 && fval == v.fval));
	}
	bool operator != (const varbase_t& v) const
	{
		return !(*this == v);
	}

	operator int() const
	{
		//PRINT("varbase_t::int " << ival)
		return ival;
	}

	varbase_t operator + (varbase_t& v) const
	{
		varbase_t ret;
		if (type == 2 || v.type == 2) {
			ret.type = 2;
			ret.fval = fval + v.fval;
		}
		else
		{
			ret.ival = ival + v.ival;
		}
		return ret;
	}
	varbase_t operator - (varbase_t& v) const
	{
		varbase_t ret;
		if (type == 2 || v.type == 2) {
			ret.type = 2;
			ret.fval = fval - v.fval;
		}
		else
		{
			ret.ival = ival - v.ival;
		}
		return ret;
	}
	varbase_t operator - () const
	{
		varbase_t ret;
		if (type == 2) {
			ret.type = 2;
			ret.fval = -fval;
		}
		else
		{
			ret.ival = -ival;
		}
		return ret;
	}
};
#define VAR_BASE(name) \
	name(int _ival) : varbase_t(_ival) {} \
	name(real _fval) : varbase_t(_fval) {} \
	bool operator == (int v) const { \
		return varbase_t::operator==(v); \
	} \
	bool operator != (int v) const { \
		return varbase_t::operator!=(v); \
	}
