package test.graph;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.Paint;
import java.awt.Point;
import java.awt.Shape;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Point2D;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JSplitPane;
import javax.swing.JToggleButton;
import javax.swing.JTextArea;
import javax.swing.JScrollPane;
import javax.swing.ToolTipManager;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

import org.apache.commons.collections15.Factory;
import org.apache.commons.collections15.Transformer;
import org.apache.commons.collections15.functors.ConstantTransformer;

import test.Env;
import test.funcgraph.FuncGraphPanel;
import test.sakuplot.gui.PlotPanel;
import test.sakuplot.gui.PlotPopupMenu;

import edu.uci.ics.jung.algorithms.layout.KKLayout;
import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.layout.PolarPoint;
import edu.uci.ics.jung.algorithms.layout.RadialTreeLayout;
import edu.uci.ics.jung.algorithms.layout.TreeLayout;
import edu.uci.ics.jung.algorithms.layout.FRLayout;
import edu.uci.ics.jung.graph.DirectedGraph;
import edu.uci.ics.jung.graph.DirectedSparseMultigraph;
import edu.uci.ics.jung.graph.Forest;
import edu.uci.ics.jung.graph.DelegateForest;
import edu.uci.ics.jung.graph.DelegateTree;
import edu.uci.ics.jung.graph.Tree;
import edu.uci.ics.jung.visualization.GraphZoomScrollPane;
import edu.uci.ics.jung.visualization.Layer;
import edu.uci.ics.jung.visualization.VisualizationServer;
import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.CrossoverScalingControl;
import edu.uci.ics.jung.visualization.control.DefaultModalGraphMouse;
import edu.uci.ics.jung.visualization.control.GraphMouseListener;
import edu.uci.ics.jung.visualization.control.ModalGraphMouse;
import edu.uci.ics.jung.visualization.control.ScalingControl;
import edu.uci.ics.jung.visualization.decorators.EdgeShape;
import edu.uci.ics.jung.visualization.decorators.ToStringLabeller;
import edu.uci.ics.jung.visualization.layout.LayoutTransition;
import edu.uci.ics.jung.visualization.picking.PickedState;
import edu.uci.ics.jung.visualization.renderers.GradientVertexRenderer;
import edu.uci.ics.jung.visualization.util.Animator;

import edu.uci.ics.jung.visualization.decorators.PickableEdgePaintTransformer;
import edu.uci.ics.jung.visualization.decorators.PickableVertexPaintTransformer;

import edu.uci.ics.jung.graph.DirectedSparseGraph;
import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.visualization.renderers.DefaultEdgeLabelRenderer;
import edu.uci.ics.jung.visualization.renderers.DefaultVertexLabelRenderer;
//import function_graph.MainFrame;
import edu.uci.ics.jung.visualization.renderers.Renderer;




@SuppressWarnings("serial")
public class PhaseViewer extends JPanel implements tokenMaker{


	//helpの内容
	   protected String pvHelpText =
	        "<html>PhaseViewer functional detail"+
	        "<p>: shift+drag=>rotatory"+
	        "<p>: ctrl+drag=>transform+" +
	        "<p>: clicking node=>mathematical constraint indication" +
	        "<p>: moving mousewheel=>scaling" +
	        "<p>: drag=>translation"+
	        "<p>: PICKING+node click+drag=>choosed node translation"+
	        "<p>: PICKING+drag and choose+drag=>movement selected graph </html>";

       private String resetmessage =
       	"<html>Reset機能をつけるつもり"+
       	"</html>";

       Forest<String,Integer> graph;

//        Factory<DirectedGraph<String,Integer>> graphFactory = new Factory<DirectedGraph<String,Integer>>() {
//        	public DirectedGraph<String, Integer> create() {
//        		return new DirectedSparseMultigraph<String,Integer>();
//        	}
//        };

//        Factory<Tree<String,Integer>> treeFactory =	new Factory<Tree<String,Integer>> () {
//        	public Tree<String, Integer> create() {
//        		return new DelegateTree<String,Integer>(graphFactory);
//        	}
//        };

        Factory<Integer> edgeFactory = new Factory<Integer>() {
                int i=0;
                public Integer create() {
                        return i++;
                }
        };

//        Factory<String> vertexFactory = new Factory<String>() {
//        		int i=0;
//        		public String create() {
//        			return "V"+i++;
//        		}
//        };


    static ConstraintTextArea consTextArea = new ConstraintTextArea("");
    static JScrollPane scrollpane = new JScrollPane(consTextArea);
    private TreeLayout<String,Integer> treeLayout;
    private VisualizationViewer<String,Integer> vv;
    private VisualizationServer.Paintable rings;
    private RadialTreeLayout<String,Integer> radialLayout;
    private String filename;
	private boolean _drag;
	private Point _mouse;
    //取得したデータの格納する番号
	private int cycleNum=0;
	private int cycleNumsec=0;
	private static boolean OnAnotherPanel;
	//取得したデータの格納
	private static String[] phasefst = new String[1500];
	private static String[] phasesec = new String[1500];
    static int consNum=0;

    public PhaseViewer() {
    	setLayout(new BorderLayout());
        graph = new DelegateForest<String,Integer>();
        treeLayout = new TreeLayout<String,Integer>(graph);
        radialLayout = new RadialTreeLayout<String,Integer>(graph);
        vv =  new VisualizationViewer<String,Integer>(treeLayout);
/*
 * testするときはここにcreateTreeを書く
 */
//        createTree("pp_ip_output.txt");

        add(setPanelvv_controls(), BorderLayout.CENTER);
        setGraphMouseListener();
        putConstraintTooltip();
        setEdgeShape();
        setColors();
        setRightClick();
        setVisible(true);
    }

    private void setGraphMouseListener() {
    	vv.addGraphMouseListener(new PvGraphMouseListener<String>());
	}

	@SuppressWarnings("unchecked")
	private void putConstraintTooltip() {
        //ノードの上での制約の表示
        vv.getRenderContext().setVertexLabelTransformer(new ExchangePhase());
        //制約をポインタ載せたら表示されるようにする
        vv.setVertexToolTipTransformer(new ConstLabeller());
	}

	@SuppressWarnings("unchecked")
	private void setEdgeShape() {
    	vv.getRenderContext().setEdgeShapeTransformer(new EdgeShape.Line());
	}

	private void setColors() {
        setBackgroud(Color.white);
        setEdgeColor(Color.lightGray);
        setNodeColor(Color.pink , Color.green);
	}

	@SuppressWarnings("unchecked")
	private void setEdgeColor(Color edgeColor) {
    	vv.getRenderContext().setArrowFillPaintTransformer(new ConstantTransformer(edgeColor));
	}

	private void setBackgroud(Color backGround) {
    	vv.setBackground(backGround);
	}

	private JSplitPane setPanelvv_controls() {
      	JSplitPane jsp;
      	jsp = new JSplitPane(JSplitPane.VERTICAL_SPLIT,vv,setControls());
      	jsp.setOneTouchExpandable(true);
      	jsp.setResizeWeight(0.9);
		return jsp;
	}

	private JPanel setControls() {
    	//制約表示コーナー
    	scrollpane.setBorder(BorderFactory.createTitledBorder("Constraint"));
    	JPanel controls = new JPanel(new BorderLayout());
    	controls.add(setRRHMB(),BorderLayout.WEST);
    	controls.add(setScaleGrid(),BorderLayout.EAST);
    	controls.add(scrollpane,BorderLayout.CENTER);
		return controls;
	}

    private JPanel setRRHMB() {
        JPanel radial_resetbutton_help_mode_box = new JPanel(new BorderLayout());
        radial_resetbutton_help_mode_box.add(setRRHB(),BorderLayout.CENTER);
        radial_resetbutton_help_mode_box.add(setModeCbBox(),BorderLayout.SOUTH);
		return radial_resetbutton_help_mode_box;
	}

	private JPanel setModeCbBox() {
        JPanel mode_cb_box = new JPanel(new GridLayout(4,0));
        mode_cb_box.add(new JLabel("SelectMode"));
        mode_cb_box.add(setModeBox());
        mode_cb_box.add(new JLabel("LabelPosition"));
        mode_cb_box.add(setDirections(/*new JComboBox()*/));
		return mode_cb_box;
	}

	@SuppressWarnings("unchecked")
	private JComboBox setModeBox() {
        //パネル中の操作
        DefaultModalGraphMouse graphMouse = new DefaultModalGraphMouse();
        vv.setGraphMouse(graphMouse);
        //パネル中の操作アイコンの変更
        JComboBox modeBox = graphMouse.getModeComboBox();
        modeBox.addItemListener(graphMouse.getModeListener());
        graphMouse.setMode(ModalGraphMouse.Mode.TRANSFORMING);
		return modeBox;
	}

	private JPanel setScaleGrid() {
        JPanel scaleGrid = new JPanel(new GridLayout(0,1));
        scaleGrid.setBorder(BorderFactory.createTitledBorder("Scale"));
        scaleGrid.add(setPlusButton());
        scaleGrid.add(setMinusButton());
    		return scaleGrid;
    	}

    private JPanel setRRHB() {
        JPanel radial_resetbutton_help_box = new JPanel(new GridLayout(3,0));
        radial_resetbutton_help_box.add(setRadial());
        radial_resetbutton_help_box.add(setStrButton("Reset",getResetMessage(),"Resetmessage"));
        radial_resetbutton_help_box.add(setStrButton("Help",getHelpText(),"help"));
		return radial_resetbutton_help_box;
	}

    protected String getResetMessage() {
    	return resetmessage;
	}

	protected String getHelpText() {
		return pvHelpText;
	}

	//色の決定メソッド
    private void setNodeColor(Color normalColor,Color pickedColor){
        //色、いろいろ グラデーションをつけます
        vv.getRenderer().setVertexRenderer(
                new GradientVertexRenderer<String,Integer>(
                                Color.white, normalColor,// Color.pink,
                                Color.white, pickedColor,//Color.green,
                                vv.getPickedVertexState(),
                                false));
        //色、いろいろその２ 塗りつぶすパターン
//        PickedState<String> ps = vv.getPickedVertexState();
//        PickedState<Integer> pes = vv.getPickedEdgeState();
//        vv.getRenderContext().setVertexFillPaintTransformer(new PickableVertexPaintTransformer<String>(ps, Color.pink, Color.green));
//        vv.getRenderContext().setEdgeDrawPaintTransformer(new PickableEdgePaintTransformer<Integer>(pes, Color.black, Color.cyan));
    }


    public void Fileloader(String filename){
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filename));

			String line;
			int element_num;
			while((line = reader.readLine())!=null){
				String[] block = line.trim().split("\\s+");
				 element_num = block.length;
				 phasefst[cycleNum] = block[0];
				 //
				 	if(element_num>1) {
				 		phasesec[cycleNumsec]=block[1];
				 		cycleNumsec++;
					 }else{
						 phasesec[cycleNumsec]="";
						 cycleNumsec++;
					 }
				 	System.out.println("phasefst["+cycleNum+"]: "+phasefst[cycleNum]+" phasesec["+cycleNum+"]: "+phasesec[cycleNum]);
				 //
				 cycleNum++;
			}//whileの終わり

		reader.close();
		} catch (FileNotFoundException e) {
			System.out.println(filename + "が見つかりません");
			String sPath = (new File(".")).getAbsoluteFile().getParentFile().toString();
			System.out.println("現在のディレクトリは" + sPath + "です");
		} catch (IOException e) {
			System.out.println(e);
		}//tryの終わり
    }

    //配列の添え字を食べて、該当する一つ目の配列を返す
    public boolean PhaseChecker(int num){
    	//indexOfは-1で含まれない
    	if(phasefst[num].indexOf("case")==0 || phasefst[num].indexOf("end")==0){return false;}
    	else if(phasefst[num].indexOf("case")==-1 && phasefst[num+1].indexOf("end")==-1 ) {
    		return PhasetoPhase(phasefst[num],phasefst[num+1]);
    	}else {
    		return false;
    	}
    }

    //配列の添え字を食べて、該当する二つ目の配列を返す
    public boolean PhaseCheckerConst(int num){
    	//indexOfは-1で含まれない
    	if(phasefst[num].indexOf("case")==0 || phasefst[num].indexOf("end")==0){return false;}
    	else if(phasefst[num].indexOf("case")==-1 && phasefst[num+1].indexOf("end")==-1 ) {
    		return PhasetoPhase(phasesec[num],phasesec[num+1]);
    	}else {
    		return false;
    	}
    }

    public void createTree(String filename) {
    	clearPanel();

    	Fileloader(filename);
    	for(int k=0;k<cycleNumsec-1;k++){
    		PhaseCheckerConst(k);
    	}
    	radialLayout = new RadialTreeLayout<String,Integer>(graph);
        treeLayout = new TreeLayout<String,Integer>(graph);
        rings = new Rings();
        vv.setGraphLayout(treeLayout);
        vv.repaint();
    }

    //ｎからｍにかけてエッジをかける
    public boolean PhasetoPhase(String n,String m){
    	return graph.addEdge(edgeFactory.create(),n,m);
    }


    public String nodeExChanger(String string){
    	String node="";
    	for(int k=0;k<cycleNum;k++){
    		if(string == phasesec[k]){
    			node = phasefst[k];
    		}
    	}
    	return node;
    }


    private static String lineFeeder(String s){
    	String str = "Constraint"+consNum+++"\n"+s.replace(",", "\n");
		return str;
    }

    //tooltipの形動時間.なんか全体的になってしまっている
   	public void init(){
		ToolTipManager tip = ToolTipManager.sharedInstance();
		tip.setInitialDelay(10);
		tip.setDismissDelay(100000);
	}

   	public void setFileName(String filename){
   		this.filename=filename;
   	}

	public void restart(){
			start();
	}


	private void clearPanel(){
//		for(int i=0;i<phasefst.length;i++){
//		phasefst[i]=null;
//		phasesec[i]=null;
//		}
		repaint();
	}

	public static void setOnAnotherPanel(boolean b){
		OnAnotherPanel = b?true:false;
	}

	public static boolean isOnAnotherPanel(){
		return OnAnotherPanel;
	}

	private void start(){
		if(OnAnotherPanel){
			CreateNewFrame cNF = new CreateNewFrame();
			cNF.createTree(filename);
		}else{
			createTree(filename);
		}

	}

	//ラベルの表示位置
	private JComboBox setDirections(){
		JComboBox cb = new JComboBox();
        cb.addItem(Renderer.VertexLabel.Position.N);
        cb.addItem(Renderer.VertexLabel.Position.NE);
        cb.addItem(Renderer.VertexLabel.Position.E);
        cb.addItem(Renderer.VertexLabel.Position.SE);
        cb.addItem(Renderer.VertexLabel.Position.S);
        cb.addItem(Renderer.VertexLabel.Position.SW);
        cb.addItem(Renderer.VertexLabel.Position.W);
        cb.addItem(Renderer.VertexLabel.Position.NW);
        cb.addItem(Renderer.VertexLabel.Position.CNTR);
        cb.addItem(Renderer.VertexLabel.Position.AUTO);
        cb.addItemListener(new ItemListener() {
                        public void itemStateChanged(ItemEvent e) {
                                Renderer.VertexLabel.Position position =
                                        (Renderer.VertexLabel.Position)e.getItem();
                                vv.getRenderer().getVertexLabelRenderer().setPosition(position);
                                vv.repaint();
                        }});
        cb.setSelectedItem(Renderer.VertexLabel.Position.SE);
        return cb;
    }

    private JButton setStrButton(String button,final String message,final String title){
    	JButton jb = new JButton(button);
        jb.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
            	//resetbuttonは中身が空です
            	JOptionPane.showMessageDialog((JComponent)e.getSource(), message, title, JOptionPane.PLAIN_MESSAGE);
            }});
        return jb;
    }

    private JButton setPlusButton(){
    	final ScalingControl scaler = new CrossoverScalingControl();
        JButton plus = new JButton("+");
        plus.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                scaler.scale(vv, 1.1f, vv.getCenter());
            }
        });
    	return plus;
    }

    private JButton setMinusButton(){
    	final ScalingControl scaler = new CrossoverScalingControl();
    	JButton minus = new JButton("-");
        minus.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                scaler.scale(vv, 1/1.1f, vv.getCenter());
            }
        });
    	return minus;
    }

    private JToggleButton setRadial(){
//        rings = new Rings();
        JToggleButton radial = new JToggleButton("Radial");
        radial.addItemListener(new ItemListener() {

                        	public void itemStateChanged(ItemEvent e) {
                                if(e.getStateChange() == ItemEvent.SELECTED) {
                                        LayoutTransition<String,Integer> lt =
                                                new LayoutTransition<String,Integer>(vv, treeLayout, radialLayout);
                                        Animator animator = new Animator(lt);
                                        animator.start();
                                        vv.getRenderContext().getMultiLayerTransformer().setToIdentity();
                                        vv.addPreRenderPaintable(rings);
                                } else {
                                        LayoutTransition<String,Integer> lt =
                                                new LayoutTransition<String,Integer>(vv, radialLayout, treeLayout);
                                        Animator animator = new Animator(lt);
                                        animator.start();
                                        vv.getRenderContext().getMultiLayerTransformer().setToIdentity();
                                        vv.removePreRenderPaintable(rings);
                                }
                                vv.repaint();
                        	}
        				}
        );
    	return radial;
    }

    private void setRightClick(){
        vv.addMouseListener(new MouseAdapter()
		{
			public void mousePressed(MouseEvent e)
			{
				if (e.isPopupTrigger())
				{
					PopupMenu popup = new PopupMenu(PhaseViewer.this);
					popup.show(PhaseViewer.this, e.getX(), e.getY());
				}
				else if (e.getButton() == MouseEvent.BUTTON1)
				{
					_mouse = e.getPoint();
					_drag = true;
				}
			}

			public void mouseReleased(MouseEvent e)
			{
				if (e.isPopupTrigger())
				{
					PopupMenu popup = new PopupMenu(PhaseViewer.this);
					popup.show(PhaseViewer.this, e.getX(), e.getY());
				}
				_drag = false;
			}
		});
    }

    //radial操作のときの円を表示するクラス
    private class Rings implements VisualizationServer.Paintable {

        Collection<Double> depths;

        public Rings() {
                depths = getDepths();
        }

        private Collection<Double> getDepths() {
                Set<Double> depths = new HashSet<Double>();
                Map<String,PolarPoint> polarLocations = radialLayout.getPolarLocations();
                for(String v : graph.getVertices()) {
                        PolarPoint pp = polarLocations.get(v);
                        depths.add(pp.getRadius());
                }
                return depths;
        }

                public void paint(Graphics g) {
                        g.setColor(Color.lightGray);

                        Graphics2D g2d = (Graphics2D)g;
                        Point2D center = radialLayout.getCenter();

                        Ellipse2D ellipse = new Ellipse2D.Double();
                        for(double d : depths) {
                                ellipse.setFrameFromDiagonal(center.getX()-d, center.getY()-d,
                                                center.getX()+d, center.getY()+d);
                                Shape shape = vv.getRenderContext().getMultiLayerTransformer().getTransformer(Layer.LAYOUT).transform(ellipse);
                                g2d.draw(shape);
                        }
                }

                public boolean useTransform() {
                        return true;
                }
    }

	private class CreateNewFrame {
		PhaseViewer tld = new PhaseViewer();
		JFrame frame = new JFrame();
		CreateNewFrame(){
			int height = Env.getInt("WINDOW_HEIGHT")*8/10;
			int width = Env.getInt("WINDOW_WIDTH")*8/10;
			frame.setTitle("PhaseViewer");
			frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
			frame.setSize(width, height);
			frame.add(tld, BorderLayout.CENTER);
			frame.setVisible(true);
		}

		void createTree(String filename){
			tld.createTree(filename);
		}
	}

    //マウスのクリックに関する操作クラス
    private static class PvGraphMouseListener<V> implements GraphMouseListener<V> {
    	JScrollBar jscrollbar = new JScrollBar();
        public void graphClicked(V v, MouseEvent me) {
        	//appendに画面が追いつくかどうか。trueならば追いつく
        	if(true){
            jscrollbar = scrollpane.getVerticalScrollBar();
            jscrollbar.setValue(jscrollbar.getMaximum());
        	}
        	String str = lineFeeder((String) v);
        	consTextArea.println(str);
        }
        public void graphPressed(V v, MouseEvent me) {}
        public void graphReleased(V v, MouseEvent me) {}
    }

	//PP/IPのラベルの表示の型を変更する
    class ExchangePhase<V> implements Transformer<V,String> {
    	public String transform(V v) {
    		return nodeExChanger((String) v);
        }
    	public String phase(String p){
    		return p;
    	}
    }
    //tooltipの表示クラス
    private class ConstLabeller<V> extends ToStringLabeller<V>{
    	public ConstLabeller(){
    		init();
    	}
    	public String transform(V v){
    		String str = (String) v;
    		String changedstr = dotToFeed(str);
			return changedstr;
    	}
    	//tooltipのフォント変え
		private String dotToFeed(String str) {
			str = str.replace(":",": ");
			str = str.replace(",","<p>");
			return "<html>"+str+"</html>";
		}
    }

}
