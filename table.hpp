/**************************************************************************
*						数值表的解析
*				数值在代数运算与现象间建立联系
**************************************************************************/
std::vector<var> gtable;	// 数值列表

inline int add2table(const var& v) {
	for (int i = 0; i < gtable.size(); i++) {
		if (gtable[i] == v) {
			//PRINT("add2table same! i=" << i);
			return i;
		}
	}
	gtable.push_back(v);
	return gtable.size() - 1;
}
struct tablerow_t
{
	string name;
	std::map<string, var> col;
};
struct table_t {
	string name;
	std::map<string, tablerow_t> row;
}realtab;
// 表数据解析
short table_get(code& cd)
{
	for (; !cd.eoc(); cd.next_()) {
		char c = cd.cur();
		if (checkspace2(c))
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
static inline bool checkspace3(char c) {
	return (c == ' ' || c == '\t' || c == '\r');
}
char table_next(code& cd)
{
	while (!cd.eoc(cd.ptr)) {
		if (!checkspace3(*(cd.ptr)) && !isname(*(cd.ptr)) && !isnum(*(cd.ptr)))
			break;
		cd.ptr++;
	}
	return (*cd.ptr);
}
char table_next_space(code& cd)
{
	while (!cd.eoc(cd.ptr)) {
		if (!isname(*(cd.ptr)) && !isnum(*(cd.ptr)))
			break;
		cd.ptr++;
	}
	return (*cd.ptr);
}
char table_next_namenum(code& cd)
{
	while (!cd.eoc(cd.ptr)) {
		if (!checkspace3(*(cd.ptr)))
			break;
		cd.ptr++;
	}
	return (*cd.ptr);
}

var table_chars2var(code& cd) {
	bool isreal = false;
	int i = 0;
	char buff[64];
	for (; i < 64; i++) {
		char c = cd.cur();
		if (c == '.')
			isreal = true;
		if (!isdigit(c) && c != '.')
			break;
		buff[i] = c;
		cd.next0();
	}
	buff[i] = '\0';
	cd.strstack.push_back(buff);
	return isreal ? var((real)atof(buff)) : var(atoi(buff));
}
static void _table(code& cd)
{
	PRINT("realtab[");
	cd.next();

	int rowcnt = 1, col = 1;
	std::vector<std::string> colnames;
	tablerow_t* currow = 0;
	while (!cd.eoc()) {
		short type = table_get(cd);
		//PRINT(rowcnt << ") " << (char)type << "=" << type);
		if (type == '\'' || type == '\"') {
			cd.next();
			getstring(cd);
		}
		else if (type == NAME) {
			const char* name = cd.getname();
			if (rowcnt == 1)
			{
				realtab.name = name;
				PRINT(realtab.name)
			}
			else if (rowcnt == 2)
			{
				PRINT("CREATE COL: " << name)
				colnames.push_back(name);
			}
			else
			{
				auto it = realtab.row.emplace(name, tablerow_t()).first;
				currow = &(it->second);
				currow->name = name;
				PRINT("CREATE ROW: " << name);
			}
			table_next_space(cd);
			table_next_namenum(cd);
		}
		else if (type == NUMBER) {
			ASSERT(currow != NULL);
			ASSERT(col <= colnames.size());
			var v = table_chars2var(cd);
			PRINT("ADD COL: " << col << "=" << v.ival);
			currow->col.emplace(colnames[col - 1], v);
			col++;
			table_next_space(cd);
			table_next_namenum(cd);
		}
		else if (type == '\n') {
			cd.next();
			col = 1;
			rowcnt++;
			PRINT("---" << rowcnt);
		}
		else if (type == ']') {
			cd.next();
			return;
		}
		else {
			table_next(cd);
		}
	}
}
// ===================================
// API
// ===================================
void tableindex_func(const char* pchar, var& v)
{
	string str(pchar); 
	PRINT(str);
	{
		vector<string> se;
		STR::split(str, se, " ");
		v = realtab.row[se.front()].col[se.back()];
	}
}
void TABLE_REG_API()
{
	tableindex = tableindex_func;
	CALC([](code& cd, char o, int args)->var {
		if (o == '.')
		{
			crstr a = GET_SPARAM(1);
			//string b = GET_SPARAM(2);
			crstr b = PHG_PARAM(2).tostr();
		
			var v;
			return v;
		}
		return 0;
		});

	//_REG_API(dumpt, dump_table);			// dump
}