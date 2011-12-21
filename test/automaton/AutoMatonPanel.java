package test.automaton;

import java.awt.BorderLayout;
import javax.swing.JPanel;


@SuppressWarnings("serial")
public class AutoMatonPanel extends JPanel{
	private Automaton automaton;
	public AutoMatonPanel(){
		setLayout(new BorderLayout());
		String filename = "Nulltext.txt";//"automaton_test.txt";
		start(filename);
}

	public void restart(String filename){
		clearPanel();
		start(filename);
		repaintPanel();
	}

	private void clearPanel(){
		removeAll();
	}

	private void start(String filename){
		automaton = new Automaton(filename);
		add(automaton,BorderLayout.CENTER);
	}

	private void repaintPanel(){
		validate();
	}

}
