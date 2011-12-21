package test.sakuplot.exp;

import java.util.Set;

public class ExpFunc extends Expression
{
	private String _name;
	private Expression _e;
	
	public ExpFunc(String name, Expression e)
	{
		_name = name.toLowerCase();
		_e = e;
	}
	
	public double getValue(ExpressionEnv env)
	{
		return env.applyFunction(_name, _e.getValue(env));
	}
	
	public Set<String> getVariables()
	{
		return _e.getVariables();
	}
	
	public String toString()
	{
		return _name + "(" + _e.toString() + ")";
	}
}
