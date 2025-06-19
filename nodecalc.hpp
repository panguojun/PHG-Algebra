/**************************************************************************
*                        Node Tree Operations
*          Potentially capable of implementing AI-like functionalities
**************************************************************************/
namespace nodecalc
{
	bool abelian_sym = true;	// 阿贝尔对称 即运算的可交换性

	phg_tree* _walk_tree_node(std::string& str, phg_tree* tree, crstr a, const char* key)
	{
		if (tree->kv[key] == a)
		{
			return tree;
		}

		// children
		for (auto it : tree->children) {
			phg_tree* t = _walk_tree_node(str, it.second, a, key);
			if (t)
				return t;
		}
		return 0;
	}
	int _walk_tree_ancestor(phg_tree** ancestor, phg_tree* tree, crstr a, crstr b, const char* key)
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
	int _calc_add(phg_tree** out, phg_tree* tree, crstr a, crstr b, const char* key)
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
		phg_tree* lstn = 0;
		for (auto& it : tree->children) {
			int ret = _calc_add(out, it.second, a, b, key);
			//PRINTV(ret);
			if (ret == 3)
				return ret;

			if (ret && tree != PHG_ROOT)
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
		PHG_NODE* node = _walk_tree_node(str, PHG_ROOT, a, key);
		if (node->parent)
			str = node->parent->kv[key];
	}
	void _calc_sub(std::string& str, crstr a, crstr b, const char* key)
	{
		if (a == b) {
			str = a;
			return;
		}
		PHG_NODE* node = _walk_tree_node(str, PHG_ROOT, a, key);

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
		PHG_NODE* node = _walk_tree_node(str, PHG_ROOT, a, key);
		if (!node->children.empty())
		{
			for (auto& it : node->children)
			{
				str = it.second->kv[key];
				return;
			}
		}
	}
	void _wak_tree(phg_tree* tree, crstr a, const char* k, const char* ok)
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
	int _calc_add(phg_tree* tree, crstr a, crstr b, const char* key, const char* ok)
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
		phg_tree* lstn = 0;
		for (auto& it : tree->children) {
			int ret = _calc_add(it.second, a, b, key, ok);
			//PRINTV(ret);
			if (ret == 3)
				return ret;

			if (ret && tree != PHG_ROOT)
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
_API(calc_add)
{
	string a = GET_SPARAM(1);
	string b = GET_SPARAM(2);
	{
		PHG_NODE* an = GET_PHG_NODE(a, PHG_ROOT); ASSERT(an);
		PHG_NODE* bn = GET_PHG_NODE(b, PHG_ROOT); ASSERT(bn);
		a = an->kv[cur_property];
		b = bn->kv[cur_property];
	}

	PHG_NODE* n = 0;
	string c;
	if (a == b) {
		c = a;
	}
	else {
		nodecalc::_calc_add(&n, PHG_ROOT,
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
		PHG_NODE* n = 0;
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
		PHG_NODE* n = nm == "me" ? ME : GET_PHG_NODE(nm, PHG_ROOT); ASSERT(n);
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
inline vec3 tovec3(crstr str)
{
	vec3 v;
	sscanf(str.c_str(), "%f,%f,%f", &v.x, &v.y, &v.z);
	return v;
}
_API(getvec3)
{
	PHG_NODE* node = PHG_ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_PHG_NODE(param1, PHG_ROOT);
		if (!node)
			return 0;
	}
	string key = "p";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	phg_node_walk(node, [key](phg_tree* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				//PRINTV(it->second);

				string str = it->second;
				STR::fixedproperty(str);
			}
		});
	POP_SPARAM;
	return 0;
}
_API(getfval)
{
	PHG_NODE* node = PHG_ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_PHG_NODE(param1, PHG_ROOT);
		if (!node)
			return 0;
	}
	string key = "x";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	phg_node_walk(node, [key](phg_tree* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				string str = it->second;
				STR::fixedproperty(str);
			}
		});
	POP_SPARAM;
	return 0;
}
_API(getival)
{
	PHG_NODE* node = PHG_ROOT;
	if (args > 0)
	{
		string param1 = GET_SPARAM(1);
		node = GET_PHG_NODE(param1, PHG_ROOT);
		if (!node)
			return 0;
	}
	string key = "x";
	if (args > 1)
	{
		key = GET_SPARAM(2);
	}
	phg_node_walk(node, [key](phg_tree* tree)->void
		{
			auto it = tree->kv.find(key);
			if (it != tree->kv.end())
			{
				string str = it->second;
				STR::fixedproperty(str);
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
	ScePHG::phg_node_walk(PHG_ROOT, [param1, param2](ScePHG::phg_tree* tree)->void
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
	_REG_API(add, calc_add);
	_REG_API(sub, calc_sub);
	_REG_API(calc, calc_wak);

#ifdef EXPORT
	_REG_API(getival, getival);		// 获得int value
	_REG_API(getfval, getfval);		// 获得float value
	_REG_API(getstr,  getstr);		// 获得string
	_REG_API(getvec3, getvec3);		// 获得vec3

	_REG_API(cls, clearstrlist);
#endif
}
