package test.sakuplot.model;

import test.sakuplot.exp.Expression;

/**
 * グラフ1つ分のモデルを表します。
 * 媒介変数表示の2つの関数 (X(t), Y(t)) と変数 t の範囲を持ちます。
 */
public class GraphModel
{
	private Expression _xfunc;
	private Expression _yfunc;
	private double _tMin;
	private double _tMax;

	private GraphModel()
	{
	}

	public Expression getExpressionX()
	{
		return _xfunc;
	}

	public void setExpressionX(Expression exp)
	{
		_xfunc = exp;
	}

	public Expression getExpressionY()
	{
		return _yfunc;
	}

	public void setExpressionY(Expression exp)
	{
		_yfunc = exp;
	}

	public double getMinT()
	{
		return _tMin;
	}

	public void setMinT(double t)
	{
		_tMin = t;
	}

	public double getMaxT()
	{
		return _tMax;
	}

	public void setMaxT(double t)
	{
		_tMax = t;
	}

	public String toString()
	{
		return "x(t) = " + _xfunc + ", y(t) = " + _yfunc + ", t: [" + _tMin + ", " + _tMax + "]";
	}

	/**
	 * <p>
	 * インターバルフェーズに含まれる2つの変数を指定してグラフを作成します。
	 * </p>
	 * <p><b>前提条件:</b> 2つの変数名はインターバルフェーズ中に定義されていて、その数式は適切な形式である</p>
	 * @param ip インターバルフェーズオブジェクト
	 * @param xVar X 軸に対応させる変数
	 * @param yVar Y 軸に対応させる変数
	 * @return 一連のグラフオブジェクト
	 * @throws RuntimeException 指定された変数名がインターバルフェーズ中に定義されていない場合または数式が正しくない場合
	 */
	public static GraphModel createFromIP(IntervalPhase ip, String xVar, String yVar) throws RuntimeException
	{
		Expression x = Expression.create(ip.getFunction(xVar));
		Expression y = Expression.create(ip.getFunction(yVar));

		if (x == null || y == null)
		{
			throw new RuntimeException("数式の解析に失敗しました。GraphModel の作成ができません。");
		}

		GraphModel g = new GraphModel();
		g.setExpressionX(x);
		g.setExpressionY(y);
		g.setMinT(ip.getMinT());
		g.setMaxT(ip.getMaxT());
		return g;
	}
}
