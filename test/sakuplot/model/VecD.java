package test.sakuplot.model;

import java.awt.Point;

/**
 * 2次元の浮動小数点座標を表現します。
 */
public final class VecD
{
	/**
	 * このベクトルの X 座標です。
	 */
	public double x;
	
	/**
	 * このベクトルの Y 座標です。
	 */
	public double y;
	
	/**
	 * 原点を指すベクトルを作成します。
	 */
	public VecD()
	{
		this(0, 0);
	}
	
	/**
	 * <code>java.awt.Point</code>型で表された座標を指すベクトルを作成します。
	 * @param point 座標
	 */
	public VecD(Point point)
	{
		this(point.x, point.y);
	}
	
	/**
	 * 指定した座標を指すベクトルを作成します。
	 * @param x ベクトルの X 座標
	 * @param y ベクトルの Y 座標
	 */
	public VecD(double x, double y)
	{
		this.x = x;
		this.y = y;
	}
	
	/**
	 * 指定した座標を指すようにベクトルの各成分を設定します。
	 * @param x ベクトルの X 座標
	 * @param y ベクトルの Y 座標
	 */
	public void set(double x, double y)
	{
		this.x = x;
		this.y = y;
	}
	
	/**
	 * ベクトルの和を計算します。
	 * @param v 加算するベクトル
	 * @return ベクトル和
	 */
	public VecD add(VecD v)
	{
		return new VecD(x + v.x, y + v.y);
	}
	
	/**
	 * ベクトルの和を計算します。
	 * @param x 加算するベクトルの X 座標
	 * @param y 加算するベクトルの Y 座標
	 * @return ベクトル和
	 */
	public VecD add(double x, double y)
	{
		return new VecD(this.x + x, this.y + y);
	}
	
	/**
	 * ベクトルの差を計算します。
	 * @param v 減算するベクトル
	 * @return ベクトル差
	 */
	public VecD sub(VecD v)
	{
		return new VecD(x - v.x, y - v.y);
	}
	
	/**
	 * ベクトルの差を計算します。
	 * @param x 減算するベクトルの X 座標
	 * @param y 減算するベクトルの Y 座標
	 * @return ベクトル差
	 */
	public VecD sub(double x, double y)
	{
		return new VecD(this.x - x, this.y - y);
	}
	
	/**
	 * ベクトルの各要素をスカラー定数倍します。
	 * @param k 乗算する係数
	 * @return 各要素が <code>k</code> 倍されたベクトル
	 */
	public VecD scale(double k)
	{
		return new VecD(x * k, y * k); 
	}
	
	/**
	 * ベクトルの各要素にスカラー定数倍します。
	 * @param a X 成分に乗算する係数
	 * @param b Y 成分に乗算する係数
	 * @return X 成分が <code>a</code> 倍、Y 成分が <code>b</code> 倍されたベクトル
	 */
	public VecD scale(double a, double b)
	{
		return new VecD(x * a, y * b); 
	}
	
	/**
	 * ベクトルの各要素をスカラー定数で除算します。
	 * @param k 各要素に対する除数
	 * @return 各要素が <code>k</code> で除算されたベクトル
	 */
	public VecD div(double k)
	{
		return new VecD(x / k, y / k); 
	}
	
	/**
	 * 内積を計算します。
	 * @param v 内積を計算するベクトル
	 * @return 内積
	 */
	public double dot(VecD v)
	{
		return x * v.x + y * v.y;
	}
	
	/**
	 * このベクトル自身との内積を返します。
	 * @return このベクトル自身との内積
	 */
	public double square()
	{
		return dot(this);
	}
	
	/**
	 * このベクトルの原点からの距離を返します。
	 * @return このベクトルの原点からの距離
	 */
	public double length()
	{
		return Math.sqrt(square());
	}
	
	/**
	 * このベクトルと指定されたベクトルの指す点間の距離の2乗を計算します。
	 * @param v 距離の2乗を計算するベクトル
	 * @return 距離の2乗
	 */
	public double distanceSq(VecD v)
	{
		return VecD.distanceSq(this, v);
	}
	
	/**
	 * このベクトルと指定されたベクトルの指す点間の距離を計算します。
	 * @param v 距離を計算するベクトル
	 * @return 距離
	 */
	public double distance(VecD v)
	{
		return VecD.distance(this, v);
	}
	
	/**
	 * このベクトルの要素に絶対値が無限量のものがあるか検査します。
	 * @return 絶対値が無限量となる要素をもつ場合に <code>true</code>、そうでない場合に <code>false</code>
	 */
	public boolean isInf()
	{
		return isInf(this);
	}
	
	/**
	 * このベクトルの要素に非数 (NaN) となるものがあるか検査します。
	 * @return 非数 (NaN) となる要素をもつ場合に <code>true</code>、そうでない場合に <code>false</code>
	 */
	public boolean isNaN()
	{
		return isNaN(this);
	}
	
	/**
	 * このベクトルの各要素が正または負の無限大でなくかつ非数でないか検査します。
	 * @return 各要素が正または負の無限大でなくかつ非数でない場合に <code>true</code>、そうでない場合に <code>false</code>
	 */
	public boolean isValid()
	{
		return isValid(this);
	}
	
	/**
	 * このベクトルが指す座標を表す <code>java.awt.Point</code> 型のオブジェクトを返します。
	 * @return 変換された <code>java.awt.Point</code> 型オブジェクト
	 */
	public Point toPoint()
	{
		return new Point((int)Math.floor(x + 0.5D), (int)Math.floor(y + 0.5D));
	}
	
	/**
	 * このベクトルの文字列表現を返します。
	 * @return このベクトルの文字列表現
	 */
	public String toString()
	{
		return "[" + x + "," + y + "]";
	}
	
	/**
	 * 2つのベクトルの指す点間の距離の2乗を返します。
	 * @param v1 ベクトル
	 * @param v2 ベクトル
	 * @return 2つのベクトルの指す点間の距離の2乗
	 */
	public static double distanceSq(VecD v1, VecD v2)
	{
		return v1.sub(v2).square();
	}
	
	/**
	 * 2つのベクトルの指す点間の距離を返します。
	 * @param v1 ベクトル
	 * @param v2 ベクトル
	 * @return 2つのベクトルの指す点間の距離
	 */
	public static double distance(VecD v1, VecD v2)
	{
		return v1.sub(v2).length();
	}
	
	/**
	 * ベクトルの要素に絶対値が無限量のものがあるか検査します。
	 * @param v 判定されるベクトル
	 * @return 絶対値が無限量となる要素をもつ場合に <code>true</code>、そうでない場合に <code>false</code>
	 */
	public static boolean isInf(VecD v)
	{
		return Double.isInfinite(v.x) || Double.isInfinite(v.y);
	}
	
	/**
	 * ベクトルの要素に非数 (NaN) となるものがあるか検査します。
	 * @param v 判定されるベクトル
	 * @return 非数 (NaN) となる要素をもつ場合に <code>true</code>、そうでない場合に <code>false</code>
	 */
	public static boolean isNaN(VecD v)
	{
		return Double.isNaN(v.x) || Double.isNaN(v.y);
	}
	
	/**
	 * ベクトルの各要素が正または負の無限大でなくかつ非数でないか検査します。
	 * @param v 判定されるベクトル
	 * @return 各要素が正または負の無限大でなくかつ非数でない場合に <code>true</code>、そうでない場合に <code>false</code>
	 */
	public static boolean isValid(VecD v)
	{
		return !isInf(v) && !isNaN(v);
	}
}
