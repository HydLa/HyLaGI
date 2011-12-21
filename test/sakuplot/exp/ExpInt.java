package test.sakuplot.exp;

public class ExpInt extends Expression
{
	private String _text;
	private int _value;
	
	public ExpInt(String text)
	{
		_text = text;
		_value = Integer.parseInt(text);
	}
	
	public double getValue(ExpressionEnv env)
	{
		return (double)_value;
	}
	
	public String toString()
	{
		return _text;
	}
}
