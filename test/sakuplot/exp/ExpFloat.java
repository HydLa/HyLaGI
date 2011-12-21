package test.sakuplot.exp;

public class ExpFloat extends Expression
{
	private String _text;
	private double _value;
	
	public ExpFloat(String text)
	{
		_text = text;
		_value = Double.parseDouble(text);
	}
	
	public double getValue(ExpressionEnv env)
	{
		return _value;
	}
	
	public String toString()
	{
		return _text;
	}
}
