package test.sakuplot.model;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class IntervalPhase
{
	private double _min_t;
	private double _max_t;
	private Map<String, String> _funcs = new HashMap<String, String>();
	
	/**
	 * インターバルフェーズを作成します。
	 */
	public IntervalPhase()
	{
		this(0, 0);
	}
	
	/**
	 * 定義域を指定してインターバルフェーズを作成します。
	 * @param min_t 定義域の下限値
	 * @param max_t 定義域の上限値
	 */
	public IntervalPhase(double min_t, double max_t)
	{
		_min_t = min_t;
		_max_t = max_t;
	}
	
	public double getMinT()
	{
		return _min_t;
	}
	
	public void setMinT(double min_t)
	{
		_min_t = min_t;
	}
	
	public double getMaxT()
	{
		return _max_t;
	}
	
	public void setMaxT(double max_t)
	{
		_max_t = max_t;
	}
	
	/**
	 * このインターバルフェーズの定義域を設定します。
	 * @param min_t 定義域の下限値
	 * @param max_t 定義域の上限値
	 */
	public void setRange(double min_t, double max_t)
	{
		_min_t = min_t;
		_max_t = max_t;
	}
	
	/**
	 * 変数 name を定義する時間関数を設定します。
	 * @param name 変数名
	 * @param expr 数式文字列
	 * @throws IllegalArgumentException 引数のいずれかが <code>NULL</code> または空の文字列である場合
	 */
	public void setFunction(String name, String expr) throws IllegalArgumentException
	{
		if (name == null || name.isEmpty() || expr == null || expr.isEmpty())
			throw new IllegalArgumentException("Parameters cannot be null or empty.");
		
		_funcs.put(name, expr);
	}
	
	/**
	 * 変数 <code>name</code> を定義する時間関数を取得します。
	 * @param name 変数名
	 * @return 関数文字列
	 * @throws IllegalArgumentException 引数が <code>null</code> または空の文字列である場合
	 * @throws RuntimeException <code>name</code> で指定されたキーが存在しない場合
	 */
	public String getFunction(String name) throws IllegalArgumentException, RuntimeException
	{
		if (name == null || name.isEmpty())
			throw new IllegalArgumentException("Parameter must not be null or empty.");
		
		if (!_funcs.containsKey(name))
			throw new RuntimeException("Key [" + name + "] was not found.");
		
		return _funcs.get(name);
	}
	
	@Deprecated
	public Map<String, String> getDefinedFunctions()
	{
		return Collections.unmodifiableMap(_funcs);
	}
	
	/**
	 * このインターバルフェーズで定義されている変数のセットを返します。
	 * @return 変数名のセット
	 */
	public Set<String> getFunctionSet()
	{
		return Collections.unmodifiableSet(_funcs.keySet());
	}
	
	/**
	 * オブジェクトの文字列表現を返します。
	 * @return このオブジェクトの文字列表現
	 */
	public String toString()
	{
		return String.format("t:[%f,%f]", _min_t, _max_t) + _funcs.toString();
	}
}
