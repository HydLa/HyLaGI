package test.graph;

public interface tokenMaker {
    //�擾�����f�[�^�̊i�[����ԍ�
	public int cycleNum=0;
	public int cycleNumsec=0;
	//�擾�����f�[�^�̊i�[
	public static String[] phasefst = new String[500];
	public static String[] phasesec = new String[500];
    public void Fileloader(String filename);
    //�z��̓Y������H�ׂāA�Y�������ڂ̔z���Ԃ�
    public boolean PhaseChecker(int num);

    //�z��̓Y������H�ׂāA�Y�������ڂ̔z���Ԃ�
    public boolean PhaseCheckerConst(int num);
    //tree�̍쐬
    public void createTree(String filename);

    //�����炍�ɂ����ăG�b�W��������
    public boolean PhasetoPhase(String n,String m);
    //node��H�ׂĕʂ̃m�[�h��Ԃ�
    public String nodeExChanger(String string);

}
