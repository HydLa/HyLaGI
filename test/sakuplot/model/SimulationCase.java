package test.sakuplot.model;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * �V�~�����[�V�������ʂ�\�����܂��B
 */
public class SimulationCase
{
	private String _name;
	private List<IntervalPhase> _ipList = new ArrayList<IntervalPhase>();
	
	/**
	 * �V�~�����[�V�������ʂ��쐬���܂��B
	 * @param name - ���̃V�~�����[�V�������ʂɕt���閼�O
	 * @throws IllegalArgumentException ������ <code>null</code> �̏ꍇ
	 */
	public SimulationCase(String name) throws IllegalArgumentException
	{
		if (name == null)
			throw new IllegalArgumentException("'name' must not be null.");
		
		_name = name;
	}
	
	/**
	 * ���̃V�~�����[�V�������ʂ̖��O���擾���܂��B
	 * @return ���O������
	 */
	public String getName()
	{
		return _name;
	}
	
	/**
	 * �C���^�[�o���t�F�[�Y��ǉ����܂��B
	 * @param ip - �ǉ�����C���^�[�o���t�F�[�Y
	 * @throws IllegalArgumentException ������ <code>null</code> �̏ꍇ
	 */
	public void addIntervalPhase(IntervalPhase ip) throws IllegalArgumentException
	{
		if (ip == null)
			throw new IllegalArgumentException("'ip' must not be null.");
		
		_ipList.add(ip);
	}
	
	/**
	 * ���̃V�~�����[�V�����P�[�X�Ɋ܂܂��ϐ����̃Z�b�g��Ԃ��܂��B
	 * @return �ϐ����̃Z�b�g
	 */
	public Set<String> getFunctionSet()
	{
		if (_ipList.size() != 0)
		{
			return _ipList.get(0).getFunctionSet();
		}
		return new HashSet<String>();
	}
	
	/**
	 * �V�~�����[�V�������ʂɊ܂܂��C���^�[�o���t�F�[�Y�̃��X�g���擾���܂��B
	 * @return �C���^�[�o���t�F�[�Y�̓ǂݎ���p���X�g
	 */
	public List<IntervalPhase> getIntervalPhases()
	{
		return Collections.unmodifiableList(_ipList);
	}
}
