package test.sakuplot.exp.parser;

import test.sakuplot.exp.ExpAdd;
import test.sakuplot.exp.ExpDiv;
import test.sakuplot.exp.ExpFloat;
import test.sakuplot.exp.ExpFunc;
import test.sakuplot.exp.ExpInt;
import test.sakuplot.exp.ExpMinus;
import test.sakuplot.exp.ExpMul;
import test.sakuplot.exp.ExpPow;
import test.sakuplot.exp.ExpSub;
import test.sakuplot.exp.ExpVar;
import test.sakuplot.exp.Expression;

public class Parser
{
	private String _text;
	private Lexer _lexer;
	private Token token;

	public Parser(Lexer lexer)
	{
		_lexer = lexer;
		_text = lexer.getSourceText();
	}

	public Expression parseRoot() throws ParserException
	{
		nextToken();

		Expression exp = expr();

		if (token.getType() == TokenType.END)
		{
			return exp;
		}
		else
		{
			throw new ParserException(_text + ": 数式が正しく終了していません。（字句 '" + token.getText() + "' の辺り）");
		}
	}

	private Expression expr() throws ParserException
	{
		Expression e1 = term();

		TokenType op = token.getType();
		while (op == TokenType.PLUS || op == TokenType.MINUS)
		{
			nextToken();
			Expression e2 = term();

			if (op == TokenType.PLUS)
			{
				e1 = new ExpAdd(e1, e2);
			}
			else
			{
				e1 = new ExpSub(e1, e2);
			}
			op = token.getType();
		}
		return e1;
	}

	private Expression term() throws ParserException
	{
		Expression e1 = power();

		TokenType op = token.getType();
		while (op == TokenType.MULT || op == TokenType.DIV)
		{
			nextToken();
			Expression e2 = power();

			if (op == TokenType.MULT)
			{
				e1 = new ExpMul(e1, e2);
			}
			else
			{
				e1 = new ExpDiv(e1, e2);
			}
			op = token.getType();
		}
		return e1;
	}

	private Expression power() throws ParserException
	{
		Expression e1 = factor();

		if (token.getType() == TokenType.HAT)
		{
			nextToken();
			Expression e2 = power();
			e1 = new ExpPow(e1, e2);
		}
		return e1;
	}

	private Expression factor() throws ParserException
	{
		Expression exp = null;

		switch (token.getType())
		{
		case INTEGER:
			exp = new ExpInt(token.getText());
			nextToken();
			break;
		case FLOAT:
			exp = new ExpFloat(token.getText());
			nextToken();
			break;
		case MINUS:
			nextToken();
			exp = new ExpMinus(factor());
			break;
		case NAME:
			Token id = token;
			nextToken();
			if (token.getType() == TokenType.LPAR) // function
			{
				nextToken();
				exp = new ExpFunc(id.getText(), expr());
				if (token.getType() == TokenType.RPAR)
				{
					nextToken();
				}
				else
				{
					throw new ParserException(_text + ": 関数呼び出しの括弧が閉じられていません。");
				}
			}
			else // variable
			{
				exp = new ExpVar(id.getText());
			}
			break;
		case LPAR:
			nextToken();
			exp = expr();
			if (token.getType() == TokenType.RPAR)
			{
				nextToken();
			}
			else
			{
				throw new ParserException(_text + ": 数式の括弧が閉じられていません。");
			}
			break;
		default:
			throw new ParserException(_text + ": 字句 '" + token.getText() + "' の位置が不正です。");
		}
		return exp;
	}

	private void nextToken() throws ParserException
	{
		try
		{
			token = _lexer.nextToken();
		}
		catch (LexerException e)
		{
			throw new ParserException(_text + ": 字句解析で失敗したため、構文解析を続行できませんでした。", e);
		}
	}
}
