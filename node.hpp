/**************************************************************************
*                      [Node Tree Parsing]
*              A node tree organizes individuals into a hierarchy
*          Includes important concepts like arrays, sequences, and selectors
*                  Any object can be defined as a node
* 
**************************************************************************/
#define	PHG_MAP			std::map
#define PHG_NODE		GROUP::phg_tree
#define PHG_ROOT		GROUP::root
#define ME				(GROUP::work_stack.empty() ? 0 : GROUP::work_stack.back())
#define GET_PHG_NODE(name, node)	name == "me" ? ME : get_node(name, node, true)
#define	MAX_ID		10000						// 最大ID

struct phg_tree;

int node_count = 0;							// 节点数量
phg_tree* root = 0;							// 暂时使用全局树
vector<phg_tree*>	work_stack;					// 工作栈
std::string			cur_property = "pr1";			// 当前属性

// -----------------------------------------------------------------------
#ifndef _PHG_STRUCT
extern void _crt_array(code& cd, phg_tree* tree, const string& pre, int depth, const string& selector);
extern void _crt_sequ(code& cd, phg_tree* tree, const string& pre);
extern phg_tree* get_node(const string& name, phg_tree* tree, bool recu);
extern phg_tree* get_node_upwards(const string& name, phg_tree* tree, bool recu = true);

#endif

// -----------------------------------------------------------------------
struct phg_tree
{
	phg_tree* parent = 0;
	string name;								// 名字
	int index = 0;								// 索引
	int user_id = 0;							// 用户设置的ID
	bool b_kv_ordered = false;						// kv表有序

	PHG_MAP<std::string, std::string> kv;					// 属性字典
	std::vector<std::string> kvorder;					// 属性字典的顺序

	PHG_MAP<std::string, phg_tree*> children;				// 子字典
	std::vector<phg_tree*>	childlist;					// 子列表(注意冗余!)，保留代码中的节点顺序

	static inline int getdepth(phg_tree* tree, int depth = 0)
	{
		if (tree->parent)
			return getdepth(tree->parent, depth + 1);
		else
			return depth;
	}
	static inline string genid()
	{
		node_count++;

		if (node_count >= MAX_ID) // 注意：id < MAX_ID
			return to_string(node_count);

		// 计算 MAX_ID 的位数
		const int num_digits = int(std::log10(MAX_ID));
		std::string str = std::to_string(node_count);
		str.insert(str.begin(), num_digits - str.length(), '0');

		return str;
	}
	inline int getindex()
	{
		return index;
	}
	void operator += (const phg_tree& t)
	{
		for (auto it : t.kv)
		{
			kv[it.first] = it.second;
		}
		for (auto& it : t.children)
		{
			phg_tree* ntree = new phg_tree();
			ntree->index = children.size() + 1;
			ntree->name = "n" + phg_tree::genid(); // +"_" + to_string(ntree->index);

			children[ntree->name] = ntree;
			childlist.push_back(ntree);	// 记录在列表里
			ntree->parent = this;

			*ntree += *it.second;
		}
	}
	void copyprop(phg_tree* t)
	{
		for (const auto& it : kv)
		{
			t->kv[it.first] = it.second;
		}
	}
	static void clear(phg_tree* ot)
	{
		if (!ot) return;

		for (auto it : ot->children)
		{
			clear(it.second);
		}
		delete(ot);
	}
};

// ------------------------------------
static void _tree(code& cd, phg_tree* tree, const string& pre, int depth = 0)
{
	work_stack.push_back(tree);
	PHG_DEBUG_PRINT(pre << "{");
	cd.next();

	std::string key, val;
	std::string* pstr = &key;
	string selector;

	while (!cd.eoc()) {
		char c = cd.cur();
		// 注解
		if (c == '#') {
			cd.nextline();

			c = cd.cur();
			if (!checkspace(c))
			{
				continue;
			}
			cd.next_();
		}
		else if (c == '/') {
			if (*(cd.ptr + 1) == '*')
			{
				while ((*cd.ptr) != '\0' &&
					((*(cd.ptr)) != '*' || (*(cd.ptr + 1)) != '/'))
					cd.ptr++;
				continue;
			}
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
					key = ("n") + phg_tree::genid();// +"_" + to_string(tree->children.size() + 1);
				}
				phg_tree* ntree = new phg_tree;
				ntree->index = tree->children.size() + 1;
				ntree->name = key;
				tree->children[key] = ntree;
				tree->childlist.push_back(ntree);// 记录在列表里
				ntree->parent = tree;

				PHG_DEBUG_PRINT(pre << key << " : ");
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
					phg_tree* t = get_node_upwards(key, tree); //GET_PHG_NODE(key, tree); // local first
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
					PHG_DEBUG_PRINT(pre << key << " : " << val);
				}
				val = "";
				key = "";
				pstr = &key;
			}
			cd.next();
			if (c == '}' || c == '>') 
				return;
		}

		// 逗号间隔,不能作为property的结尾！
		else if (c == ',' && pstr != &val) {
			if (!key.empty())
			{// inhert
				phg_tree* t = get_node_upwards(key, tree); //GET_PHG_NODE(key, tree); // local first
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
			cd.next_();

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
			cd.next_();
		}
	}
}
phg_tree* get_node(const string& name, phg_tree* tree = PHG_ROOT, bool recu = true)
{
	ASSERT(!name.empty());
	ASSERT(tree);
	if (name == tree->name)
		return tree;
	{
		auto it = tree->children.find(name);
		if (it != tree->children.end())
			return it->second;
	}
	if (recu)
	{
		for (auto it : tree->children) {
			phg_tree* ret = get_node(name, it.second, recu);
			if (ret)
				return ret;
		}
	}
	return 0;
}
phg_tree* get_node_upwards(const string& name, phg_tree* tree, bool recu)
{
	ASSERT(!name.empty());
	ASSERT(tree);

	if (name == tree->name)
		return tree;

	auto parent = tree->parent;
	if (parent) {
		for (auto& sibling : parent->children) {
			if (sibling.second != tree) {
				if (sibling.second->name == name)
					return sibling.second;
				{
					auto* node = get_node(name, sibling.second);
					if (node)
						return node;
				}
			}
		}
		if (recu) {
			phg_tree* ret = get_node_upwards(name, parent, recu);
			if (ret)
				return ret;
		}
	}
	return 0;
}
phg_tree* get_node_in_childlist(const string& name, phg_tree* parent)
{
	if (parent) {
		for (auto& node : parent->childlist)
			if (node->name == name)
				return node;
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

// ------------------------------------
// 阵列
// ------------------------------------
void _crt_array(code& cd, phg_tree* tree, const string& pre, int depth, const string& selector)
{
	PHG_DEBUG_PRINT("crt_array")

	cd.next();
	int index = 0;
	int rnd = rand();
	string node;
	while (!cd.eoc()) 
	{
		char c = cd.cur();
		//PHG_DEBUG_PRINT(c);
		phg_tree* ntree = 0;
		if (c == '#') {
			cd.nextline();
			cd.next();
			continue;
		}
		else if (c == '/') {
			if (*(cd.ptr + 1) == '*')
			{
				while ((*cd.ptr) != '\0' &&
					((*(cd.ptr)) != '*' || (*(cd.ptr + 1)) != '/'))
					cd.ptr++;
				continue;
			}
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
			ntree = new phg_tree;
			ntree->index = tree->children.size() + 1;
			if (node.empty())
			{
				node = ("n") + phg_tree::genid();// + "_" + to_string(ntree->index);
			}
			ntree->name = node;

			tree->children[ntree->name] = ntree;
			tree->childlist.push_back(ntree);// 记录在列表里

			ntree->parent = tree;

			_tree(cd, ntree, pre, depth + 1);
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
				tree->childlist.pop_back();
				ntree->parent = 0;
				phg_tree::clear(ntree);
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

			if (!ntree)
			{// create node
				work_stack.push_back(tree);
				ntree = new phg_tree;
				ntree->index = tree->children.size() + 1;
				ntree->name = ("n") + phg_tree::genid();// + "_" + to_string(ntree->index);
				{// inhert
					phg_tree* t = get_node_upwards(node, tree); // GET_PHG_NODE(node, PHG_ROOT);
					if (t) {
						(*ntree) += (*t);
					}
				}
				int ret = select(index, rnd, selector);
				if (ret) {
					tree->children[ntree->name] = ntree;
					tree->childlist.push_back(ntree);// 记录在列表里
					ntree->parent = tree;
					if (ret == 1)
					{
						cd.next();
						break;
					}
				}
				else
				{
					phg_tree::clear(ntree);
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

// ------------------------------------
// 序列
// ------------------------------------
void _crt_sequ(code& cd, phg_tree* tree, const string& pre)
{
	PHG_DEBUG_PRINT("crt_sequ")

	cd.next();

	string node;
	while (!cd.eoc())
	{
		char c = cd.cur();
		phg_tree* ntree = 0;
		if (c == '#') {
			cd.nextline();
			cd.next();
			continue;
		}
		else if (c == '/') {
			if (*(cd.ptr + 1) == '*')
			{
				while ((*cd.ptr) != '\0' &&
					((*(cd.ptr)) != '*' || (*(cd.ptr + 1)) != '/'))
					cd.ptr++;
				continue;
			}
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
			ntree = new phg_tree;
			ntree->index = tree->children.size() + 1;
			if (node.empty())
			{
				node = ("n") + phg_tree::genid();// +"_" + to_string(ntree->index);
			}
			ntree->name = node;
			tree->children[ntree->name] = ntree;
			tree->childlist.push_back(ntree);// 记录在列表里
			ntree->parent = tree;

			PHG_DEBUG_PRINT(pre << ntree->name << " : ");

			_tree(cd, ntree, pre, phg_tree::getdepth(tree) + 1);
			tree = ntree;
			PHG_DEBUG_PRINT("cur: " << cd.cur())
			node = "";
		}
		else if (c == ',' || c == '>'){}
		else
			node += c;

		c = cd.cur();
		if (c == ',' || c == '>')
		{
			if (!ntree && !node.empty())
			{// create node
				work_stack.push_back(tree);
				ntree = new phg_tree;
				ntree->index = tree->children.size() + 1;
				ntree->name = ("n") + phg_tree::genid();// +"_" + to_string(tree->children.size() + 1);
				{// inhert
					phg_tree* t = get_node_upwards(node, tree); //GET_PHG_NODE(node, PHG_ROOT);
					if (t) {
						(*ntree) += (*t);
					}
				}
				tree->children[ntree->name] = ntree;
				tree->childlist.push_back(ntree);// 记录在列表里
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

// ------------------------------------
// 树节点解析
// ------------------------------------
void _tree(code& cd)
{
	phg_tree::clear(root);

	node_count = 0;
	root = new phg_tree;
	root->name = "root";
	_tree(cd, root, "", 0);
	
	//phg_tree::clear(tree);
}

// ------------------------------------
// 寻源式加法
// 两个节点之和为最近的相同祖先
// ------------------------------------
bool porperty_intree(phg_tree* tree, const char* key, crstr val)
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

// ------------------------------------
// 在节点树上搜索加法规则
// ------------------------------------
const char* walk_addtree(phg_tree* tree, crstr v_a, crstr v_b, const char* key)
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
void phg_node_walk(phg_tree* tree, std::function<void(phg_tree*)> fun)
{
	fun(tree);

	// children
	for (auto& it : tree->children) {
		phg_node_walk(it.second, fun);
	}
}

// ====================================
// API
// ====================================
phg_tree* _getbyprop(phg_tree* tree, crstr key, crstr val)
{
	if (tree->kv[key] == val){
		return tree;
	}

	// children
	for (auto it : tree->children) {
		phg_tree* t = _getbyprop(it.second, key, val);
		if (t)
			return t;
	}
	return 0;
}
_API(kv_order)
{
	UNUSED(args); UNUSED(cd);
	if (ME)
		ME->b_kv_ordered = true;
	return 0;
}
_API(api_im)
{
	ASSERT_RET(PHG_ROOT);
	ASSERT(args == 1);

	PHG_NODE* me = 0;
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
		me = _getbyprop(PHG_ROOT, key, val.c_str());
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
			me = get_node(node, PHG_ROOT);
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
	UNUSED(args); UNUSED(cd);
	work_stack.clear();
	return 0;
}
_API(api_on)
{
	ASSERT_RET(PHG_ROOT);
	ASSERT(args == 1);

	SPARAM(on);
	cur_property = on;

	return 0;
}
_API(array)
{
	ASSERT(ME);
	phg_tree* ntree = new phg_tree;

	if (args == 1)
	{
		SPARAM(clonenode);
		phg_tree* t = get_node(clonenode);
		if (t)
		{
			(*ntree) += (*t);
		}
	}
	if (ME->parent && (!cd.iter.empty() && cd.iter.back() > 1))
	{
		ntree->name = ("n") + phg_tree::genid();// +"_" + to_string(ME->parent->children.size() + 1);
		ntree->index = ME->parent->children.size() + 1;
		ME->parent->children[ntree->name] = ntree;
		ME->parent->childlist.push_back(ntree);// 记录在列表里
		ntree->parent = ME->parent;
	}
	else
	{
		ntree->name = ("n") + phg_tree::genid();// + "_" + to_string(ME->children.size() + 1);
		ntree->index = ME->children.size() + 1;
		ME->children[ntree->name] = ntree;
		ME->childlist.push_back(ntree);// 记录在列表里
		ntree->parent = ME;
	}

	GROUP::work_stack.empty() ? 0 : GROUP::work_stack.back() = ntree;

	return 0;
}
_API(sequ)
{
	ASSERT(ME);
	phg_tree* ntree = new phg_tree;
	ntree->index = ME->children.size() + 1;
	ntree->name = ("n") + phg_tree::genid();// + "_" + to_string(ME->children.size() + 1);

	if (args == 1)
	{
		SPARAM(clonenode);
		phg_tree* t = get_node(clonenode, PHG_ROOT);
		if (t)
		{
			PRINTV(clonenode);
			(*ntree) += (*t);
		}
	}
	{
		ME->children[ntree->name] = ntree;
		ME->childlist.push_back(ntree);// 记录在列表里
		ntree->parent = ME;
	}
	work_stack.push_back(ntree);

	return 0;
}
void property(phg_tree* tree, const string& key, const string& val, const string& filter = "")
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
	property(PHG_ROOT, key, val, filter);

	POP_SPARAM;
	return 0;
}
void dump(phg_tree* tree, const string& pre = "")
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
	PHG_NODE* n = PHG_ROOT;
	if (args >= 1)
	{
		SPARAM(a);
		n = a == "me" ? (ME) : GET_PHG_NODE(a, PHG_ROOT);
	}
	ASSERT(n);
	dump(n);
	return 0;
}
_API(do_expr)
{
	ASSERT_RET(args == 0);
	GROUP::phg_node_walk(PHG_ROOT, [](GROUP::phg_tree* tree)->void
		{
			for (auto& it : tree->kv) {
				const char* ps = it.second.c_str();
				while (checkspace(*ps)) ps++;
					
				if (*ps == '(') {
					string result;
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
								gvarmapstack.addvar("_t", phg_tree::getdepth(tree));
							}
							//PRINTV(phg_expr);
							string str = phg_expr + ";";
							phg_var v = GROUP::doexpr(str.c_str());

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
	PHG_NODE* node = PHG_ROOT;
	if (args > 1)
	{
		string param1 = GET_SPARAM(2);
		node = GET_PHG_NODE(param1, PHG_ROOT);
		if (!node)
			return 0;
	}
	phg_node_walk(node, [script](phg_tree* tree)->void
		{
			work_stack.push_back(tree);
			gvarmapstack.addvar("_name", tree->name.c_str());
			gvarmapstack.addvar("_i", tree->index);
			gvarmapstack.addvar("_t", phg_tree::getdepth(tree));
			dostring((script.back() != '}' ? script + ";" : script).c_str());
		});
	POP_SPARAM;

	return 0;
}

// ------------------------------------
// PHG Node Resource
// ------------------------------------
namespace phg_node 
{
	struct res_t
	{
		// 携带的属性
		PHG_NODE* node;
		string key;

		res_t() {}
		res_t(const res_t& v)
		{
			node = v.node;
			key = v.key;
		};
	};
	vector<res_t*> reslist;		// 资源列表
	res_t& cres(const phg_var& ent)
	{
		ASSERT(ent.resid >= 0 && ent.resid < reslist.size());
		return *reslist[ent.resid];
	}
	res_t& res(phg_var& ent)
	{
		if (ent.resid == -1)
		{
			res_t* rs = new res_t();
			reslist.push_back(rs);
			ent.resid = reslist.size() - 1;
			ent.type = 0; // 自定义元素类型

			// 在资源上定义加法运算
			ent.fun_set = [&ent](const phg_var& v) {
				if (v.type == 0)
				{
					res(ent).node->kv[res(ent).key] = cres(v).node->kv[cres(v).key];
				}
			};
		}
		ASSERT(ent.resid < reslist.size());
		return *reslist[ent.resid];
	}
}

// ===================================
// REG APIs
// ===================================
void NODE_REG_API()
{
	CALC([](code& cd, char o, int args)->phg_var {
		if (o == '.')
		{
			ASSERT_RET(PHG_ROOT);
			crstr a = GET_SPARAM(1);
			crstr b = PHG_PARAM(2).tostr();

			//PRINT("PROP GET: " << a << "." << b);
			PHG_NODE* n = a == "me" ? (ME) : GET_PHG_NODE(a, PHG_ROOT);
			ASSERT_RET(n);

			string c = n->kv[b];
			//PRINT(a << "." << b << "=" << c);
			phg_var v; v.type = 0; v.sval = c; phg_node::res(v).node = n; phg_node::res(v).key = b;
			
			POP_SPARAM;
			cd.strstack.push_back(c);
			strlist.push_back(c);
			return v;
		}
		return 0;
		});
	PROP([](code& cd, const char* a, const char* b, phg_var& v) {
		UNUSED(v);
		if (!PHG_ROOT) return;
		int args = 1;

		PHG_NODE* n = (strcmp(a, "me") == 0) ? (ME) : GET_PHG_NODE(a, PHG_ROOT);
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
