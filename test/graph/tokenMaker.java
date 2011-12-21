package test.graph;

public interface tokenMaker {
    //取得したデータの格納する番号
	public int cycleNum=0;
	public int cycleNumsec=0;
	//取得したデータの格納
	public static String[] phasefst = new String[500];
	public static String[] phasesec = new String[500];
    public void Fileloader(String filename);
    //配列の添え字を食べて、該当する一つ目の配列を返す
    public boolean PhaseChecker(int num);

    //配列の添え字を食べて、該当する二つ目の配列を返す
    public boolean PhaseCheckerConst(int num);
    //treeの作成
    public void createTree(String filename);

    //ｎからｍにかけてエッジをかける
    public boolean PhasetoPhase(String n,String m);
    //nodeを食べて別のノードを返す
    public String nodeExChanger(String string);

}
