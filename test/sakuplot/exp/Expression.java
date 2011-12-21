package test.sakuplot.exp;

import java.util.HashSet;
import java.util.Set;

import test.sakuplot.exp.parser.Lexer;
import test.sakuplot.exp.parser.Parser;
import test.sakuplot.exp.parser.ParserException;

public abstract class Expression
{
	/**
	 * <p>
	 * 文字列をパースして数式オブジェクトを生成します。
	 * 数式の構文解析に失敗した場合は <code>null</code> を返します。
	 * </p>
	 * @param text 数式文字列
	 * @return 数式オブジェクト
	 */
	public static Expression create(String text)
	{
		Parser parser = new Parser(new Lexer(text));
		try
		{
			return new ExpRoot(text, parser.parseRoot());
		}
		catch (ParserException e)
		{
			e.printStackTrace();
		}
		return null;
	}

	/**
	 * <p>
	 * 指定された式環境を用いてこの数式の値を計算します。
	 * 式環境中で定義されていない変数や関数が参照された場合は <code>NaN</code> を返します。
	 * </p>
	 * @param env 式環境
	 * @return 計算結果の値
	 */
	public abstract double getValue(ExpressionEnv env);

	/**
	 * この数式中で参照される変数名のセットを返します。
	 * @return この数式が参照する変数名のセット
	 */
	public Set<String> getVariables()
	{
		return new HashSet<String>();
	}

	/**
	 * この数式の文字列表現を返します。
	 * @return この数式の文字列表現
	 */
	public abstract String toString();
}
