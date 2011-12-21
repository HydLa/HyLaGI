package test.sakuplot.model;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * シミュレーション結果を表現します。
 */
public class SimulationCase
{
	private String _name;
	private List<IntervalPhase> _ipList = new ArrayList<IntervalPhase>();
	
	/**
	 * シミュレーション結果を作成します。
	 * @param name - このシミュレーション結果に付ける名前
	 * @throws IllegalArgumentException 引数が <code>null</code> の場合
	 */
	public SimulationCase(String name) throws IllegalArgumentException
	{
		if (name == null)
			throw new IllegalArgumentException("'name' must not be null.");
		
		_name = name;
	}
	
	/**
	 * このシミュレーション結果の名前を取得します。
	 * @return 名前文字列
	 */
	public String getName()
	{
		return _name;
	}
	
	/**
	 * インターバルフェーズを追加します。
	 * @param ip - 追加するインターバルフェーズ
	 * @throws IllegalArgumentException 引数が <code>null</code> の場合
	 */
	public void addIntervalPhase(IntervalPhase ip) throws IllegalArgumentException
	{
		if (ip == null)
			throw new IllegalArgumentException("'ip' must not be null.");
		
		_ipList.add(ip);
	}
	
	/**
	 * このシミュレーションケースに含まれる変数名のセットを返します。
	 * @return 変数名のセット
	 */
	public Set<String> getFunctionSet()
	{
		if (_ipList.size() != 0)
		{
			return _ipList.get(0).getFunctionSet();
		}
		return new HashSet<String>();
	}
	
	/**
	 * シミュレーション結果に含まれるインターバルフェーズのリストを取得します。
	 * @return インターバルフェーズの読み取り専用リスト
	 */
	public List<IntervalPhase> getIntervalPhases()
	{
		return Collections.unmodifiableList(_ipList);
	}
}
