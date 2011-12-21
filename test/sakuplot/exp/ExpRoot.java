package test.sakuplot.exp;

import java.util.Set;

/**
 * �����̌��ƂȂ����e�L�X�g�\����ێ�����m�[�h�ł��B
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
