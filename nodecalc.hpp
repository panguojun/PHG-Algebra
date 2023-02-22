/**************************************************************************
*							节点树的运算
*					 可能可以实现一些类似AI的功能
**************************************************************************/
struct tree_t;
namespace nodecalc
{
	bool abelian_sym = true;	// 阿贝尔对称 即运算的可交换性

	tree_t* _walk_tree_node(std::string& str, tree_t* tree, crstr a, const char* key)
	{
		if (tree->kv[key] == a)
		{
			return tree;
		}

		// children
		for (auto it : tree->children) {
			tree_t* t = _walk_tree_node(str, it.second, a, key);
			if (t)
				return t;
		}
		return 0;
	}
	int _walk_tree_ancestor(tree_t** ancestor, tree_t* tree, crstr a, crstr b, const char* key)
	{
		if (tree->kv[key] == a)
			return 1;
		if (tree->kv[key] == b)
			return 2;

		// children
		int flag = 0;
		for (auto it : tree->children) {
			int ret = _walk_tree_ancestor(ancestor, it.second, a, b, key);

			if (ret == 3)
				return ret;

			if (ret)
			{
				flag |= ret;
				if (flag == 3)
				{
					*ancestor = tree;
					PRINT("found ancestor!")
						return flag;
				}
			}
		}
		return flag;
	}
	int _calc_add(tree_t** out, tree_t* tree, crstr a, crstr b, const char* key)
	{
		{// kv
			crstr v = tree->kv[key];
			//PRINT("key:" << key << " val:" << v);
			if (v == a)
				return 1;
			if (v == b)
				return 2;
		}

		// children
		int flag = 0;
		tree_t* lstn = 0;
		for (auto& it : tree->children) {
			int ret = _calc_add(out, it.second, a, b, key);
			//PRINTV(ret);
			if (ret == 3)
				return ret;

			if (ret && tree != ROOT)
			{
				flag |= ret;
				if (flag == 3)
				{
					if (!abelian_sym)
					{// 顺序判断
						if (lstn != 0)
						{
							PRINTV(it.second->getindex());
							PRINTV(lstn->getindex());
							if (ret == 1 && it.second->getindex() > lstn->getindex() || ret == 2 && it.second->getindex() < lstn->getindex())
							{
								PRINT("order failed!")
									return 0;
							}
						}
					}
					*out = tree;
					//str = tree->kv[key];
					return flag;
				}
				lstn = it.second;
			}
		}
		return flag;
	}
	void _calc_addd(std::string& str, crstr a, const char* key)
	{
		NODE* node = _walk_tree_node(str, ROOT, a, key);
		if (node->parent)
			str = node->parent->kv[key];
	}
	void _calc_sub(std::string& str, crstr a, crstr b, const char* key)
	{
		if (a == b) {
			str = a;
			return;
		}
		NODE* node = _walk_tree_node(str, ROOT, a, key);

		if (node)
		{
			bool bina = false;
			for (auto& it : node->children)
			{
				if (auto& v = it.second->kv[key]; v == b)
				{
					bina = true;
					break;
				}
			}
			if (bina) {
				for (auto& it : node->children)
				{
					if (auto& v = it.second->kv[key]; v != b)
					{
						str = v;
						return;
					}
				}
			}
		}
	}
	void _calc_subb(std::string& str, crstr a, const char* key)
	{
		NODE* node = _walk_tree_node(str, ROOT, a, key);
		if (!node->children.empty())
		{
			for (auto& it : node->children)
			{
				str = it.second->kv[key];
				return;
			}
		}
	}
	void _wak_tree(tree_t* tree, crstr a, const char* k, const char* ok)
	{
		if (tree->kv[k] == a)
		{
			strlist.push_back(tree->kv[ok]);
		}

		// children
		for (auto it : tree->children) {
			_wak_tree(it.second, a, k, ok);
		}
	}
	int _calc_add(tree_t* tree, crstr a, crstr b, const char* key, const char* ok)
	{
		{// kv
			crstr v = tree->kv[key];
			//PRINT("key:" << key << " val:" << v);
			if (v == a)
				return 1;
			if (v == b)
				return 2;
		}

		// children
		int flag = 0;
		tree_t* lstn = 0;
		for (auto& it : tree->children) {
			int ret = _calc_add(it.second, a, b, key, ok);
			//PRINTV(ret);
			if (ret == 3)
				return ret;

			if (ret && tree != ROOT)
			{
				flag |= ret;
				if (flag == 3)
				{
					if (!abelian_sym)
					{// 顺序判断
						if (lstn != 0)
						{
							PRINTV(it.second->getindex());
							PRINTV(lstn->getindex());
							if (ret == 1 && it.second->getindex() > lstn->getindex() || ret == 2 && it.second->getindex() < lstn->getindex())
							{
								PRINT("order failed!")
									return 0;
							}
						}
					}
					strlist.push_back(tree->kv[ok]);
					return flag;
				}
				lstn = it.second;
			}
		}
		return flag;
	}
}

// ------------------------------------
// API
// ------------------------------------
_API(calc_set_abelian)
{
	crstr b = GET_SPARAM(1);

	nodecalc::abelian_sym = atoi(b.c_str());
	PRINTV(nodecalc::abelian_sym);

	POP_SPARAM; return 0;
}
_API(calc_addd)
{
	crstr a = GET_SPARAM(1);
	string c;
	nodecalc::_calc_addd(c, a, cur_property.c_str());

	PRINTV(c);
	strlist.push_back(c);

	POP_SPARAM; return 0;
}
_API(calc_add)
{
	string a = GET_SPARAM(1);
	string b = GET_SPARAM(2);
	{
		NODE* an = GET_NODE(a, ROOT); ASSERT(an);
		NODE* bn = GET_NODE(b, ROOT); ASSERT(bn);
		a = an->kv[cur_property];
		b = bn->kv[cur_property];
	}

	NODE* n = 0;
	string c;
	if (a == b) {
		c = a;
	}
	else {
		nodecalc::_calc_add(&n, ROOT,
			a, b,
			cur_property.c_str());
		if (n) {
			c = n->kv[cur_property];

			PRINTV(n->name);
		}
	}
	strlist.push_back(c);

	if (ME) {
		if (n) *(work_stack.back()) += *n;
		ME->kv[cur_property] = c;
	}

	POP_SPARAM; return 0;
}
_API(calc_subb)
{
	crstr a = GET_SPARAM(1);
	string c;
	nodecalc::_calc_subb(c, a, cur_property.c_str());

	PRINTV(c);
	strlist.push_back(c);

	POP_SPARAM; return 0;
}
_API(calc_sub)
{
	crstr a = GET_SPARAM(1);
	crstr b = GET_SPARAM(2);
	string c;
	nodecalc::_calc_sub(c, a, b, cur_property.c_str());

	PRINTV(c);
	strlist.push_back(c);

	POP_SPARAM; return 0;
}
_API(calc_wak)
{
	if (args == 2)
	{
		// on('md');wak('pr1','a + b');
		NODE* n = 0;
		crstr key = GET_SPARAM(1);
		crstr expr = GET_SPARAM(2);
		code ccd(expr.c_str());
		string a = ccd.getname();
		ccd.next3();
		char op = ccd.cur();
		ccd.next();
		string b = ccd.getname();
		if (op == '+')
			nodecalc::_calc_add(n, a, b, key.c_str(), cur_property.c_str());
		else if (op == '-')
		{
			string c;
			nodecalc::_calc_sub(c, a, b, key.c_str());

			PRINTV(c);
			strlist.push_back(c);
		}
	}
	else
	{
		crstr nm = GET_SPARAM(1);
		NODE* n = nm == "me" ? ME : GET_NODE(nm, ROOT); ASSERT(n);
		if (args == 3) {
			crstr a = GET_SPARAM(2);
			crstr ok = GET_SPARAM(3);

			nodecalc::_wak_tree(n, a, cur_property.c_str(), ok.c_str());
		}
		else if (args == 4)
		{
			crstr a = GET_SPARAM(2);
			crstr b = GET_SPARAM(3);
			crstr ok = GET_SPARAM(4);

			nodecalc::_calc_add(n, a, b, cur_property.c_str(), ok.c_str());
		}
	}
	POP_SPARAM; return 0;
}

_API(clearstrlist)
{
	strlist.clear();
	return 0;
}

#ifdef EXPORT
// -----------------------------------
// 数据输出 I/O
// -----------------------------------
inline string fixedname(crstr name)
{
	const char* p = name.c_str();
	if (*p == '\'' || *p == '\"')
	{
		string ret = p + 1;
		ret.pop_back();
		return ret;
	}
	return name;
}
inline void fixedproperty(string& str)
{
	str.erase(std::remove(str.begin(), str.end(), '\''), str.end());
	str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());

}
inline rect_t torect(crstr str)
{
	rect_t rect;
	sscanf(str.c_str(), "{x:%d,y:%d,width:%d,height:%d}", &rect.x, &rect.y, &rect.width, &rect.height);
	return rect;
}
_API(getrect)
{
	rectlist.clear();
	NODE* node = ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_NODE(param1, ROOT);
		if (!node)
		{
			ERRORMSG("Node:" << param1 << " not found!")
				return 0;
		}
	}
	string key = "rect";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	node_walker(node, [key](tree_t* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				string str = it->second;
				fixedproperty(str);
				rectlist.push_back(torect(str));
			}
		});
	POP_SPARAM;
	return 0;
}
inline vec3 tovec3(crstr str)
{
	vec3 v;
	sscanf(str.c_str(), "%f,%f,%f", &v.x, &v.y, &v.z);
	return v;
}
_API(getvec3)
{
	vec3list.clear();
	NODE* node = ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_NODE(param1, ROOT);
		if (!node)
			return 0;
	}
	string key = "p";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	node_walker(node, [key](tree_t* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				PRINTV(it->second);

				string str = it->second;
				fixedproperty(str);
				vec3list.push_back(tovec3(str));
			}
		});
	POP_SPARAM;
	return 0;
}
_API(getfval)
{
	reallist.clear();
	NODE* node = ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_NODE(param1, ROOT);
		if (!node)
			return 0;
	}
	string key = "x";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	node_walker(node, [key](tree_t* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				string str = it->second;
				fixedproperty(str);
				reallist.push_back(atof(str.c_str()));
			}
		});
	POP_SPARAM;
	return 0;
}
_API(getival)
{
	intlist.clear();
	NODE* node = ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_NODE(param1, ROOT);
		if (!node)
			return 0;
	}
	string key = "x";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	node_walker(node, [key](tree_t* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				string str = it->second;
				fixedproperty(str);
				intlist.push_back(atoi(str.c_str()));
			}
		});
	POP_SPARAM;
	return 0;
}
_API(getstr)
{
	string param1 = GET_SPARAM(1);
	string param2 = GET_SPARAM(2);

	//strlist.clear();
	ScePHG::node_walker(ROOT, [param1, param2](ScePHG::tree_t* tree)->void
		{
			if (tree->name == param1) {
				auto it = tree->kv.find(param2);
				if (it != tree->kv.end())
				{
					strlist.push_back(it->second.c_str());
				}
			}
		});
	POP_SPARAM;

	PRINTV(strlist.size());
	return 0;
}
#endif

void NODECALC_REG_API()
{	
	_REG_API(abe, calc_set_abelian);

	_REG_API(addd, calc_addd);
	_REG_API(add, calc_add);
	_REG_API(subb, calc_subb);
	_REG_API(sub, calc_sub);
	_REG_API(calc, calc_wak);

#ifdef EXPORT
	_REG_API(getival, getival);		// 获得int value
	_REG_API(getfval, getfval);		// 获得float value
	_REG_API(getstr, getstr);		// 获得string
	_REG_API(getvec3, getvec3);		// 获得RECT
	_REG_API(getrect, getrect);		// 获得RECT

	_REG_API(cls, clearstrlist);
#endif
}
