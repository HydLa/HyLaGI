package test.sakuplot.model;

import test.sakuplot.exp.Expression;

/**
 * �O���t1���̃��f����\���܂��B
 * �}��ϐ��\����2�̊֐� (X(t), Y(t)) �ƕϐ� t �͈̔͂������܂��B
 */
public class GraphModel
{
	private Expression _xfunc;
	private Expression _yfunc;
	private double _tMin;
	private double _tMax;

	private GraphModel()
	{
	}

	public Expression getExpressionX()
	{
		return _xfunc;
	}

	public void setExpressionX(Expression exp)
	{
		_xfunc = exp;
	}

	public Expression getExpressionY()
	{
		return _yfunc;
	}

	public void setExpressionY(Expression exp)
	{
		_yfunc = exp;
	}

	public double getMinT()
	{
		return _tMin;
	}

	public void setMinT(double t)
	{
		_tMin = t;
	}

	public double getMaxT()
	{
		return _tMax;
	}

	public void setMaxT(double t)
	{
		_tMax = t;
	}

	public String toString()
	{
		return "x(t) = " + _xfunc + ", y(t) = " + _yfunc + ", t: [" + _tMin + ", " + _tMax + "]";
	}

	/**
	 * <p>
	 * �C���^�[�o���t�F�[�Y�Ɋ܂܂��2�̕ϐ����w�肵�ăO���t���쐬���܂��B
	 * </p>
	 * <p><b>�O�����:</b> 2�̕ϐ����̓C���^�[�o���t�F�[�Y���ɒ�`����Ă��āA���̐����͓K�؂Ȍ`���ł���</p>
	 * @param ip �C���^�[�o���t�F�[�Y�I�u�W�F�N�g
	 * @param xVar X ���ɑΉ�������ϐ�
	 * @param yVar Y ���ɑΉ�������ϐ�
	 * @return ��A�̃O���t�I�u�W�F�N�g
	 * @throws RuntimeException �w�肳�ꂽ�ϐ������C���^�[�o���t�F�[�Y���ɒ�`����Ă��Ȃ��ꍇ�܂��͐������������Ȃ��ꍇ
	 */
	public static GraphModel createFromIP(IntervalPhase ip, String xVar, String yVar) throws RuntimeException
	{
		Expression x = Expression.create(ip.getFunction(xVar));
		Expression y = Expression.create(ip.getFunction(yVar));

		if (x == null || y == null)
		{
			throw new RuntimeException("�����̉�͂Ɏ��s���܂����BGraphModel �̍쐬���ł��܂���B");
		}

		GraphModel g = new GraphModel();
		g.setExpressionX(x);
		g.setExpressionY(y);
		g.setMinT(ip.getMinT());
		g.setMaxT(ip.getMaxT());
		return g;
	}
}
