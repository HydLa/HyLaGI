package test.sakuplot.model;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class IntervalPhase
{
	private double _min_t;
	private double _max_t;
	private Map<String, String> _funcs = new HashMap<String, String>();
	
	/**
	 * �C���^�[�o���t�F�[�Y���쐬���܂��B
	 */
	public IntervalPhase()
	{
		this(0, 0);
	}
	
	/**
	 * ��`����w�肵�ăC���^�[�o���t�F�[�Y���쐬���܂��B
	 * @param min_t ��`��̉����l
	 * @param max_t ��`��̏���l
	 */
	public IntervalPhase(double min_t, double max_t)
	{
		_min_t = min_t;
		_max_t = max_t;
	}
	
	public double getMinT()
	{
		return _min_t;
	}
	
	public void setMinT(double min_t)
	{
		_min_t = min_t;
	}
	
	public double getMaxT()
	{
		return _max_t;
	}
	
	public void setMaxT(double max_t)
	{
		_max_t = max_t;
	}
	
	/**
	 * ���̃C���^�[�o���t�F�[�Y�̒�`���ݒ肵�܂��B
	 * @param min_t ��`��̉����l
	 * @param max_t ��`��̏���l
	 */
	public void setRange(double min_t, double max_t)
	{
		_min_t = min_t;
		_max_t = max_t;
	}
	
	/**
	 * �ϐ� name ���`���鎞�Ԋ֐���ݒ肵�܂��B
	 * @param name �ϐ���
	 * @param expr ����������
	 * @throws IllegalArgumentException �����̂����ꂩ�� <code>NULL</code> �܂��͋�̕�����ł���ꍇ
	 */
	public void setFunction(String name, String expr) throws IllegalArgumentException
	{
		if (name == null || name.isEmpty() || expr == null || expr.isEmpty())
			throw new IllegalArgumentException("Parameters cannot be null or empty.");
		
		_funcs.put(name, expr);
	}
	
	/**
	 * �ϐ� <code>name</code> ���`���鎞�Ԋ֐����擾���܂��B
	 * @param name �ϐ���
	 * @return �֐�������
	 * @throws IllegalArgumentException ������ <code>null</code> �܂��͋�̕�����ł���ꍇ
	 * @throws RuntimeException <code>name</code> �Ŏw�肳�ꂽ�L�[�����݂��Ȃ��ꍇ
	 */
	public String getFunction(String name) throws IllegalArgumentException, RuntimeException
	{
		if (name == null || name.isEmpty())
			throw new IllegalArgumentException("Parameter must not be null or empty.");
		
		if (!_funcs.containsKey(name))
			throw new RuntimeException("Key [" + name + "] was not found.");
		
		return _funcs.get(name);
	}
	
	@Deprecated
	public Map<String, String> getDefinedFunctions()
	{
		return Collections.unmodifiableMap(_funcs);
	}
	
	/**
	 * ���̃C���^�[�o���t�F�[�Y�Œ�`����Ă���ϐ��̃Z�b�g��Ԃ��܂��B
	 * @return �ϐ����̃Z�b�g
	 */
	public Set<String> getFunctionSet()
	{
		return Collections.unmodifiableSet(_funcs.keySet());
	}
	
	/**
	 * �I�u�W�F�N�g�̕�����\����Ԃ��܂��B
	 * @return ���̃I�u�W�F�N�g�̕�����\��
	 */
	public String toString()
	{
		return String.format("t:[%f,%f]", _min_t, _max_t) + _funcs.toString();
	}
}
