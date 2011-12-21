package test.sakuplot.model;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * いくつかのグラフを組み合わせて作られる複合グラフを表します。
 * コンポジットパターンにすることも考えられますが、ケースと IP との対応を考えてとりあえずはこの構造です。
 */
public class CompoundGraph
{
	private String _name;
	private List<GraphModel> _graphList = null;
	
	private CompoundGraph(String name, List<GraphModel> graphList)
	{
		_name = name;
		_graphList = Collections.unmodifiableList(graphList);
	}
	
	/**
	 * この複合グラフに付けられた名前を返します。
	 * @return 名前文字列
	 */
	public String getName()
	{
		return _name;
	}
	
	/**
	 * この複合グラフに含まれるグラフのリストを返します。
	 * @return グラフオブジェクトのリスト
	 */
	public List<GraphModel> getGraphList()
	{
		return _graphList;
	}
	
	public String toString()
	{
		return getName();
	}
	
	/**
	 * <p>シミュレーションケースオブジェクト中の2つの変数を指定して複合グラフオブジェクトを作成します。</p>
	 * <p><b>前提条件:</b> 2つの変数名はインターバルフェーズ中に定義されていて、その数式は適切な形式である</p>
	 * @param simulation シミュレーションケースオブジェクト
	 * @param xVar X 軸に対応させる変数名
	 * @param yVar Y 軸に対応させる変数名
	 * @return 複合グラフオブジェクト
	 * @throws RuntimeException 指定された変数名が定義されていない場合、またはその数式の形式が正しくない場合
	 */
	public static CompoundGraph createFromSimulation(SimulationCase simulation, String xVar, String yVar) throws RuntimeException
	{
		List<GraphModel> graphList = new ArrayList<GraphModel>();
		for (IntervalPhase ip : simulation.getIntervalPhases())
		{
			graphList.add(GraphModel.createFromIP(ip, xVar, yVar));
		}
		return new CompoundGraph(simulation.getName(), graphList);
	}
}
