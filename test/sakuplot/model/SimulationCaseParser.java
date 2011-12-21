package test.sakuplot.model;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import test.sakuplot.exp.Expression;
import test.sakuplot.exp.ExpressionEnv;

/**
 * ���̓e�L�X�g�𐳋K�\���ɂ���ĉ�͂��A�V�~�����[�V�������ʃ��f�����\�z���܂��B
 */
public class SimulationCaseParser
{
	private static final Pattern PATTERN_ONE_CASE = Pattern.compile("(case(\\d+))(.*?)end\\2", Pattern.DOTALL);
	private static final Pattern PATTERN_IP = Pattern.compile("IP\\d+\\s+time:(.+?)->(.+?),(.+),");

	/**
	 * �e�L�X�g����͂��A�V�~�����[�V�����P�[�X�̃��X�g���쐬���܂��B
	 * ��������͂ł��Ȃ����̂ɑ΂��Ă̓G���[���o�͂��܂����A��͏����͑��s���܂��B
	 * @param text �V�~�����[�V�������ʂ��L�q���ꂽ�e�L�X�g
	 * @return �V�~�����[�V�����P�[�X�̃��X�g
	 */
	public List<SimulationCase> parseCases(String text)
	{
		List<SimulationCase> cases = new ArrayList<SimulationCase>();
		ExpressionEnv env = new ExpressionEnv();

		Matcher m = PATTERN_ONE_CASE.matcher(text);
		while (m.find())
		{
			String name = m.group(1);
			String value = m.group(3);

			SimulationCase simcase = new SimulationCase(name);

			Matcher m_ip = PATTERN_IP.matcher(value);
			while (m_ip.find())
			{
				Expression minExp = Expression.create(m_ip.group(1));
				Expression maxExp = Expression.create(m_ip.group(2));

				if (minExp == null || maxExp == null)
				{
					continue;
				}

				double min = minExp.getValue(env);
				double max = maxExp.getValue(env);

				if (Double.isNaN(min) || Double.isNaN(max))
				{
					continue;
				}

				String funcs = m_ip.group(3);

				IntervalPhase ip = new IntervalPhase();
				ip.setRange(min, max);
				ip.setFunction("time", "t");

				for (String pair : funcs.split(","))
				{
					String[] keyvalue = pair.split(":");
					ip.setFunction(keyvalue[0], keyvalue[1]);
				}

				simcase.addIntervalPhase(ip);
			}

			cases.add(simcase);
		}
		return cases;
	}
}
