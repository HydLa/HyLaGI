package test.sakuplot.exp;

import java.util.Set;

/**
 * 数式の元となったテキスト表現を保持するノードです。
 */
public class ExpRoot extends Expression
{
	private String _srcText;
	private Expression _exp;
	
	public ExpRoot(String srcText, Expression exp)
	{
		_srcText = srcText;
		_exp = exp;
	}
	
	public double getValue(ExpressionEnv env)
	{
		return _exp.getValue(env);
	}
	
	public Set<String> getVariables()
	{
		return _exp.getVariables();
	}
	
	public String toString()
	{
		return _srcText;
	}
}
