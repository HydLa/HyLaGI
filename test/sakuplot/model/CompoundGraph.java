package test.sakuplot.model;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * �������̃O���t��g�ݍ��킹�č���镡���O���t��\���܂��B
 * �R���|�W�b�g�p�^�[���ɂ��邱�Ƃ��l�����܂����A�P�[�X�� IP �Ƃ̑Ή����l���ĂƂ肠�����͂��̍\���ł��B
 */
public class CompoundGraph
{
	private String _name;
	private List<GraphModel> _graphList = null;
	
	private CompoundGraph(String name, List<GraphModel> graphList)
	{
		_name = name;
		_graphList = Collections.unmodifiableList(graphList);
	}
	
	/**
	 * ���̕����O���t�ɕt����ꂽ���O��Ԃ��܂��B
	 * @return ���O������
	 */
	public String getName()
	{
		return _name;
	}
	
	/**
	 * ���̕����O���t�Ɋ܂܂��O���t�̃��X�g��Ԃ��܂��B
	 * @return �O���t�I�u�W�F�N�g�̃��X�g
	 */
	public List<GraphModel> getGraphList()
	{
		return _graphList;
	}
	
	public String toString()
	{
		return getName();
	}
	
	/**
	 * <p>�V�~�����[�V�����P�[�X�I�u�W�F�N�g����2�̕ϐ����w�肵�ĕ����O���t�I�u�W�F�N�g���쐬���܂��B</p>
	 * <p><b>�O�����:</b> 2�̕ϐ����̓C���^�[�o���t�F�[�Y���ɒ�`����Ă��āA���̐����͓K�؂Ȍ`���ł���</p>
	 * @param simulation �V�~�����[�V�����P�[�X�I�u�W�F�N�g
	 * @param xVar X ���ɑΉ�������ϐ���
	 * @param yVar Y ���ɑΉ�������ϐ���
	 * @return �����O���t�I�u�W�F�N�g
	 * @throws RuntimeException �w�肳�ꂽ�ϐ�������`����Ă��Ȃ��ꍇ�A�܂��͂��̐����̌`�����������Ȃ��ꍇ
	 */
	public static CompoundGraph createFromSimulation(SimulationCase simulation, String xVar, String yVar) throws RuntimeException
	{
		List<GraphModel> graphList = new ArrayList<GraphModel>();
		for (IntervalPhase ip : simulation.getIntervalPhases())
		{
			graphList.add(GraphModel.createFromIP(ip, xVar, yVar));
		}
		return new CompoundGraph(simulation.getName(), graphList);
	}
}
