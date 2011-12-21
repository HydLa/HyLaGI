package test.sakuplot.exp.parser;

/**
 * 字句要素を表します。
 */
class Token
{
	public String _text;
	public TokenType _type;
	
	/**
	 * 字句文字列と字句種別から字句要素を作成します。
	 * @param text 字句文字列
	 * @param type 字句種別
	 */
	public Token(String text, TokenType type)
	{
		_text = text;
		_type = type;
	}
	
	/**
	 * 字句文字列を取得します。
	 * @return 字句文字列
	 */
	public String getText()
	{
		return _text;
	}
	
	/**
	 * 字句種別を取得します。
	 * @return 字句種別
	 */
	public TokenType getType()
	{
		return _type;
	}
	
	/**
	 * この字句オブジェクトの文字列表現を返します。
	 * このメソッドは主にデバッグ用途に使用されます。
	 * 字句文字列を得るには {@link #getText()} メソッドを使用してください。
	 * @return この字句オブジェクトの文字列表現
	 */
	public String toString()
	{
		return _text + "<type:" + _type + ">";
	}
}
