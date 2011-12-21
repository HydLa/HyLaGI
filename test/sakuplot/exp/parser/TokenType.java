package test.sakuplot.exp.parser;

/**
 * 字句の種類を表します。
 */
enum TokenType
{
	/** 未定義の字句 */
	UNDEF,
	
	/** 終端字句 */
	END,
	
	/** 左丸括弧 */
	LPAR,
	
	/** 右丸括弧 */
	RPAR,
	
	/** 加算記号 */
	PLUS,
	
	/** 減算記号 */
	MINUS,
	
	/** 乗算記号 */
	MULT,
	
	/** 除算記号 */
	DIV,
	
	/** べき乗記号 */
	HAT,
	
	/** 整数 */
	INTEGER,
	
	/** 浮動小数点数 */
	FLOAT,
	
	/** 識別子 */
	NAME,
}
