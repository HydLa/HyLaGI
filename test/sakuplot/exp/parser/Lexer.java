package test.sakuplot.exp.parser;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Lexer
{
	private static Map<Character, TokenType> opTypes = new HashMap<Character, TokenType>();

	static
	{
		opTypes.put('(', TokenType.LPAR);
		opTypes.put(')', TokenType.RPAR);
		opTypes.put('+', TokenType.PLUS);
		opTypes.put('-', TokenType.MINUS);
		opTypes.put('*', TokenType.MULT);
		opTypes.put('/', TokenType.DIV);
		opTypes.put('^', TokenType.HAT);
	}

	private String _text;
	private char[] cs;
	private int p;
	private List<Token> tokens = new ArrayList<Token>();

	public Lexer(String text)
	{
		_text = text;
		cs = text.toCharArray();
		reset();
	}

	public String getSourceText()
	{
		return _text;
	}

	public void reset()
	{
		p = 0;
	}

	@Deprecated
	public void lex() throws LexerException
	{
		while (!end())
		{
			skipWhiteSpaces();

			if (end())
			{
				break;
			}

			Token token;
			char c = peek();

			if (Character.isDigit(c))
			{
				token = lexNumber();
			}
			else if (Character.isLetter(c))
			{
				token = lexName();
			}
			else
			{
				c = get();
				if (!opTypes.containsKey(c))
				{
					throw new LexerException("Illegal character '" + c + "' at column " + p + ".");
				}
				token = new Token(Character.toString(c), opTypes.get(c));
			}
			tokens.add(token);
		}
		tokens.add(new Token("End", TokenType.END));
	}

	public Token nextToken() throws LexerException
	{
		skipWhiteSpaces();

		if (end())
		{
			return new Token("End", TokenType.END);
		}

		char c = peek();
		if (Character.isDigit(c))
		{
			return lexNumber();
		}
		else if (Character.isLetter(c))
		{
			return lexName();
		}
		else
		{
			c = get();
			if (!opTypes.containsKey(c))
			{
				throw new LexerException("Illegal character '" + c + "' at column " + p + ".");
			}
			return new Token(Character.toString(c), opTypes.get(c));
		}
	}

	public List<Token> getTokens()
	{
		return Collections.unmodifiableList(tokens);
	}

	private void skipWhiteSpaces()
	{
		while (!end() && Character.isWhitespace(peek())) get();
	}

	/**
	 * 整数のみの字句解析を行うレガシーなメソッド。
	 * @deprecated
	 */
	@SuppressWarnings("unused")
	@Deprecated
	private Token lexInteger()
	{
		StringBuffer buf = new StringBuffer();

		while (Character.isDigit(peek()))
		{
			buf.append(get());
		}

		return new Token(buf.toString(), TokenType.INTEGER);
	}

	/**
	 * 整数、浮動小数点数の定数として解釈される字句を解析する。
	 */
	private Token lexNumber()
	{
		StringBuffer buf = new StringBuffer();

		int state = 0;
		while (state != -1)
		{
			char c = peek();

			switch (state)
			{
			case 0:
				if (Character.isDigit(c))
				{
					buf.append(get());
					state = 1;
				}
				else if (c == '.')
				{
					buf.append(get());
					state = 2;
				}
				else
				{
					state = -1;
				}
				break;
			case 1:
				if (Character.isDigit(c))
				{
					buf.append(get());
				}
				else if (c == '.')
				{
					buf.append(get());
					state = 3;
				}
				else
				{
					return new Token(buf.toString(), TokenType.INTEGER);
				}
				break;
			case 2:
				if (Character.isDigit(c))
				{
					buf.append(get());
					state = 3;
				}
				else
				{
					state = -1;
				}
				break;
			case 3:
				if (Character.isDigit(c))
				{
					buf.append(get());
				}
				else if (c == 'e' || c == 'E')
				{
					buf.append(get());
					state = 4;
				}
				else
				{
					return new Token(buf.toString(), TokenType.FLOAT);
				}
				break;
			case 4:
				if (Character.isDigit(c))
				{
					buf.append(get());
					state = 6;
				}
				else if (c == '-' || c == '+')
				{
					buf.append(get());
					state = 5;
				}
				else
				{
					state = -1;
				}
				break;
			case 5:
				if (Character.isDigit(c))
				{
					buf.append(get());
					state = 6;
				}
				else
				{
					state = -1;
				}
				break;
			case 6:
				if (Character.isDigit(c))
				{
					buf.append(get());
				}
				else
				{
					return new Token(buf.toString(), TokenType.FLOAT);
				}
				break;
			default:
				break;
			}
		}
		return new Token(buf.toString(), TokenType.UNDEF);
	}

	private Token lexName()
	{
		StringBuffer buf = new StringBuffer();

		char c = peek();
		while (Character.isLetterOrDigit(c))
		{
			buf.append(get());
			c = peek();
		}
		return new Token(buf.toString(), TokenType.NAME);
	}

	private boolean end()
	{
		return p == cs.length;
	}

	private char peek()
	{
		return !end() ? cs[p] : 0;
	}

	private char get()
	{
		return !end() ? cs[p++] : 0;
	}
}
