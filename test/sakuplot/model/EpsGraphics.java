package test.sakuplot.model;

/**
 * PostScript�O���t�B�b�N�X�f�[�^���J�v�Z�������܂��B
 */
public class EpsGraphics
{
	private static final String ENDL = "\n";
	
	private int _width;
	private int _height;
	private StringBuilder _value = new StringBuilder();
	
	/**
	 * ���A�������w�肵��EPS�O���t�B�b�N���쐬���܂��B
	 * @param width �C���[�W�̕�
	 * @param height �C���[�W�̍���
	 */
	public EpsGraphics(int width, int height)
	{
		_width = width;
		_height = height;
		clear();
	}
	
	/**
	 * �C���[�W�̕����擾���܂��B
	 * @return ��
	 */
	public int getWidth()
	{
		return _width;
	}
	
	/**
	 * �C���[�W�̕���ݒ肵�܂��B
	 * @param width ��
	 */
	public void setWidth(int width)
	{
		_width = width;
	}
	
	/**
	 * �C���[�W�̍������擾���܂��B
	 * @return ����
	 */
	public int getHeight()
	{
		return _height;
	}
	
	/**
	 * �C���[�W�̍�����ݒ肵�܂��B
	 * @param height ����
	 */
	public void setHeight(int height)
	{
		_height = height;
	}
	
	/**
	 * �`�悳�ꂽ���e���������A�C���[�W�����������܂��B
	 */
	public void clear()
	{
		_value.setLength(0);
	}
	
	/**
	 * �y�����w����W�ֈړ����܂��B
	 * @param x �y����x���W
	 * @param y �y����y���W
	 */
	public void moveTo(double x, double y)
	{
		appendLine(String.format("%.2f %.2f M", x, y));
	}
	
	/**
	 * ���݂̃y���̈ʒu����w����W�֐��������܂��B
	 * @param x �I�_��x���W
	 * @param y �I�_��y���W
	 */
	public void lineTo(double x, double y)
	{
		appendLine(String.format("%.2f %.2f L", x, y));
	}
	
	/**
	 * ���܂łɎw���������e�����ۂɕ`�悵�܂��B
	 */
	public void stroke()
	{
		appendLine("stroke");
	}
	
	/**
	 * ������ݒ肵�܂��B
	 * @param width ���̕�
	 */
	public void setLineWidth(double width)
	{
		appendLine(String.format("%.2f setlinewidth", width));
	}
	
	/**
	 * �_���E�j���p�^�[����ݒ肵�܂��B
	 * @param pattern �`��s�N�Z�����A�󔒃s�N�Z���������݂Ɋi�[�����z��
	 */
	public void setDash(int[] pattern)
	{
		setDash(pattern, 0);
	}
	
	/**
	 * �I�t�Z�b�g���w�肵�ē_���E�j���p�^�[����ݒ肵�܂��B
	 * @param pattern �`��s�N�Z�����A�󔒃s�N�Z���������݂Ɋi�[�����z��
	 * @param offset �p�^�[���z��̂ǂ̗v�f���n�_�Ƃ��邩�̃I�t�Z�b�g
	 */
	public void setDash(int[] pattern, int offset)
	{
		String arrayStr = "";
		for (int p : pattern) arrayStr += " " + p;
		appendLine(String.format("[%s] %d setdash", arrayStr, offset));
	}
	
	/**
	 * �_���E�j���p�^�[����j�����y���̏�Ԃ������ɂ��܂��B
	 */
	public void setSolid()
	{
		setDash(new int[] { });
	}
	
	/**
	 * �`����e���܂�EPS�`���̕�������擾���܂��B
	 * @return EPS�e�L�X�g
	 */
	public String getContents()
	{
		StringBuilder contents = new StringBuilder();
		
		contents.append("%!PS-Adobe-3.0 EPSF-3.0" + ENDL);
		contents.append("%%BoundingBox: 0 0 " + _width + " " + _height + ENDL);
		contents.append("/gdict 120 dict def" + ENDL);
		contents.append("gdict begin" + ENDL);
		contents.append("gsave" + ENDL);
		contents.append("/M {moveto} def" + ENDL);
		contents.append("/L {lineto} def" + ENDL);
		contents.append(_value);
		contents.append("showpage" + ENDL);
		contents.append("grestore");
		
		return contents.toString();
	}
	
	private void appendLine(String str)
	{
		_value.append(str);
		_value.append(ENDL);
	}
}
