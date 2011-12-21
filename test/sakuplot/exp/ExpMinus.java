package test.sakuplot.exp;

import java.util.Set;

public class ExpMinus extends Expression
{
	private Expression _e;
	
	public ExpMinus(Expression e)
	{
		_e = e;
	}
	
	public double getValue(ExpressionEnv env)
	{
		return -_e.getValue(env);
	}
	
	public Set<String> getVariables()
	{
		return _e.getVariables();
	}
	
	public String toString()
	{
		return "-(" + _e.toString() + ")";
	}
}
