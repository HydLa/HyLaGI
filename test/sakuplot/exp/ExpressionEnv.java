package test.sakuplot.exp;

import java.util.HashMap;
import java.util.Map;

/**
 * 変数と関数の定義を含む式環境です。
 */
public class ExpressionEnv
{
	// 関数テーブル (name:String -> impl:UnaryFunctor)
	private Map<String, UnaryFunctor> _ftbl = new HashMap<String, UnaryFunctor>();
	
	// 変数テーブル (name:String -> value:double)
	private Map<String, Double> _vtbl = new HashMap<String, Double>();
	
	public void clearFuncs()
	{
		_ftbl.clear();
	}
	
	public void clearVars()
	{
		_vtbl.clear();
	}
	
	public void clear()
	{
		clearFuncs();
		clearVars();
	}
	
	public void setVar(String name, double value)
	{
		_vtbl.put(name, value);
	}
	
	public double getVar(String name)
	{
		if (_vtbl.containsKey(name))
		{
			return _vtbl.get(name);
		}
		return Double.NaN;
	}
	
	public void setFunction(String name, UnaryFunctor funcimpl)
	{
		_ftbl.put(name, funcimpl);
	}
	
	public double applyFunction(String name, double x)
	{
		if (_ftbl.containsKey(name))
		{
			return _ftbl.get(name).apply(x);
		}
		return Double.NaN;
	}
}
