package test.sakuplot.exp.parser;

/**
 * ����v�f��\���܂��B
 */
class Token
{
	public String _text;
	public TokenType _type;
	
	/**
	 * ���啶����Ǝ����ʂ��玚��v�f���쐬���܂��B
	 * @param text ���啶����
	 * @param type ������
	 */
	public Token(String text, TokenType type)
	{
		_text = text;
		_type = type;
	}
	
	/**
	 * ���啶������擾���܂��B
	 * @return ���啶����
	 */
	public String getText()
	{
		return _text;
	}
	
	/**
	 * �����ʂ��擾���܂��B
	 * @return ������
	 */
	public TokenType getType()
	{
		return _type;
	}
	
	/**
	 * ���̎���I�u�W�F�N�g�̕�����\����Ԃ��܂��B
	 * ���̃��\�b�h�͎�Ƀf�o�b�O�p�r�Ɏg�p����܂��B
	 * ���啶����𓾂�ɂ� {@link #getText()} ���\�b�h���g�p���Ă��������B
	 * @return ���̎���I�u�W�F�N�g�̕�����\��
	 */
	public String toString()
	{
		return _text + "<type:" + _type + ">";
	}
}
