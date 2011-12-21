package test.sakuplot.exp.parser;

/**
 * ����̎�ނ�\���܂��B
 */
enum TokenType
{
	/** ����`�̎��� */
	UNDEF,
	
	/** �I�[���� */
	END,
	
	/** ���ۊ��� */
	LPAR,
	
	/** �E�ۊ��� */
	RPAR,
	
	/** ���Z�L�� */
	PLUS,
	
	/** ���Z�L�� */
	MINUS,
	
	/** ��Z�L�� */
	MULT,
	
	/** ���Z�L�� */
	DIV,
	
	/** �ׂ���L�� */
	HAT,
	
	/** ���� */
	INTEGER,
	
	/** ���������_�� */
	FLOAT,
	
	/** ���ʎq */
	NAME,
}
