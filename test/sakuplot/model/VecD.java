package test.sakuplot.model;

import java.awt.Point;

/**
 * 2�����̕��������_���W��\�����܂��B
 */
public final class VecD
{
	/**
	 * ���̃x�N�g���� X ���W�ł��B
	 */
	public double x;
	
	/**
	 * ���̃x�N�g���� Y ���W�ł��B
	 */
	public double y;
	
	/**
	 * ���_���w���x�N�g�����쐬���܂��B
	 */
	public VecD()
	{
		this(0, 0);
	}
	
	/**
	 * <code>java.awt.Point</code>�^�ŕ\���ꂽ���W���w���x�N�g�����쐬���܂��B
	 * @param point ���W
	 */
	public VecD(Point point)
	{
		this(point.x, point.y);
	}
	
	/**
	 * �w�肵�����W���w���x�N�g�����쐬���܂��B
	 * @param x �x�N�g���� X ���W
	 * @param y �x�N�g���� Y ���W
	 */
	public VecD(double x, double y)
	{
		this.x = x;
		this.y = y;
	}
	
	/**
	 * �w�肵�����W���w���悤�Ƀx�N�g���̊e������ݒ肵�܂��B
	 * @param x �x�N�g���� X ���W
	 * @param y �x�N�g���� Y ���W
	 */
	public void set(double x, double y)
	{
		this.x = x;
		this.y = y;
	}
	
	/**
	 * �x�N�g���̘a���v�Z���܂��B
	 * @param v ���Z����x�N�g��
	 * @return �x�N�g���a
	 */
	public VecD add(VecD v)
	{
		return new VecD(x + v.x, y + v.y);
	}
	
	/**
	 * �x�N�g���̘a���v�Z���܂��B
	 * @param x ���Z����x�N�g���� X ���W
	 * @param y ���Z����x�N�g���� Y ���W
	 * @return �x�N�g���a
	 */
	public VecD add(double x, double y)
	{
		return new VecD(this.x + x, this.y + y);
	}
	
	/**
	 * �x�N�g���̍����v�Z���܂��B
	 * @param v ���Z����x�N�g��
	 * @return �x�N�g����
	 */
	public VecD sub(VecD v)
	{
		return new VecD(x - v.x, y - v.y);
	}
	
	/**
	 * �x�N�g���̍����v�Z���܂��B
	 * @param x ���Z����x�N�g���� X ���W
	 * @param y ���Z����x�N�g���� Y ���W
	 * @return �x�N�g����
	 */
	public VecD sub(double x, double y)
	{
		return new VecD(this.x - x, this.y - y);
	}
	
	/**
	 * �x�N�g���̊e�v�f���X�J���[�萔�{���܂��B
	 * @param k ��Z����W��
	 * @return �e�v�f�� <code>k</code> �{���ꂽ�x�N�g��
	 */
	public VecD scale(double k)
	{
		return new VecD(x * k, y * k); 
	}
	
	/**
	 * �x�N�g���̊e�v�f�ɃX�J���[�萔�{���܂��B
	 * @param a X �����ɏ�Z����W��
	 * @param b Y �����ɏ�Z����W��
	 * @return X ������ <code>a</code> �{�AY ������ <code>b</code> �{���ꂽ�x�N�g��
	 */
	public VecD scale(double a, double b)
	{
		return new VecD(x * a, y * b); 
	}
	
	/**
	 * �x�N�g���̊e�v�f���X�J���[�萔�ŏ��Z���܂��B
	 * @param k �e�v�f�ɑ΂��鏜��
	 * @return �e�v�f�� <code>k</code> �ŏ��Z���ꂽ�x�N�g��
	 */
	public VecD div(double k)
	{
		return new VecD(x / k, y / k); 
	}
	
	/**
	 * ���ς��v�Z���܂��B
	 * @param v ���ς��v�Z����x�N�g��
	 * @return ����
	 */
	public double dot(VecD v)
	{
		return x * v.x + y * v.y;
	}
	
	/**
	 * ���̃x�N�g�����g�Ƃ̓��ς�Ԃ��܂��B
	 * @return ���̃x�N�g�����g�Ƃ̓���
	 */
	public double square()
	{
		return dot(this);
	}
	
	/**
	 * ���̃x�N�g���̌��_����̋�����Ԃ��܂��B
	 * @return ���̃x�N�g���̌��_����̋���
	 */
	public double length()
	{
		return Math.sqrt(square());
	}
	
	/**
	 * ���̃x�N�g���Ǝw�肳�ꂽ�x�N�g���̎w���_�Ԃ̋�����2����v�Z���܂��B
	 * @param v ������2����v�Z����x�N�g��
	 * @return ������2��
	 */
	public double distanceSq(VecD v)
	{
		return VecD.distanceSq(this, v);
	}
	
	/**
	 * ���̃x�N�g���Ǝw�肳�ꂽ�x�N�g���̎w���_�Ԃ̋������v�Z���܂��B
	 * @param v �������v�Z����x�N�g��
	 * @return ����
	 */
	public double distance(VecD v)
	{
		return VecD.distance(this, v);
	}
	
	/**
	 * ���̃x�N�g���̗v�f�ɐ�Βl�������ʂ̂��̂����邩�������܂��B
	 * @return ��Βl�������ʂƂȂ�v�f�����ꍇ�� <code>true</code>�A�����łȂ��ꍇ�� <code>false</code>
	 */
	public boolean isInf()
	{
		return isInf(this);
	}
	
	/**
	 * ���̃x�N�g���̗v�f�ɔ� (NaN) �ƂȂ���̂����邩�������܂��B
	 * @return �� (NaN) �ƂȂ�v�f�����ꍇ�� <code>true</code>�A�����łȂ��ꍇ�� <code>false</code>
	 */
	public boolean isNaN()
	{
		return isNaN(this);
	}
	
	/**
	 * ���̃x�N�g���̊e�v�f�����܂��͕��̖�����łȂ����񐔂łȂ����������܂��B
	 * @return �e�v�f�����܂��͕��̖�����łȂ����񐔂łȂ��ꍇ�� <code>true</code>�A�����łȂ��ꍇ�� <code>false</code>
	 */
	public boolean isValid()
	{
		return isValid(this);
	}
	
	/**
	 * ���̃x�N�g�����w�����W��\�� <code>java.awt.Point</code> �^�̃I�u�W�F�N�g��Ԃ��܂��B
	 * @return �ϊ����ꂽ <code>java.awt.Point</code> �^�I�u�W�F�N�g
	 */
	public Point toPoint()
	{
		return new Point((int)Math.floor(x + 0.5D), (int)Math.floor(y + 0.5D));
	}
	
	/**
	 * ���̃x�N�g���̕�����\����Ԃ��܂��B
	 * @return ���̃x�N�g���̕�����\��
	 */
	public String toString()
	{
		return "[" + x + "," + y + "]";
	}
	
	/**
	 * 2�̃x�N�g���̎w���_�Ԃ̋�����2���Ԃ��܂��B
	 * @param v1 �x�N�g��
	 * @param v2 �x�N�g��
	 * @return 2�̃x�N�g���̎w���_�Ԃ̋�����2��
	 */
	public static double distanceSq(VecD v1, VecD v2)
	{
		return v1.sub(v2).square();
	}
	
	/**
	 * 2�̃x�N�g���̎w���_�Ԃ̋�����Ԃ��܂��B
	 * @param v1 �x�N�g��
	 * @param v2 �x�N�g��
	 * @return 2�̃x�N�g���̎w���_�Ԃ̋���
	 */
	public static double distance(VecD v1, VecD v2)
	{
		return v1.sub(v2).length();
	}
	
	/**
	 * �x�N�g���̗v�f�ɐ�Βl�������ʂ̂��̂����邩�������܂��B
	 * @param v ���肳���x�N�g��
	 * @return ��Βl�������ʂƂȂ�v�f�����ꍇ�� <code>true</code>�A�����łȂ��ꍇ�� <code>false</code>
	 */
	public static boolean isInf(VecD v)
	{
		return Double.isInfinite(v.x) || Double.isInfinite(v.y);
	}
	
	/**
	 * �x�N�g���̗v�f�ɔ� (NaN) �ƂȂ���̂����邩�������܂��B
	 * @param v ���肳���x�N�g��
	 * @return �� (NaN) �ƂȂ�v�f�����ꍇ�� <code>true</code>�A�����łȂ��ꍇ�� <code>false</code>
	 */
	public static boolean isNaN(VecD v)
	{
		return Double.isNaN(v.x) || Double.isNaN(v.y);
	}
	
	/**
	 * �x�N�g���̊e�v�f�����܂��͕��̖�����łȂ����񐔂łȂ����������܂��B
	 * @param v ���肳���x�N�g��
	 * @return �e�v�f�����܂��͕��̖�����łȂ����񐔂łȂ��ꍇ�� <code>true</code>�A�����łȂ��ꍇ�� <code>false</code>
	 */
	public static boolean isValid(VecD v)
	{
		return !isInf(v) && !isNaN(v);
	}
}
