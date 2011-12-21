package test.sakuplot.exp.parser;

import test.FrontEnd;

public class ParserException extends Exception
{
	private static final long serialVersionUID = 1L;

	public ParserException()
	{
		super();
	}

	public ParserException(String message)
	{
		super(message);
		FrontEnd.errPrintln(message);
	}

	public ParserException(Throwable cause)
	{
		super(cause);
		FrontEnd.printException((Exception) cause);
	}

	public ParserException(String message, Throwable cause)
	{
		super(message, cause);
		FrontEnd.errPrintln(message);
		FrontEnd.printException((Exception) cause);
	}
}
