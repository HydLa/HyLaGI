package test.sakuplot.exp;

import java.util.HashSet;
import java.util.Set;

import test.sakuplot.exp.parser.Lexer;
import test.sakuplot.exp.parser.Parser;
import test.sakuplot.exp.parser.ParserException;

public abstract class Expression
{
	/**
	 * <p>
	 * ��������p�[�X���Đ����I�u�W�F�N�g�𐶐����܂��B
	 * �����̍\����͂Ɏ��s�����ꍇ�� <code>null</code> ��Ԃ��܂��B
	 * </p>
	 * @param text ����������
	 * @return �����I�u�W�F�N�g
	 */
	public static Expression create(String text)
	{
		Parser parser = new Parser(new Lexer(text));
		try
		{
			return new ExpRoot(text, parser.parseRoot());
		}
		catch (ParserException e)
		{
			e.printStackTrace();
		}
		return null;
	}

	/**
	 * <p>
	 * �w�肳�ꂽ������p���Ă��̐����̒l���v�Z���܂��B
	 * �������Œ�`����Ă��Ȃ��ϐ���֐����Q�Ƃ��ꂽ�ꍇ�� <code>NaN</code> ��Ԃ��܂��B
	 * </p>
	 * @param env ����
	 * @return �v�Z���ʂ̒l
	 */
	public abstract double getValue(ExpressionEnv env);

	/**
	 * ���̐������ŎQ�Ƃ����ϐ����̃Z�b�g��Ԃ��܂��B
	 * @return ���̐������Q�Ƃ���ϐ����̃Z�b�g
	 */
	public Set<String> getVariables()
	{
		return new HashSet<String>();
	}

	/**
	 * ���̐����̕�����\����Ԃ��܂��B
	 * @return ���̐����̕�����\��
	 */
	public abstract String toString();
}
