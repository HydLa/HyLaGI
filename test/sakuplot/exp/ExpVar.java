package test.sakuplot.exp;

import java.util.HashSet;
import java.util.Set;

public class ExpVar extends Expression
{
	private String _name;
	
	public ExpVar(String name)
	{
		_name = name;
	}
	
	public double getValue(ExpressionEnv env)
	{
		return env.getVar(_name);
	}
	
	public Set<String> getVariables()
	{
		Set<String> set = new HashSet<String>();
		set.add(_name);
		return set;
	}
	
	public String toString()
	{
		return _name;
	}
}
