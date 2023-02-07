/***************************************************************************
								{PHG}
						包含元素与节点树二元
					本文件是一个框架，内部引用了不同模块的技术
***************************************************************************/
#ifdef GROUP
#undef GROUP
#endif
#define GROUP				ScePHG	// 组定义

// API APP 
#define API(fun)				static var fun(GROUP::valstack_t& valstack, int args)
#define REG_API(funname, fun)	GROUP::custom_functions[#funname] = fun;
#define GET_SPARAM_C(index)		valstack.get(args - 1 - index)

// API INNER
#define _API(funname)			static var api_##funname(GROUP::code& cd, int args)
#define PHG_API(funname)		_API(funname)
#define _REG_API(funname,cppfunname)	GROUP::register_api(#funname, api_##cppfunname)
#define PHG_REG_API(funname,cppfunname)	GROUP::register_api(funname, api_##cppfunname)
#define REG_LUAAPI				lua_register

// PARAM
#define PARAM(name)				var name = cd.valstack.back();
#define GET_PARAM(index)		cd.valstack.get(args - (index))
#define POP_PARAM				cd.valstack.pop_back();
#define POP_PARAMN(n)			for(int i = 0; i < n; i ++) cd.valstack.pop_back();

#define SPARAM(name)			string name = cd.strstack.back(); cd.strstack.pop_back();
#define GET_SPARAM(index)		cd.strstack[cd.strstack.size() - 1 - (args - (index))]
#define POP_SPARAM				for(int i = 0; i < args; i ++) cd.strstack.pop_back();
#define POP_SPARAMN(n)			for(int i = 0; i < n; i ++) cd.strstack.pop_back();

// VAR
#define GETVAR(name)			gvarmapstack.getvar(name)
#define GETVAR2(out, name)		gvarmapstack.getvar(out, name)
#define ADDVAR(name,ent)		gvarmapstack.addvar(name,ent)

// CALC
#define CALC					GROUP::fun_calc = 
#define PROP					GROUP::add_var2 = 

#ifdef WIN
	#define CHEKCK_KEY(k)		if(tree->kv.find(k)!= tree->kv.end())
	#define KEY_VAL(val)		if (auto it = tree->kv.find(val); it != tree->kv.end())
	#define KEY					it->first
	#define VALUE				it->second
	#define KV_IT					
#else
	#define CHEKCK_KEY(k)		tree->kv.find(val)
	#define KEY_VAL(val)		it = tree->kv.find(val); if (it != tree->kv.end())
	#define KEY					it->first
	#define VALUE				it->second
	#define KV_IT				std::map<std::string, std::string>::iterator it;
#endif

#include "str.hpp"

//#define XML
#ifdef XML
#include "tinyxml.h"
#endif

//#define PYTHON
#ifdef PYTHON
#undef real
#include "python.hpp"
#define real	float
#endif

// data convert
inline int stoint(crstr sval)
{
	return atoi(sval.c_str());
}
inline real storeal(crstr sval)
{
	return atof(sval.c_str());
}
inline vec3 stovec(crstr sval)
{
	vec3 ret;
	sscanf(sval.c_str(), "%f,%f,%f", &ret.x, &ret.y, &ret.z);
	return ret;
}
inline vec2 stovec2(crstr sval)
{
	vec2 ret;
	sscanf(sval.c_str(), "%f,%f", &ret.x, &ret.y);
	return ret;
}
inline quaternion stoq(crstr sval)
{
	quaternion ret;
	sscanf(sval.c_str(), "%f,%f,%f,%f", &ret.x, &ret.y, &ret.z, &ret.w);
	return ret;
}
inline string v3tos(crvec v)
{
	stringstream ss;
	ss << v.x << "," << v.y << "," << v.z;
	return ss.str();
}

namespace ScePHG
{
	std::vector<string> strlist; // string values
	// ---------------------------------------------------------------------
	#include "phg_head.hpp"
	#include "element.hpp"
	#include "table.hpp"
	#include "node.hpp"
	// ---------------------------------------------------------------------
	using customfun_t = std::function<var(valstack_t&, int)>;
	std::map<string, customfun_t> custom_functions; // custom functions

	// ---------------------------------------------------------------------
	// 注册PHG的应用
	// ---------------------------------------------------------------------
	#ifdef OCC
	#include "occ_ent.hpp"
	#endif
	#include "polyent.hpp"
	#include "entity3d.hpp"
	#include "sprite.hpp"
	#include "nodecalc.hpp"
	#include "datatable_phg.hpp"
	#include "measure.hpp"
	//#include "bp_projector_app.hpp"
	#include "pipeent.hpp"
	#include "codegenerator.hpp"
	#include "constraint_ent.hpp"
	#include "constraint_algebra.hpp"

	// ---------------------------------------------------------------------
	void clear()
	{
		PRINT("clear all")

		poly::clearres();
		entity::clearres();
		sprite::clearres();
		
		tree_t::clear(ROOT);
		ROOT = 0;
		work_stack.clear();
		strlist.clear();
		gvarmapstack.clear();
	}
	// =================================================================
	// API
	// =================================================================
	_API(echo)
	{
		SPARAM(param1);
		becho = atoi(param1.c_str());
		return 0;
	}
	_API(rendermod)
	{
		SPARAM(param1);
		renderstate = atoi(param1.c_str());
		return 0;
	}
	_API(nform)
	{
		PRINT("nform");
		//tricombine::test();

		return 0;
	}
	_API(rnd)
	{
		int param1 = GET_PARAM(1).type == 1 ? GET_PARAM(1).ival + 1 : GET_PARAM(1).fval + 1;
		POP_PARAM;
		return var(rand() % param1);
	}
	_API(phg_cos)
	{
		real ang = GET_PARAM(1).fval * PI / 180.0f;
		POP_PARAM;
		return var(cos(ang));
	}
	_API(phg_sin)
	{
		real ang = GET_PARAM(1).fval * PI / 180.0f;
		POP_PARAM;
		return var(sin(ang));
	}
	_API(phg_pow)
	{
		real param1 = GET_PARAM(1).fval;
		real param2 = GET_PARAM(2).fval;
		POP_PARAM;
		return var(pow(param1, param2));
	}
	_API(phg_log)
	{
		real param1 = GET_PARAM(1).fval;
		return var(log(param1));
	}
	_API(phg_exp)
	{
		real param1 = GET_PARAM(1).fval;
		return var(exp(param1));
	}
	_API(scat)
	{
		ASSERT(args == 2)
		string param1 = GET_SPARAM(1);
		string param2 = GET_SPARAM(2);
		POP_SPARAM;
		cd.strstack.push_back(param1 + param2);
		return 0;
	}
	_API(scmp)
	{
		ASSERT(args == 2)
		string param1 = GET_SPARAM(1);
		string param2 = GET_SPARAM(2);
		POP_SPARAM; return int(param1 == param2);
	}
	_API(tos)
	{
		ASSERT(args == 1)
		PARAM(v);
		cd.strstack.push_back(v.type == 1 ? to_string(v.ival) : to_string(v.fval));
		return 0;
	}
	_API(strout)
	{
		ASSERT(args == 1)
		string param1 = GET_SPARAM(1);
		//PRINT(param1);
		
		POP_SPARAM; return 0;
	}
	// setuptree
	_API(setuptree)
	{
		string type; // 节点可以实现化为不同的实体类型
		if (args > 0)
		{
			type = GET_SPARAM(1);
			PRINTV(type);
		}
		NODE* node = ROOT;
		if (args > 1)
		{
			string nodename = GET_SPARAM(2);
			node = GET_NODE(nodename, ROOT);
			PRINTV(nodename);
		}
		if (type == "spr")
			sprite::setup(node, { vec2::ZERO, 0, vec2::ONE });
		else if (type == "ent")
			entity::setup(node, { vec3::ZERO, quaternion(), vec3::ZERO, vec3::ONE });
		else if (type == "poly")
			poly::setup(node, { vec3::ZERO, quaternion(), vec3::ONE });
		else if (type == "prj")
			PROJ_REG_API();
		else if (type == "cad")
			measurement::setup(node, coord3());
		else if (type == "pipe")
		{
			pipe::setup(node, { vec3::ZERO, quaternion(), vec3::ONE });
			PIPE_REG_API();
		}
		else if (type == "code")
		{
			codegenerator::CORD_REG_API(node);
		}
		else if (type == "cst0")
		{
			constraint_ent::setup(node);
			constraint_ent::CONSTRAINT_REG_API();
		}
		else if (type == "cst")
		{
			if(node)
				constraint_algebra::setup(node);
			constraint_algebra::REG_APIs();
		}
		else if (type == "dat")
		{
			datatable_phg::setup(node);
		}
#ifdef OCC
		else 
		if (type == "occ")
		{
			coord3 coord0;
			occent::setup(node, coord0);

			OCC_REG_API();
		}
#endif
		POP_SPARAM;
		return 0;
	}
	
#ifdef XML
// ----------------------------------------
#include "xmlparser.hpp"
// ----------------------------------------
	_API(fromXML)
	{
		ASSERT(!work_stack.empty());

		SPARAM(filename);

		fromXML(filename, ME);

		return 0;
	}
#endif

// ----------------------------------------
#include "jsonparser.hpp"
// ----------------------------------------
	_API(tojson)
	{
		PRINT("------------- tojson ----------------");
		NODE* node = ROOT;
		if (args > 0)
		{
			string nodename = GET_SPARAM(1);
			node = GET_NODE(nodename, ROOT);
		}
		if (node) JSON_PARSER::tojson(node);
		return 0;
	}
	_API(tojson2d)
	{
		//PRINT("------------- tojson ----------------");
		NODE* node = ROOT;
		if (args > 0)
		{
			string nodename = GET_SPARAM(1);
			node = GET_NODE(nodename, ROOT);
		}
		if (node) JSON_PARSER::tojson2d(node);
		return 0;
	}
	_API(tojsonraw)
	{
		PRINT("------------- tojsonraw ----------------");
		if (ROOT)
			JSON_PARSER::tojson_raw(ROOT);
		return 0;
	}
	_API(fromjson)
	{
		NODE* node = ROOT;
		{
			string nodename = GET_SPARAM(1);
			node = GET_NODE(nodename, ROOT);
		}
		string json = GET_SPARAM(2);
		if (node)
			JSON_PARSER::fromJSON(node, json);
		return 0;
	}
#ifdef PYTHON
	_API(formImgIDF)
	{
		SPARAM(img);

		ASSERT(ME);

		string json;
		python_interface::detect_rectangle(json, img);

		JSON_PARSER::fromJSON(ME,json);

		return 0;
	}
#endif
	// ---------------------------
	// pdms
	#include "pdms.hpp"
	// ---------------------------

	_API(msgbox)
	{
		var a = GET_PARAM(1);
		MSGBOX(a.tostr());
		return 0;
	}

	std::map<std::string, std::function<void()> > phg_hdrmap;
	_API(api)
	{
		SPARAM(funname);
		for (auto& it : phg_hdrmap)
		{
			if (funname == it.first)
			{
				it.second();
			}
		}
		return 0;
	}
	_API(lua)
	{
		PARAM(s);
		PRINT(s.sval);
		dolua(s.sval,true);
		return 0;
	}
	_API(onemap)
	{
		SPARAM(s);
		PRINT(s);
		ONEMAP::main(s);
		return 0;
	}

	// -----------------------------------------------------
	// CUSTOM FUNCITONS
	// -----------------------------------------------------
	int getintvar(crstr varnm)
	{
		return GET_VAR(varnm.c_str()).ival;
	}
	real getrealvar(crstr varnm)
	{
		return GET_VAR(varnm.c_str()).fval;
	}
	string getstrvar(crstr varnm)
	{
		return GET_VAR(varnm.c_str()).sval;
	}
	void setintvar(crstr varnm, int val)
	{
		ADD_VAR(varnm.c_str(), val);
	}
	void setrealvar(crstr varnm, real val)
	{
		ADD_VAR(varnm.c_str(), val);
	}
	void setstrvar(crstr varnm, crstr val)
	{
		ADD_VAR(varnm.c_str(), val.c_str());
	}
	_API(custom_api)
	{
		crstr funname = GET_PARAM(1).sval;
		PRINT("custom_api: " << funname)
		var v = custom_functions[funname](cd.valstack, args - 1);
		//POP_SPARAMN(args);
		//POP_PARAMN(args);
		return 0;
	}
	_API(phg_dump)
	{
		dump_strstack(cd);
		return 0;
	}
	// -----------------------------------
	// LUA API
	// -----------------------------------
	static int setvar(lua_State* L)
	{
		std::string key = lua_tostring(L, 1);
		real val = (float)lua_tonumber(L, 2);
		PRINT("var " << key << "=" << val);
		ADD_VAR(key.c_str(), val);
		return 0;
	}
	
	// -----------------------------------
	// REG API
	// -----------------------------------
	void setup()
	{
		//ASSERT(!tree);
		//ASSERT(!table);

		PRINT("setup {PHG}");

		tree = _tree;
		table = _table;
		act = _act;

		_REG_API(echo, echo);			// 打印开关
		_REG_API(mod, rendermod);		// 渲染模式
		_REG_API(msg, msgbox);			// msgbox
		_REG_API(msgbox, msgbox);		

		_REG_API(rnd, rnd);				// 随机函数
		_REG_API(cos, phg_cos);			// cos函数
		_REG_API(sin, phg_sin);			// sin函数
		_REG_API(pow, phg_pow);			// pow函数
		_REG_API(log, phg_log);			// log函数
		_REG_API(exp, phg_exp);			// exp函数

		_REG_API(tos, tos);				// 转化字符串

		_REG_API(setup, setuptree);		// 实例化安装

#ifdef XML
		_REG_API(fromXML, fromXML);		// xml读入
#endif
#ifdef PYTHON
		_REG_API(formImgIDF, formImgIDF);// 图像识别读入
#endif
		_REG_API(tojson, tojson);		// json
		_REG_API(tojsonraw, tojsonraw);	
		_REG_API(tojson2, tojson2d);
		_REG_API(fromjson, fromjson);
		
		// ELEMENT API 

		_REG_API(api, api);				// api
		_REG_API(lua, lua);				// lua
		_REG_API(map, onemap);			// onemap

		NODE_REG_API();					// NODE
		NODECALC_REG_API();				// node calc

		TABLE_REG_API();				// TABLE

		datatable_phg::DT_REG_API();

		ENTITY_REG_API();				// ENTITY 
		SPRITE_REG_API();				// sprite 注册
		POLY_REG_API();					// polyent
		
		PDMS_REG_API();					// PDMS

		CARD_REG_API();					// 计算卡

		// custom
		_REG_API(api, custom_api);

		// dump
		_REG_API(dump, phg_dump);

		// lua
		REG_LUAAPI(L, "var", setvar);
	}
};

// ====================================
// test
// ====================================
#include "cmdmonitor.hpp"
void do_cmdstring()
{
	string cmdstr;
	if (!CMD::readfile(cmdstr, "main.r"))
		return;

	//PRINTV(cmdstr);
	while (cmdstr.back() == '\n' || cmdstr.back() == '\r' || cmdstr.back() == ' ')
		cmdstr.pop_back();
	if (cmdstr.back() != ';' && cmdstr.back() != '}' && cmdstr.back() != ']')
	{// CMD
		PRINT("do_cmdstring CMD")
		CMD::dostring(cmdstr);
	}
	else 
	{// PHG
		ScePHG::dostring(cmdstr.c_str());
	}
}
#ifdef WIN
//------------------------------------------
// VB
//------------------------------------------
extern "C"
{
	int EXPORT_API doPHG(char* script)
	{
		estack.clear();
		init();

		std::string str(script);

		if (str.find(".phg") != std::string::npos || 
			str.find(".r") != std::string::npos || 
			str.find(".txt") != std::string::npos)
		{
			ScePHG::clear();
			ScePHG::setup();
			renderstate = 0;
			resetsm();

			ScePHG::dofile(str.c_str());
		}
		else
		{
			ScePHG::dostring(str.c_str());
		}
		return 0;
	}
	int EXPORT_API _stdcall VB_doPHG(const char* script)
	{
		estack.clear();
		init();

		std::string str(script);
		//PRINTV(str);

		if (str.find(".r") != std::string::npos || str.find(".txt") != std::string::npos || str.find(".phg") != std::string::npos)
		{
			ScePHG::clear();
			ScePHG::setup();
			renderstate = 0;
			resetsm();

			ScePHG::dofile(str.c_str());
		}
		else
		{
			if (!ROOT) {
				ScePHG::setup();
			}
			ScePHG::dostring(str.c_str());
		}
		//MSGBOX("VB_doPHG Done!");
		return 0;
	}
	int EXPORT_API _stdcall getIntVar(const char* key)
	{
		return GET_VAR(key).ival;
	}
	float EXPORT_API _stdcall getRealVar(const char* key)
	{
		return GET_VAR(key).fval;
	}
	int EXPORT_API _stdcall getLuaInt(const char* key)
	{
		return Global::intmap[key];
	}
	float EXPORT_API _stdcall getLuaReal(const char* key)
	{
		return Global::floatmap[key];
	}
	int EXPORT_API _stdcall getStringSize()
	{
		return ScePHG::strlist.size();
	}
	int EXPORT_API _stdcall getStringDat(int index, char* buf)
	{
		if (ScePHG::strlist.empty() || index >= ScePHG::strlist.size())
		{
			//buf[0] = '\0';
			return 0;
		}

		const string& str = ScePHG::strlist[index % ScePHG::strlist.size()];
		if (str.length() > 16)
			return 0;
		memcpy(buf, str.c_str(), str.length()+1);
		return  str.length();
	}
	int EXPORT_API _stdcall getMapWidth()
	{
		return ONEMAP::onemap.head.w;
	}
	int EXPORT_API _stdcall getMapHeight()
	{
		return ONEMAP::onemap.head.h;
	}
	int EXPORT_API _stdcall getMapChannel(char* buf, int channel)
	{
		ONEMAP::body_t& body = ONEMAP::onemap.body;
		ONEMAP::head_t& head = ONEMAP::onemap.head;
		for(int i = 0; i < head.w; i ++)
			for (int j = 0; j < head.h; j++)
			{
				buf[i + j * head.w] = body.buffer[i + j * head.w].dat[channel];
			}
		return 0;
	}
}
#else
extern "C"
{
	int EXPORT_API doPHG(char* script)
	{
		estack.clear();
		init();

		std::string str(script);

		if (str.find(".phg") != std::string::npos || str.find(".r") != std::string::npos || str.find(".txt") != std::string::npos)
		{
			ScePHG::clear();
			ScePHG::setup();
			renderstate = 0;
			resetsm();

			ScePHG::dofile(str.c_str());
		}
		else
		{
			ScePHG::dostring(str.c_str());
		}
		return 0;
	}
}
#endif