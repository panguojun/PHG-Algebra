/**************************************************************************
*						节点树的解析
*				节点树就是把个体组成家族
*				包括阵列，序列, 选择子等重要概念
*					任何对象都可以定义为节点
**************************************************************************/
#define NODE		GROUP::tree_t
#define ROOT		GROUP::gtree
#define ME			GROUP::work_stack.empty() ? 0 : GROUP::work_stack.back()
#define GET_NODE(name, node)	name == "me" ? ME : get_node(name, node, true)

int node_count = 0;								// 节点数量

typedef struct tree_t
{
	tree_t* parent = 0;
	string name;								// 名字
	int index = 0;								// 索引
	bool b_kv_ordered = false;					// kv表有序

	std::map<std::string, std::string> kv;		// 属性字典
	std::vector<std::string> kvorder;			// 属性字典的顺序

	std::map<std::string, tree_t*> children;	// 子字典
	std::vector<tree_t*>	childlist;			// 子列表，用于json/xml等格式转化

	static inline int getdepth(tree_t* tree, int depth = 0)
	{
		//PRINT("getdepth: " << tree->name);
		if (tree->parent)
			return getdepth(tree->parent, depth + 1);
		else
			return depth;
	}
	static inline string genid0()
	{
		node_count++;
		if (node_count >= 10000) // 注意：id < 10000
			return to_string(node_count);

		// 简单字典顺序
		return	to_string(node_count/1000%10) +
				to_string(node_count/100%10) +
				to_string(node_count/10%10) +
				to_string(node_count%10);
	}
	static inline string genid()
	{
		node_count++;
		if (node_count >= 10000) // 注意：id < 10000
			return to_string(node_count);

		string str = to_string(node_count);
		int len = str.length();
		for (int i = 0; i < 4 - len; i++) {
			str = "0" + str;
		}
		return str;
	}
	inline int getindex()
	{
		return index;
		/*int id, index;
		PRINTV(name);
		ASSERT(2 == sscanf(name.c_str(), "%d_%d", &id, &index));
		return id;*/
	}
	void operator += (const tree_t& t)
	{
		// 暂时拷贝，以后可以做运算

		for (auto it : t.kv)
		{
			kv[it.first] = it.second;
		}
		for (auto& it : t.children)
		{
			tree_t* ntree = new tree_t();
			ntree->index = children.size() + 1;
			ntree->name = string("n") + tree_t::genid();// +"_" + to_string(ntree->index);
			children[ntree->name] = ntree;
			ntree->parent = this;

			*ntree += *it.second;
		}
	}
	void copyprop(tree_t* t)
	{
		for (auto& it : kv)
		{
			t->kv[it.first] = it.second;
		}
	}
	static void clear(tree_t* ot)
	{
		if (!ot) return;

		for (auto it : ot->children)
		{
			clear(it.second);
		}
		delete(ot);
	}
}tree_t;

// -----------------------------------------------------------------------
tree_t* gtree = 0;						// 暂时使用全局树
vector<tree_t*>	work_stack;				// 工作栈
std::string		cur_property = "pr1";	// 当前属性
extern void _crt_array(code& cd, tree_t* tree, const string& pre, int depth, const string& selector);
extern void _crt_sequ(code& cd, tree_t* tree, const string& pre);
extern tree_t* get_node(const string& name, tree_t* tree, bool recu);

// -----------------------------------------------------------------------
static void _tree(code& cd, tree_t* tree, const string& pre, int depth = 0)
{
	work_stack.push_back(tree);
	//PRINT(pre << "{");
	cd.next();

	std::string key, val;
	std::string* pstr = &key;
	string selector;
	while (!cd.eoc()) {
		char c = cd.cur();
		//PRINT("c=" << c );

		// 注解
		if (c == '#') {
			cd.nextline();
			//cd.next();
			continue;
		}

		// PHG表达式
		else if (c == '(' && pstr != &val)
		{
			int bracket_d = 1;
			string phg_expr = "";
			while (true) {
				char nc = cd.next();

				if (nc == '(')
					bracket_d++;
				else if (nc == ')')
					bracket_d--;

				if (bracket_d == 0)
					break;

				phg_expr.push_back(nc);
			}
			{// do expr
				work_stack.push_back(tree);
				GROUP::dostring(phg_expr.c_str());
			}
			cd.next();
		}

		// 选择子
		else if (c == '?' && pstr == &key) {
			cd.next();
			selector = getstring(cd, '\'', '\"', '[');
			cd.ptr--; // move back
		}

		// 节点开始
		else if (c == '{' || c == '[' || c == '<') {
			if (!val.empty()) {
				if (key.empty())
					key = "pr" + to_string(tree->kv.size() + 1); // default porperty name
				tree->kv[key] = val;
				if (tree->b_kv_ordered)
					tree->kvorder.push_back(key);
			}

			if (c == '{') {
				if (val != "")
				{
					SYNTAXERR("missing ';' to end an property(k:v) line!");
					cd.ptr = 0;
				}
				if (key.empty())
				{
					key = string("n") + tree_t::genid();// +"_" + to_string(tree->children.size() + 1);
				}
				tree_t* ntree = new tree_t;
				ntree->index = tree->children.size() + 1;
				ntree->name = key;
				tree->children[key] = ntree;
				ntree->parent = tree;

				//PRINT(pre << key << " : ");
				_tree(cd, ntree, pre + "\t", depth + 1);
			}
			else if (c == '[') // 阵列
			{
				_crt_array(cd, tree, pre, depth + 1, selector);
			}
			else if (c == '<') // 序列
			{
				_crt_sequ(cd, tree, pre);
			}
			val = "";
			key = "";
			pstr = &key;
		}

		// 节点结束
		else if (c == ';' || c == '}' || c == '>' || c == '\n' || c == '\r') {
			if (!key.empty() || !val.empty()) {
				if (val.empty())
				{// inhert
					tree_t* t = GET_NODE(key, ROOT);
					if (t) {
						(*tree) += (*t);
					}
					else
					{
						tree->kv[key] = ""; // key with an empty value 
					}
				}
				else
				{
					tree->kv[key] = val;
					if (tree->b_kv_ordered)
						tree->kvorder.push_back(key);
					//PRINT(pre << key << " : " << val);
				}
				val = "";
				key = "";
				pstr = &key;
			}
			cd.next();
			if (c == '}' || c == '>') {
				return;
			}
		}

		// 逗号间隔,不能作为property的结尾！
		else if (c == ',' && pstr != &val) {
			if (!key.empty())
			{// inhert
				tree_t* t = GET_NODE(key, ROOT);
				if (t)
				{
					(*tree) += (*t);
				}
			}
			val = "";
			key = "";
			pstr = &key;

			cd.next();
		}

		// 名字 与 数值/子节点
		else if (c == ':' || c == '=') {
			if (key.empty())
			{
				//SYNTAXERR("key is missing before ':' !");cd.ptr = 0;return;
				key = "pr" + to_string(tree->kv.size() + 1); // default porperty name
			}
			if (pstr == &val) {
				SYNTAXERR("':' or '=' is in value!");
				cd.ptr = 0;
				return;
			}
			pstr = &val;
			cd.next4();

			c = cd.cur();
			if (c != '\'' && c != '\"')
			{// 读到尾部
				*pstr += c;
				while(true){
					c = *(++cd.ptr);
					if (c == 0 || c == ';' || c == '}' || c == '\n' || c == '\r')
						break;
					*pstr += c;
				}
			}
		}

		// 字符串
		else if (c == '\'' || c == '\"') {

			cd.next();
			*pstr += getstring(cd, c, c, c);
		}

		// default
		else {
			*pstr += cd.cur();
			cd.next4();
		}
	}
}

tree_t* get_node(const string& name, tree_t* tree = gtree, bool recu = true)
{
	if (name == tree->name)
		return tree;
	{
		auto it = tree->children.find(name);
		if (it != tree->children.end())
		{
			return it->second;
		}
	}
	if (recu)
	{
		for (auto it : tree->children) {
			tree_t* ret = get_node(name, it.second, recu);
			if (ret)
				return ret;
		}
	}
	return 0;
}
int select(int ind, int rnd, crstr selector)
{
	if (selector.empty())
		return 2; // select all

	int pa, len;
	sscanf(selector.c_str(), "%d/%d", &pa, &len);
	ASSERT(len != 0);

	if (rnd % len == ind) // 随机选择一个
	{
		PRINT("selected!");
		return 1; // select one
	}

	PRINT("select failed! " << ind);
	return 0;
}

// 阵列
void _crt_array(code& cd, tree_t* tree, const string& pre, int depth, const string& selector)
{
	cd.next();
	int index = 0;
	int rnd = rand();
	//vector<string> nodes;
	string node;
	while (!cd.eoc()) 
	{
		char c = cd.cur();
		//PRINT(c);
		tree_t* ntree = 0;
		if (c == '#') {
			cd.nextline();
			cd.next();
			continue;
		}
		else if (c == '<' || c == '>') {
			SYNTAXERR("'<' or '>' is not allowed in '[]', use '{}'!");
			cd.ptr = 0;
			break;
		}
		else if (c == ':' || c == '=')
		{
			if (node == "") {
				SYNTAXERR("key is missing before ':' or '='!");
				cd.ptr = 0;
				break;
			}
		}
		else if (c == '{')
		{
			index++;
			work_stack.push_back(tree);
			ntree = new tree_t;
			ntree->index = tree->children.size() + 1;
			if (node.empty())
			{
				node = string("n") + tree_t::genid();// + "_" + to_string(ntree->index);
			}
			ntree->name = node;

			tree->children[ntree->name] = ntree;
			ntree->parent = tree;

			//PRINT("_crt_array >>");
			_tree(cd, ntree, pre, depth + 1);
			// PRINT("_crt_array << " << cd.cur())
			int ret = select(index, rnd, selector);
			if (ret) {
				if (ret == 1)
				{
					cd.next();
					break;
				}
			}
			else
			{
				tree->children[ntree->name] = 0;
				ntree->parent = 0;
				tree_t::clear(ntree);
			}
			node = "";
		}
		else if (c == ',' || c == ']'){}
		else
			node += c;

		c = cd.cur();
		if (c == ',' || c == ']')
		{
			if (!ntree)
				index++;

			//nodes.push_back(node);
			if (!ntree)
			{// create node
				work_stack.push_back(tree);
				ntree = new tree_t;
				ntree->index = tree->children.size() + 1;
				ntree->name = string("n") + tree_t::genid();// + "_" + to_string(ntree->index);
				{// inhert
					tree_t* t = GET_NODE(node, ROOT);
					if (t) {
						(*ntree) += (*t);
					}
				}

				//PRINTV(ntree->name)
				int ret = select(index, rnd, selector);
				if (ret) {

					tree->children[ntree->name] = ntree;
					ntree->parent = tree;
					if (ret == 1)
					{
						cd.next();
						//PRINTV(ret);
						break;
					}
				}
				else
				{
					tree_t::clear(ntree);
				}
				ntree = 0; // clear
			}
			node = "";

			if (c == ']')
			{
				cd.next();
				break;
			}
		}
		cd.next();
	}
}

// 序列
void _crt_sequ(code& cd, tree_t* tree, const string& pre)
{
	cd.next();

	//vector<string> nodes;
	string node;
	while (!cd.eoc())
	{
		char c = cd.cur();
		//PRINT(c)
		tree_t* ntree = 0;
		if (c == '#') {
			cd.nextline();
			cd.next();
			continue;
		}
		else if (c == '[' || c == ']') {
			SYNTAXERR("'[' or ']' is not allowed in '<>', use '{}'!");
			cd.ptr = 0;
			break;
		}
		else if (c == ':')
		{
			if (node == "") {
				SYNTAXERR("key is missing before ':'!");
				cd.ptr = 0;
				break;
			}
		}
		else if (c == '{')
		{
			work_stack.push_back(tree);
			ntree = new tree_t;
			ntree->index = tree->children.size() + 1;
			if (node.empty())
			{
				node = string("n") + tree_t::genid();// +"_" + to_string(ntree->index);
			}
			ntree->name = node;
			tree->children[ntree->name] = ntree;
			ntree->parent = tree;

			//PRINT(pre << ntree->name << " : ");

			_tree(cd, ntree, pre, tree_t::getdepth(tree) + 1);
			tree = ntree; // parent->child
			//PRINTV(cd.cur())
			node = "";
		}
		else if (c == ',' || c == '>')
		{
		}
		else
			node += c;

		c = cd.cur();
		if (c == ',' || c == '>')
		{
			//nodes.push_back(node);
			if (!ntree)
			{// create node
				work_stack.push_back(tree);
				ntree = new tree_t;
				ntree->index = tree->children.size() + 1;
				ntree->name = string("n") + tree_t::genid();// +"_" + to_string(tree->children.size() + 1);
				{// inhert
					tree_t* t = GET_NODE(node, ROOT);
					if (t) {
						(*ntree) += (*t);
					}
				}
				tree->children[ntree->name] = ntree;
				ntree->parent = tree;
				tree = ntree; // parent->child

				ntree = 0; // clear
			}
			node = "";

			if (c == '>')
			{
				cd.next();
				break;
			}
		}
		cd.next();
	}
}
// 树节点解析
void _tree(code& cd)
{
	//tree_t* tree = new tree_t;
	tree_t::clear(gtree);
	node_count = 0;
	gtree = new tree_t;
	gtree->name = "root";
	_tree(cd, gtree, "", 0);
	
	//tree_t::clear(tree);
}

// ------------------------------------
// 寻源式加法
// 两个节点之和为最近的相同祖先
// ------------------------------------
bool porperty_intree(tree_t* tree, const char* key, crstr val)
{
	auto it = tree->kv.find(key);
	if (it != tree->kv.end())
		return  it->second == val;

	// children
	for (auto it : tree->children) {
		if (porperty_intree(it.second, key, val))
			return true;
	}
	return false;
}
// 在节点树上搜索加法规则
const char* walk_addtree(tree_t* tree, crstr v_a, crstr v_b, const char* key)
{
	if (tree->children.size() >= 2)
	{
		int findcnt = 0;
		for (auto it : tree->children) {
			if (it.second->children.empty() && it.second->kv[key] == v_a)
			{
				findcnt++;
			}
			if (it.second->children.empty() && it.second->kv[key] == v_b)
			{
				findcnt++;
			}
		}

		if (findcnt == 2)
		{
			return tree->kv[key].c_str();
		}
	}

	for (auto it : tree->children) {
		const char* c = walk_addtree(it.second, v_a, v_b, key);
		if (c != 0)
		{
			return c;
		}
	}
	return 0;
}
// ------------------------------------
// node walker
// ------------------------------------
void node_walker(tree_t* tree, std::function<void(tree_t*)> fun)
{
	fun(tree);

	// children
	for (auto& it : tree->children) {
		node_walker(it.second, fun);
	}
}
// ====================================
// API
// ====================================
tree_t* _getbyprop(tree_t* tree, crstr key, crstr val)
{
	if (tree->kv[key] == val){
		return tree;
	}

	// children
	for (auto it : tree->children) {
		tree_t* t = _getbyprop(it.second, key, val);
		if (t)
			return t;
	}
	return 0;
}
_API(kv_order)
{
	if (ME)
		ME->b_kv_ordered = true;
	return 0;
}
_API(api_im)
{
	ASSERT_RET(ROOT);
	ASSERT(args == 1);

	NODE* me = 0;
	crstr expr = GET_SPARAM(1);
	if (expr[0] == '.')
	{
		code ccd(expr.c_str()); ccd.next();
		string key = ccd.getname();
		PRINTV(key)
			ccd.next3();
		ASSERT(ccd.cur() == '=');
		ccd.next();
		string val = getstring(ccd);
		PRINTV(val);
		me = _getbyprop(ROOT, key, val.c_str());
		if (me)
			PRINTV(me->name);
	}
	else 
	{
		SPARAM(node);
		if (node == "parent")
		{
			if (me) me = me->parent;
		}
		else if (node == "child")
		{
			if (me && !me->children.empty())
				me = me->children.begin()->second;
		}
		else
		{
			me = get_node(node, ROOT);
		}
		if (me)
			PRINTV(me->name);
	}
	if (me)
		work_stack.push_back(me);
	return 0;
}
_API(api_bye)
{
	work_stack.clear();
	return 0;
}
_API(api_on)
{
	ASSERT_RET(ROOT);
	ASSERT(args == 1);

	SPARAM(on);
	cur_property = on;

	return 0;
}
_API(array)
{
	ASSERT(ME);
	tree_t* ntree = new tree_t;

	if (args == 1)
	{
		SPARAM(clonenode);
		tree_t* t = get_node(clonenode);
		if (t)
		{
			(*ntree) += (*t);
		}
	}
	if (ME->parent && (!cd.iter.empty() && cd.iter.back() > 1))
	{
		ntree->name = string("n") + tree_t::genid();// +"_" + to_string(ME->parent->children.size() + 1);
		ntree->index = ME->parent->children.size() + 1;
		ME->parent->children[ntree->name] = ntree;
		ntree->parent = ME->parent;
	}
	else
	{
		ntree->name = string("n") + tree_t::genid();// + "_" + to_string(ME->children.size() + 1);
		ntree->index = ME->children.size() + 1;
		ME->children[ntree->name] = ntree;
		ntree->parent = ME;
	}
	ME = ntree;

	return 0;
}
_API(sequ)
{
	ASSERT(ME);
	tree_t* ntree = new tree_t;
	ntree->index = ME->children.size() + 1;
	ntree->name = string("n") + tree_t::genid();// + "_" + to_string(ME->children.size() + 1);

	if (args == 1)
	{
		SPARAM(clonenode);
		tree_t* t = get_node(clonenode, ROOT);
		if (t)
		{
			PRINTV(clonenode);
			(*ntree) += (*t);
		}
	}
	{
		ME->children[ntree->name] = ntree;
		ntree->parent = ME;
	}
	work_stack.push_back(ntree);

	return 0;
}
void property(tree_t* tree, const string& key, const string& val, const string& filter = "")
{
	const char* p = 0;
	if (!filter.empty())
		p = filter.c_str();
	if (p == 0 ||
		(*p) != '!' && tree->name.find(filter) != std::string::npos ||
		(*p) == '!' && tree->name.find(p + 1) == std::string::npos)
		tree->kv[key] = val;

	// children
	for (auto& it : tree->children) {
		property(it.second, key, val, filter);
	}
}
_API(property)
{
	string& key = GET_SPARAM(1);
	string& val = GET_SPARAM(2);
	string filter = "";
	if (args >= 3)
		filter = GET_SPARAM(3);
	property(ROOT, key, val, filter);

	POP_SPARAM;
	return 0;
}
void dump(tree_t* tree, const string& pre = "")
{
	PRINT(pre << tree->name << "{")
	{
		for (auto& it : tree->kv)
		{
			PRINT(pre << "\t" << it.first << ":" << it.second);
		}
	}

	// children
	for (auto& it : tree->children) {
		dump(it.second, pre + "\t");
	}
	PRINT(pre << "}")
}
_API(dump_node)
{
	PRINT("-------- DUMP --------");
	NODE* n = ROOT;
	if (args >= 1)
	{
		SPARAM(a);
		n = a == "me" ? (ME) : GET_NODE(a, ROOT);
	}
		
	dump(n);
	return 0;
}
_API(do_expr)
{
	ASSERT_RET(args == 0);
	GROUP::node_walker(ROOT, [](GROUP::tree_t* tree)->void
		{
			for (auto& it : tree->kv) {
				const char* ps = it.second.c_str();
				while (checkspace(*ps)) ps++;
					
				if (*ps == '(') {
					string result;
					const char* start = ps;
					while (*ps != '\0') {
						string phg_expr = "";
						if ((*ps) == '(') {
							int bracket_d = 1;
							while (true) {
								char nc = *(++ps);

								if (nc == '(')
									bracket_d++;
								else if (nc == ')')
									bracket_d--;

								if (bracket_d == 0)
									break;

								phg_expr.push_back(nc);
							}
							{
								// 内部变量
								gvarmapstack.addvar("_i", tree->index);
								gvarmapstack.addvar("_t", tree_t::getdepth(tree));
							}
							//PRINTV(phg_expr);
							string str = phg_expr + ";";
							var v = GROUP::doexpr(str.c_str());

							result += v.tostr();
						}
						else
						{
							result += *ps;
						}
						++ps;
					}
					it.second = result;
					//PRINTV(result);
				}
			}
		});
	POP_SPARAM;

	return 0;
}
_API(walknode)
{
	ASSERT_RET(args >= 1);
	string script = GET_SPARAM(1);
	NODE* node = ROOT;
	if (args > 1)
	{
		string param1 = GET_SPARAM(2);
		node = GET_NODE(param1, ROOT);
		if (!node)
			return 0;
	}
	node_walker(node, [script](tree_t* tree)->void
		{
			work_stack.push_back(tree);
			gvarmapstack.addvar("_name", tree->name.c_str());
			gvarmapstack.addvar("_i", tree->index);
			gvarmapstack.addvar("_t", tree_t::getdepth(tree));
			dostring((script.back() != '}' ? script + ";" : script).c_str());
		});
	POP_SPARAM;

	return 0;
}
namespace node 
{
	struct res_t
	{
		// 携带的属性
		NODE* node;
		string key;

		res_t() {}
		res_t(const res_t& v)
		{
			node = v.node;
			key = v.key;
		};
		~res_t() {}
	};
	vector<res_t*> reslist;		// 资源列表

	res_t& cres(const var& ent)
	{
		ASSERT(ent.resid >= 0 && ent.resid < reslist.size());
		return *reslist[ent.resid];
	}
	res_t& res(var& ent)
	{
		if (ent.resid == -1)
		{
			res_t* rs = new res_t();
			reslist.push_back(rs);
			ent.resid = reslist.size() - 1;

			ent.type = 0; // 自定义元素类型
			// 在资源上定义加法运算
			ent.fun_set = [&ent](const var& v) {
				if (v.type == 0)
				{
					res(ent).node->kv[res(ent).key] = cres(v).node->kv[cres(v).key];
				}
				else {
					//ent.node->kv[ent.key] = v.tostr();
				}
			};

		}
		//PRINTV(ent.resid);
		ASSERT(ent.resid < reslist.size());
		return *reslist[ent.resid];
	}
}
// ===================================
// REG APIs
// ===================================
void NODE_REG_API()
{
	CALC([](code& cd, char o, int args)->var {
		if (o == '.')
		{
			ASSERT_RET(ROOT);
			crstr a = GET_SPARAM(1);
			//string b = GET_SPARAM(2);
			crstr b = PHG_PARAM(2).tostr();

			//PRINT("PROP GET: " << a << "." << b);
			NODE* n = a == "me" ? (ME) : GET_NODE(a, ROOT);
			ASSERT_RET(n);

			string c = n->kv[b];
			//PRINT(a << "." << b << "=" << c);
			var v; v.type = 0; v.sval = c; node::res(v).node = n; node::res(v).key = b;
			
			POP_SPARAM;
			cd.strstack.push_back(c);
			strlist.push_back(c);
			return v;
		}
		return 0;
		});
	PROP([](code& cd, const char* a, const char* b, var& v) {
		if (!ROOT) return;
		int args = 1;

		NODE* n = strcmp(a, "me") == 0 ? (ME) : GET_NODE(a, ROOT);
		ASSERT(n);
		//PRINTV(cd.strstack.size());
		string sv = GET_SPARAM(1);
		if (sv == "nil")
			n->kv.erase(b);
		else
		{
			n->kv[b] = sv;
			PHG_PRINT("PROP SET: " << a << "." << b << "=" << sv);
		}
		POP_SPARAM;
		});

	_REG_API(order, kv_order);		// kv_order
	_REG_API(im, api_im);			// ME
	_REG_API(bye, api_bye);			// ME = NULL(正在放弃中...)
	_REG_API(on, api_on);			// 当前属性
	_REG_API(array, array);			// 节点阵列 (正在放弃中...)
	_REG_API(sequ, sequ);			// 节点序列 (正在放弃中...)

	_REG_API(prop, property);		// 添加属性(正在放弃中...)
	_REG_API(wak, walknode);		// 遍历节点树
	_REG_API(expr, do_expr);		// 执行表达式
	_REG_API(dump, dump_node);		// dump
}