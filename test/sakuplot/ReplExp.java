package test.sakuplot;

import java.util.Scanner;

import test.sakuplot.exp.DefaultExpressionEnv;
import test.sakuplot.exp.Expression;

/**
 * <p style="font-size:28pt;">
 * �������悤�I<br />
 * ���̃N���X�͐����p�[�T�̃e�X�g�x���`�ł���I<br />
 * ��������̋������������Ɗ������炱���Ńe�X�g���悤�I<br />
 * </p>
 */
public class ReplExp
{
	public static void main(String[] args)
	{
		Scanner scan = new Scanner(System.in);
		System.out.print("> ");
		while (scan.hasNext())
		{
			String line = scan.nextLine();
			if (line.equals("q"))
			{
				break;
			}
			Expression exp = Expression.create(line);
			if (exp != null)
			{
				System.out.println("--text: " + exp);
				System.out.println("--vars: " + exp.getVariables());
				if (exp.getVariables().size() == 0)
				{
					System.out.println("---val: " + exp.getValue(new DefaultExpressionEnv()));
				}
			}
			System.out.print("> ");
		}
	}
}
