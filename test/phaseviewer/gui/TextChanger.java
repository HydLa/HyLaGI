package test.phaseviewer.gui;

public class TextChanger {

	static int Num;

	public String changedTooltipText(String str) {
		str = str.replace(":",": ");
		str = str.replace(",","<p>");
		return "<html>"+str+"</html>";
	}

	public String changedText(String s){
		String str = "Constraint"+Num+++"\n"+s.replace(",", "\n");
		return str;
	}

}
