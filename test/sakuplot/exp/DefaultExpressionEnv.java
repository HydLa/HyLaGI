package test.sakuplot.exp;

/**
 * �ʏ�悭�g����Ǝv����萔����ъ֐������炩���ߒ�`���ꂽ�����ł��B
 */
public class DefaultExpressionEnv extends ExpressionEnv
{
	/**
	 * �ʏ�悭�g����Ǝv����萔����ъ֐������炩���ߒ�`���ꂽ�������쐬���܂��B
	 */
	public DefaultExpressionEnv()
	{
		setFunction("sin", new UnaryFunctor()
		{
			public double apply(double x) { return Math.sin(x); }
		});
		setFunction("cos", new UnaryFunctor()
		{
			public double apply(double x) { return Math.cos(x); }
		});
		setFunction("tan", new UnaryFunctor()
		{
			public double apply(double x) { return Math.tan(x); }
		});
		setFunction("asin", new UnaryFunctor()
		{
			public double apply(double x) { return Math.asin(x); }
		});
		setFunction("acos", new UnaryFunctor()
		{
			public double apply(double x) { return Math.acos(x); }
		});
		setFunction("atan", new UnaryFunctor()
		{
			public double apply(double x) { return Math.atan(x); }
		});
		setFunction("exp", new UnaryFunctor()
		{
			public double apply(double x) { return Math.exp(x); }
		});
		setFunction("log", new UnaryFunctor()
		{
			public double apply(double x) { return Math.log(x); }
		});
		setFunction("log10", new UnaryFunctor()
		{
			public double apply(double x) { return Math.log10(x); }
		});
		setFunction("sqrt", new UnaryFunctor()
		{
			public double apply(double x) { return Math.sqrt(x); }
		});
		setFunction("cbrt", new UnaryFunctor()
		{
			public double apply(double x) { return Math.cbrt(x); }
		});
		
		setVar("pi", Math.PI);
		setVar("e", Math.E);
	}
}
