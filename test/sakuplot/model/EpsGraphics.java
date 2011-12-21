package test.sakuplot.model;

/**
 * PostScriptグラフィックスデータをカプセル化します。
 */
public class EpsGraphics
{
	private static final String ENDL = "\n";
	
	private int _width;
	private int _height;
	private StringBuilder _value = new StringBuilder();
	
	/**
	 * 幅、高さを指定してEPSグラフィックを作成します。
	 * @param width イメージの幅
	 * @param height イメージの高さ
	 */
	public EpsGraphics(int width, int height)
	{
		_width = width;
		_height = height;
		clear();
	}
	
	/**
	 * イメージの幅を取得します。
	 * @return 幅
	 */
	public int getWidth()
	{
		return _width;
	}
	
	/**
	 * イメージの幅を設定します。
	 * @param width 幅
	 */
	public void setWidth(int width)
	{
		_width = width;
	}
	
	/**
	 * イメージの高さを取得します。
	 * @return 高さ
	 */
	public int getHeight()
	{
		return _height;
	}
	
	/**
	 * イメージの高さを設定します。
	 * @param height 高さ
	 */
	public void setHeight(int height)
	{
		_height = height;
	}
	
	/**
	 * 描画された内容を消去し、イメージを初期化します。
	 */
	public void clear()
	{
		_value.setLength(0);
	}
	
	/**
	 * ペンを指定座標へ移動します。
	 * @param x ペンのx座標
	 * @param y ペンのy座標
	 */
	public void moveTo(double x, double y)
	{
		appendLine(String.format("%.2f %.2f M", x, y));
	}
	
	/**
	 * 現在のペンの位置から指定座標へ線を引きます。
	 * @param x 終点のx座標
	 * @param y 終点のy座標
	 */
	public void lineTo(double x, double y)
	{
		appendLine(String.format("%.2f %.2f L", x, y));
	}
	
	/**
	 * 今までに指示した内容を実際に描画します。
	 */
	public void stroke()
	{
		appendLine("stroke");
	}
	
	/**
	 * 線幅を設定します。
	 * @param width 線の幅
	 */
	public void setLineWidth(double width)
	{
		appendLine(String.format("%.2f setlinewidth", width));
	}
	
	/**
	 * 点線・破線パターンを設定します。
	 * @param pattern 描画ピクセル数、空白ピクセル数を交互に格納した配列
	 */
	public void setDash(int[] pattern)
	{
		setDash(pattern, 0);
	}
	
	/**
	 * オフセットを指定して点線・破線パターンを設定します。
	 * @param pattern 描画ピクセル数、空白ピクセル数を交互に格納した配列
	 * @param offset パターン配列のどの要素を始点とするかのオフセット
	 */
	public void setDash(int[] pattern, int offset)
	{
		String arrayStr = "";
		for (int p : pattern) arrayStr += " " + p;
		appendLine(String.format("[%s] %d setdash", arrayStr, offset));
	}
	
	/**
	 * 点線・破線パターンを破棄しペンの状態を実線にします。
	 */
	public void setSolid()
	{
		setDash(new int[] { });
	}
	
	/**
	 * 描画内容を含むEPS形式の文字列を取得します。
	 * @return EPSテキスト
	 */
	public String getContents()
	{
		StringBuilder contents = new StringBuilder();
		
		contents.append("%!PS-Adobe-3.0 EPSF-3.0" + ENDL);
		contents.append("%%BoundingBox: 0 0 " + _width + " " + _height + ENDL);
		contents.append("/gdict 120 dict def" + ENDL);
		contents.append("gdict begin" + ENDL);
		contents.append("gsave" + ENDL);
		contents.append("/M {moveto} def" + ENDL);
		contents.append("/L {lineto} def" + ENDL);
		contents.append(_value);
		contents.append("showpage" + ENDL);
		contents.append("grestore");
		
		return contents.toString();
	}
	
	private void appendLine(String str)
	{
		_value.append(str);
		_value.append(ENDL);
	}
}
