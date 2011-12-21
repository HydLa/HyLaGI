package test.sakuplot.exp;

import java.util.HashSet;
import java.util.Set;

public class ExpSub extends Expression
{
	private Expression _e1, _e2;
	
	public ExpSub(Expression e1, Expression e2)
	{
		_e1 = e1;
		_e2 = e2;
	}
	
	public double getValue(ExpressionEnv env)
	{
		return _e1.getValue(env) - _e2.getValue(env);
	}
	
	public Set<String> getVariables()
	{
		Set<String> set = new HashSet<String>(_e1.getVariables());
		set.addAll(_e2.getVariables());
		return set;
	}
	
	public String toString()
	{
		return "(" + _e1.toString() + ")-(" + _e2.toString() + ")";
	}
}
