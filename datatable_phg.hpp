/***
*						数据表格
*				包括统计学误差分析等API函数
*
*/
namespace datatable_phg
{
	enum
	{
		PHGNODE = 10
	};
	std::map<string, string> nodemap;

	void setup(NODE* tree)
	{
		KV_IT;

		var v;
		KEY_VAL("node")
		{
			v.type = PHGNODE;
			v.sval = tree->name;
			ADD_VAR(tree->name.c_str(), v);
		}
		// children
		for (auto it : tree->children) {
			setup(it.second);
		}
	}
	// --------------------------------------
	_API(dt_sum)
	{
		var& a = GET_PARAM(1);
		{
			auto& it = realtab.row.find(a.sval);
			ASSERT(it != realtab.row.end());
			real sum = 0;
			for (auto& v : it->second.col)
			{
				sum += (real)v.second;
			}
			return var(sum);
		}
		return 0;
	}
	_API(dt_avg)
	{
		var& a = GET_PARAM(1);
		{
			auto& it = realtab.row.find(a.sval);
			ASSERT(it != realtab.row.end());
			real sum = 0;
			int i = 0;
			for (auto& v : it->second.col)
			{
				sum += (real)v.second;
				i++;
			}
			return var(sum / i);
		}
		return 0;
	}
	_API(dt_gm)
	{
		var& a = GET_PARAM(1);
		{
			auto& it = realtab.row.find(a.sval);
			ASSERT(it != realtab.row.end());
			real sum = 1;
			int i = 0;
			for (auto& v : it->second.col)
			{
				sum *= (real)v.second;
				i++;
			}
			return var(pow(sum,1.0f / i));
		}
		return 0;
	}
	_API(dt_mM)
	{
		var& a = GET_PARAM(1);
		{
			auto& it = realtab.row.find(a.sval);
			ASSERT(it != realtab.row.end());
			real M = -1e10, m = 1e10;
			int i = 0;
			for (auto& v : it->second.col)
			{
				if((real)v.second > M)
					M = (real)v.second;
				if ((real)v.second < m)
					m = (real)v.second;
			}
			return var(M - m);
		}
		return 0;
	}
	_API(dt_VR)
	{
		var& a = GET_PARAM(1);
		var& mu = GET_PARAM(2);
		{
			auto& it = realtab.row.find(a.sval);
			ASSERT(it != realtab.row.end());
			
			real sum = 1;
			int i = 0;
			for (auto& v : it->second.col)
			{
				sum += ((real)v.second - (real)mu) * ((real)v.second - (real)mu);
				i++;
			}
			return var(sum / i);
		}
		return 0;
	}
	_API(dt_SVR)
	{
		var& a = GET_PARAM(1);
		var& mu = GET_PARAM(2);
		{
			auto& it = realtab.row.find(a.sval);
			ASSERT(it != realtab.row.end());

			real sum = 1;
			int i = 0;
			for (auto& v : it->second.col)
			{
				sum += ((real)v.second - (real)mu) * ((real)v.second - (real)mu);
				i++;
			}
			return var(sqrt(sum / i));
		}
		return 0;
	}
	// --------------------------------------
	void DT_REG_API()
	{
		_REG_API(dt_sum, dt_sum);	// 总数
		_REG_API(dt_avg, dt_avg);	// 平均数
		_REG_API(dt_gm, dt_gm);		// 几何平均数(Geometric Mean)

		_REG_API(dt_mM, dt_mM);		// 极差：最大值与最小值之差
		_REG_API(dt_VR, dt_VR);		// 方差
		_REG_API(dt_SVR, dt_SVR);	// 标准差
	}
}