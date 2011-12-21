package test.funcgraph;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import test.Env;
import test.FrontEnd;
import test.editor.EditorButtonPanel;
import test.sakuplot.gui.MainFrame;
import test.sakuplot.gui.PlotPanel;
import test.sakuplot.model.CompoundGraph;
import test.sakuplot.model.SimulationCase;
import test.sakuplot.model.SimulationCaseParser;

public class FuncGraphRunner implements Runnable{

	private FileToString fts = new FileToString();
//	MainFrame frame = new MainFrame();
//	FuncGraphPanel funcgraphpanel = new FuncGraphPanel();
//	PlotPanel panel;
	String filename;
	private static String x_axis;
	private static String y_axis;
	private static boolean ErrFlag=false;

	public FuncGraphRunner(String filename)
	{
		this.filename = filename;
	}

	@SuppressWarnings("static-access")
	public FuncGraphRunner(){
		this.x_axis=Env.get("FUNC_X_AXIS");
		this.y_axis=Env.get("FUNC_Y_AXIS");
	}

	public void stop(){
		this.stop();
	}

	public static String getX_axis(){
		return x_axis;
	}

	@SuppressWarnings("static-access")
	public void setX_axis(String x_axis){
		this.x_axis=x_axis;
	}

	public static String getY_axis(){
		return y_axis;
	}

	@SuppressWarnings("static-access")
	public void setY_axis(String y_axis){
		this.y_axis=y_axis;
	}

	public void setFilename(String filename) {
		this.filename=filename;
	}

	public String getFilename(){
		return filename;
	}


	public static boolean isErrFlag() {
		return ErrFlag;
	}

	public static void setErrFlag(boolean errflag) {
		ErrFlag=errflag;
	}


	public void run() {
		String str;

		try {
			SimulationCaseParser scp = new SimulationCaseParser();
			str = fts.fileToString(new File(filename));
			x_axis = Env.get("FUNC_X_AXIS");
			y_axis = Env.get("FUNC_Y_AXIS");
			if (!str.isEmpty()){
				List<SimulationCase> cases = scp.parseCases(str);
				List<CompoundGraph> graphList = new ArrayList<CompoundGraph>();

				for (SimulationCase c : cases)
				{
					try{
						graphList.add(CompoundGraph.createFromSimulation(c, x_axis, y_axis));
					}catch(Exception e){
						setErrFlag(true);
					}
				}
				if(!isErrFlag()){
					FrontEnd.println("FuncGraphSuccess ");
				}else{
					FrontEnd.errPrintln("axis err\nFuncGraphFaild");
				}
				FuncGraphPanel.frame.doPlot(graphList);
			}
		} catch (IOException e) {
			// TODO é©ìÆê∂ê¨Ç≥ÇÍÇΩ catch ÉuÉçÉbÉN
			e.printStackTrace();
		}
	}
}
