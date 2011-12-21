package test.automaton;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.BoundedRangeModel;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.DefaultBoundedRangeModel;
import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.apache.commons.collections15.Factory;
import org.apache.commons.collections15.Transformer;

import test.graph.tokenMaker;

import edu.uci.ics.jung.algorithms.layout.CircleLayout;
import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.layout.SpringLayout;
import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.SparseMultigraph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.visualization.GraphZoomScrollPane;
import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.CrossoverScalingControl;
import edu.uci.ics.jung.visualization.control.DefaultModalGraphMouse;
import edu.uci.ics.jung.visualization.control.GraphMouseListener;
import edu.uci.ics.jung.visualization.control.ModalGraphMouse;
import edu.uci.ics.jung.visualization.control.ScalingControl;
import edu.uci.ics.jung.visualization.decorators.AbstractEdgeShapeTransformer;
import edu.uci.ics.jung.visualization.decorators.ConstantDirectionalEdgeValueTransformer;
import edu.uci.ics.jung.visualization.decorators.EdgeShape;
import edu.uci.ics.jung.visualization.decorators.GradientEdgePaintTransformer;
import edu.uci.ics.jung.visualization.decorators.PickableEdgePaintTransformer;
import edu.uci.ics.jung.visualization.decorators.PickableVertexPaintTransformer;
import edu.uci.ics.jung.visualization.decorators.ToStringLabeller;
import edu.uci.ics.jung.visualization.renderers.EdgeLabelRenderer;
import edu.uci.ics.jung.visualization.renderers.GradientVertexRenderer;
import edu.uci.ics.jung.visualization.renderers.Renderer;
import edu.uci.ics.jung.visualization.renderers.VertexLabelRenderer;

/**
 * Demonstrates jung support for drawing edge labels that
 * can be positioned at any point along the edge, and can
 * be rotated to be parallel with the edge.
 *
 * @author Tom Nelson
 *
 */
public class Automaton extends JPanel {
    /**
         *
         */
        private static final long serialVersionUID = -6077157664507049647L;

        /**
     * the graph
     */
    Graph<String,Integer> graph;

    /**
     * the visual component and renderer for the graph
     */
    VisualizationViewer<String, Integer> vv;

    /**
     */
    VertexLabelRenderer vertexLabelRenderer;
    EdgeLabelRenderer edgeLabelRenderer;

    ScalingControl scaler = new CrossoverScalingControl();
    //メインフレーム
    JFrame frame = new JFrame();
    //オプションフレーム
    JPanel optionPanel = new JPanel();
    JFrame optionFrame = new JFrame();

    public static int edgeNameFontSize;
    public static String edgeNameColor;

    Container content = frame.getContentPane();
    double nodesize = 1;
    /**
     * create an instance of a simple graph with controls to
     * demo the label positioning features
     *
     */
    @SuppressWarnings("serial")
        public Automaton(String filename) {

    	setLayout(new BorderLayout());


        // create a simple graph for the demo
        graph = new SparseMultigraph<String,Integer>();
        //Integer[] v = createVertices(3);

        createTree(filename);

        Layout<String,Integer> layout = new CircleLayout<String,Integer>(graph);
        vv =  new VisualizationViewer<String,Integer>(layout , new Dimension(600,500) );
        vv.setBackground(Color.white);

        vertexLabelRenderer = vv.getRenderContext().getVertexLabelRenderer();
        edgeLabelRenderer = vv.getRenderContext().getEdgeLabelRenderer();


        //edgeの表示
        Transformer<Integer, String> edgename = new Transformer<Integer,String>(){
            public String transform(Integer e) {
            	String str = phasethi[e].toString();
                return "<html>"+"<font color="+edgeNameColor+">"+"<font size="+edgeNameFontSize+">"+str+"</html>";
            }
        };
        vv.getRenderContext().setEdgeLabelTransformer(edgename);
        //edgeの色
        vv.getRenderContext().setEdgeDrawPaintTransformer(new GradientEdgePaintTransformer<String, Integer>(Color.black,Color.LIGHT_GRAY,new VisualizationViewer<String, Integer>(layout)));

        //nodeの表示
        Transformer<String, String> nodename = new Transformer<String,String>(){
            public String transform(String e) {
                return e;            }
        };
        vv.getRenderContext().setVertexLabelTransformer(nodename);

        //nodeの形状の変更
        Transformer<String, Shape> nodeshape = new Transformer<String,Shape>(){
        	public Shape transform(String e) {
                return new Rectangle(rect_element(-25),rect_element(-20),rect_element(40),rect_element(30));            }
        };
        vv.getRenderContext().setVertexShapeTransformer(nodeshape);

        //nodeの色決め
        setColor();
        // add my listener for ToolTips
        vv.setVertexToolTipTransformer(new ToStringLabeller<String>());


        // create a frome to hold the graph
//        final GraphZoomScrollPane panel = new GraphZoomScrollPane(vv);
//        content.add(panel,BorderLayout.EAST);

        final DefaultModalGraphMouse<Integer,Number> graphMouse = new DefaultModalGraphMouse<Integer,Number>();
        vv.setGraphMouse(graphMouse);

        JButton plus = new JButton("+");
        plus.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                scaler.scale(vv, 1.1f, vv.getCenter());
                nodesize = nodesize*1.1;
                vv.repaint();
            }
        });

        JButton minus = new JButton("-");
        minus.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                scaler.scale(vv, 1/1.1f, vv.getCenter());
                nodesize = nodesize/1.1;
                vv.repaint();
            }
        });

        ButtonGroup radio = new ButtonGroup();

        JRadioButton lineButton = new JRadioButton("Line");
        lineButton.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent e) {
                if(e.getStateChange() == ItemEvent.SELECTED) {
                    vv.getRenderContext().setEdgeShapeTransformer(new EdgeShape.Line<String,Integer>());//Integer,Number
                    vv.repaint();
                }
            }
        });

        JRadioButton quadButton = new JRadioButton("QuadCurve");
        quadButton.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent e) {
                if(e.getStateChange() == ItemEvent.SELECTED) {
                    vv.getRenderContext().setEdgeShapeTransformer(new EdgeShape.QuadCurve<String,Integer>());
                    vv.repaint();
                }
            }
        });

        JRadioButton cubicButton = new JRadioButton("CubicCurve");
        cubicButton.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent e) {
                if(e.getStateChange() == ItemEvent.SELECTED) {
                    vv.getRenderContext().setEdgeShapeTransformer(new EdgeShape.CubicCurve<String,Integer>());
                    vv.repaint();
                }
            }
        });

        JRadioButton bentLineButton = new JRadioButton("BentLine");
        bentLineButton.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent e) {
                if(e.getStateChange() == ItemEvent.SELECTED) {
                    vv.getRenderContext().setEdgeShapeTransformer(new EdgeShape.BentLine<String,Integer>());
                    vv.repaint();
                }
            }
        });

        radio.add(lineButton);
        radio.add(quadButton);
        radio.add(cubicButton);
        radio.add(bentLineButton);
        graphMouse.setMode(ModalGraphMouse.Mode.TRANSFORMING);

        JCheckBox rotate = new JCheckBox("<html><center>EdgeType<p>Parallel</center></html>");
        rotate.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent e) {
                AbstractButton b = (AbstractButton)e.getSource();
                edgeLabelRenderer.setRotateEdgeLabels(b.isSelected());
                vv.repaint();
            }
        });
        rotate.setSelected(true);
        MutableDirectionalEdgeValue mv = new MutableDirectionalEdgeValue(.7, .7);
        vv.getRenderContext().setEdgeLabelClosenessTransformer(mv);
        JSlider directedSlider = new JSlider(mv.getDirectedModel()) {
            public Dimension getPreferredSize() {
                Dimension d = super.getPreferredSize();
                d.width /= 2;
                return d;
            }
        };
//        JSlider undirectedSlider = new JSlider(mv.getUndirectedModel()) {
//            public Dimension getPreferredSize() {
//                Dimension d = super.getPreferredSize();
//                d.width /= 2;
//                return d;
//            }
//        };

        JSlider edgeOffsetSlider = new JSlider(0,100) {
            public Dimension getPreferredSize() {
                Dimension d = super.getPreferredSize();
                d.width /= 2;
                return d;
            }
        };
        edgeOffsetSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                JSlider s = (JSlider)e.getSource();
                AbstractEdgeShapeTransformer<String,Integer> aesf =
                    (AbstractEdgeShapeTransformer<String,Integer>)vv.getRenderContext().getEdgeShapeTransformer();
                aesf.setControlOffsetIncrement(s.getValue());
                vv.repaint();
            }

        });

        //オプションパネル
        JButton option = new JButton("option");
        option.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                optionFrame.add(optionPanel);
                optionFrame.setSize(600,150);
                optionFrame.setResizable(false);
                optionFrame.setTitle("Control Option");
                optionFrame.setVisible(true);
            }
        });

        //FontSizeを変更します
        String[] edgefontsize = {"2","3","4","5","6","9","12","15"};
        final JComboBox fontsizeBox = new JComboBox(edgefontsize);
        fontsizeBox.setPreferredSize(new Dimension(60,20));//width,height
        fontsizeBox.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
            		setEdgeNameFontSize((int) fontsizeBox.getSelectedIndex());
                    vv.repaint();
            }});

        fontsizeBox.setSelectedItem(""+setEdgeNameFontSize(6));
        JPanel fsbBox = new JPanel(new GridLayout(0,1));
        fsbBox.add(fontsizeBox);
        //fsbBox.setBorder(BorderFactory.createTitledBorder("FontSize"));


      //FontColorを変更します
        String[] edgeColor = {
        		"black",
        		"red",
        		"green",
        		"blue",
        		"olive",
        		"aqua",
        		"navy",
        		"teal",
        		"lime",
        		"silver",
        		"gray"
        		};
        final JComboBox fontColorBox = new JComboBox(edgeColor);
        fontsizeBox.setPreferredSize(new Dimension(60,20));//width,height
        fontColorBox.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
            		setEdgeNameColor((String) fontColorBox.getSelectedItem());
                    vv.repaint();
            }});
        fontColorBox.setSelectedItem(setEdgeNameColor("black"));
        JPanel fcbBox = new JPanel(new GridLayout(0,1));
        fcbBox.add(fontColorBox);
        //fcbBox.setBorder(BorderFactory.createTitledBorder("FontColor"));


        //ラベルの表示位置
        JComboBox cb = new JComboBox();
        cb.addItem(Renderer.VertexLabel.Position.N);
        cb.addItem(Renderer.VertexLabel.Position.NE);
        cb.addItem(Renderer.VertexLabel.Position.E);
        cb.addItem(Renderer.VertexLabel.Position.SE);
        cb.addItem(Renderer.VertexLabel.Position.S);
        cb.addItem(Renderer.VertexLabel.Position.SW);
        cb.addItem(Renderer.VertexLabel.Position.W);
        cb.addItem(Renderer.VertexLabel.Position.NW);
        cb.addItem(Renderer.VertexLabel.Position.N);
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
        JPanel cbbox = new JPanel(new GridLayout(0,1));
        cbbox.add(cb);
        cbbox.setBorder(BorderFactory.createTitledBorder("NamePos"));

        JPanel zoomPanel = new JPanel(new GridLayout(0,1));
        zoomPanel.setBorder(BorderFactory.createTitledBorder("Scale"));
        zoomPanel.add(plus);
        zoomPanel.add(minus);

        JPanel edgePanel = new JPanel(new GridLayout(0,1));
        edgePanel.setBorder(BorderFactory.createTitledBorder("EdgeType Type"));
        edgePanel.add(lineButton);
        edgePanel.add(quadButton);
        edgePanel.add(cubicButton);
        edgePanel.add(bentLineButton);

        JPanel rotatePanel = new JPanel();
        rotatePanel.setBorder(BorderFactory.createTitledBorder("Alignment"));
        rotatePanel.add(rotate);

        JPanel labelPanel = new JPanel(new BorderLayout());
        JPanel sliderPanel = new JPanel(new GridLayout(3,1));
        JPanel sliderLabelPanel = new JPanel(new GridLayout(3,1));
        JPanel offsetPanel = new JPanel(new BorderLayout());
        offsetPanel.setBorder(BorderFactory.createTitledBorder("Offset"));
        sliderPanel.add(directedSlider);
//        sliderPanel.add(undirectedSlider);
        sliderPanel.add(edgeOffsetSlider);
        sliderLabelPanel.add(new JLabel("Directed", JLabel.RIGHT));
//        sliderLabelPanel.add(new JLabel("Undirected", JLabel.RIGHT));
        sliderLabelPanel.add(new JLabel("Edges", JLabel.RIGHT));
        offsetPanel.add(sliderLabelPanel, BorderLayout.WEST);
        offsetPanel.add(sliderPanel);
        labelPanel.add(offsetPanel);
        labelPanel.add(rotatePanel, BorderLayout.WEST);

        JPanel modePanel = new JPanel(new GridLayout(2,1));
        modePanel.setBorder(BorderFactory.createTitledBorder("Mouse Mode"));
        modePanel.add(graphMouse.getModeComboBox());


        //制約表示コーナー
        scrollpane.setBorder(BorderFactory.createTitledBorder("Constraint"));

        JLabel edgefontlabel = new JLabel("Size");
        JLabel edgecolorlabel = new JLabel("Color");
        JPanel edgefont = new JPanel(new GridLayout(0,1));
        edgefont.setBorder(BorderFactory.createTitledBorder("EdgeFont"));
        edgefont.add(edgefontlabel);
        edgefont.add(fsbBox);
        edgefont.add(edgecolorlabel);
        edgefont.add(fcbBox);

        //オプションパネル
        optionPanel.add(edgePanel);
        optionPanel.add(labelPanel);
        optionPanel.add(cbbox);
        optionPanel.add(edgefont);

        JPanel setOption = new JPanel();
        setOption.setBorder(BorderFactory.createTitledBorder("Option"));
        setOption.add(option);

        modePanel.add(setOption);

        quadButton.setSelected(true);

        content.add(zoomPanel,BorderLayout.EAST);
        content.add(modePanel,BorderLayout.WEST);
//        content.add(setOption,BorderLayout.SOUTH);

        content.add(scrollpane,BorderLayout.CENTER);

    	JSplitPane jsp;
		jsp = new JSplitPane(JSplitPane.VERTICAL_SPLIT,vv,content);
		jsp.setOneTouchExpandable(true);
        jsp.setResizeWeight(0.8);
        add(jsp);

        setVisible(true);
    }

    /**
     * subclassed to hold two BoundedRangeModel instances that
     * are used by JSliders to move the edge label positions
     * @author Tom Nelson
     *
     *
     */
    class MutableDirectionalEdgeValue extends ConstantDirectionalEdgeValueTransformer<String,Integer> {
        BoundedRangeModel undirectedModel = new DefaultBoundedRangeModel(5,0,0,10);
        BoundedRangeModel directedModel = new DefaultBoundedRangeModel(7,0,0,10);

        public MutableDirectionalEdgeValue(double undirected, double directed) {
            super(undirected, directed);
            undirectedModel.setValue((int)(undirected*10));
            directedModel.setValue((int)(directed*10));

            undirectedModel.addChangeListener(new ChangeListener(){
                public void stateChanged(ChangeEvent e) {
                    setUndirectedValue(new Double(undirectedModel.getValue()/10f));
                    vv.repaint();
                }
            });
            directedModel.addChangeListener(new ChangeListener(){
                public void stateChanged(ChangeEvent e) {
                    setDirectedValue(new Double(directedModel.getValue()/10f));
                    vv.repaint();
                }
            });
        }
        /**
         * @return Returns the directedModel.
         */
        public BoundedRangeModel getDirectedModel() {
            return directedModel;
        }

        /**
         * @return Returns the undirectedModel.
         */
        public BoundedRangeModel getUndirectedModel() {
            return undirectedModel;
        }
    }

    //制約をテキストエリアに表示するクラス
	static ConstraintTextArea consTextArea = new ConstraintTextArea("");
    static JScrollPane scrollpane = new JScrollPane(consTextArea);
    @SuppressWarnings("serial")
	static class ConstraintTextArea extends JTextArea{
    	public ConstraintTextArea(String s){
    		super(s);
    		this.setEditable(false);
    	}
    }

    //マウスのクリックに関する操作クラス
    static class TestGraphMouseListener<V> implements GraphMouseListener<V> {
        public void graphClicked(V v, MouseEvent vv) {
        	//appendに画面が追いつくかどうか。trueならば追いつく
        	if(true){
        		javax.swing.JScrollBar jcrollbar = scrollpane.getVerticalScrollBar();
        		jcrollbar.setValue(jcrollbar.getMaximum());
        	}
            consTextArea.append(v+"\n");
        }
        public void graphPressed(V v, MouseEvent vv) {
            //System.err.println("Vertex "+v+" was pressed at ("+me.getX()+","+me.getY()+")");
        }
        public void graphReleased(V v, MouseEvent vv) {
            //System.err.println("Vertex "+v+" was released at ("+me.getX()+","+me.getY()+")");
        }
    }


    private void setColor(){
    	//色指定１
//    	vv.getRenderContext().setEdgeDrawPaintTransformer(new PickableEdgePaintTransformer<Integer>(vv.getPickedEdgeState(), Color.black, Color.cyan));
//        vv.getRenderContext().setVertexFillPaintTransformer(new PickableVertexPaintTransformer<String>(vv.getPickedVertexState(), Color.red, Color.yellow));
        //色指定２
        vv.getRenderer().setVertexRenderer(
                new GradientVertexRenderer<String,Integer>(
                                Color.white, Color.pink,
                                Color.white, Color.cyan,
                                vv.getPickedVertexState(),
                                false));
    }

    //nodeサイズの変更器
    int rect_element(int num){
    	double i = num*nodesize;
    	int n = (int)i;
    	return n;
    }



	public int cycleNum=0;
	public int cycleNumsec=0;
	public int cycleNumthi=0;
	//取得したデータの格納
	public static String[] phasefst = new String[15000];
	public static String[] phasesec = new String[15000];
	public static String[] phasethi = new String[15000];
	public void Fileloader(String filename) {
		final int FST = 0;
		final int SEC = 1;
		final int THI = 2;
		// TODO 自動生成されたメソッド・スタブ
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filename));

			String line;
			int element_num;
			while((line = reader.readLine())!=null){
				String[] block = line.trim().split("\\s+");
				 element_num = block.length;
				 phasefst[cycleNum] = block[FST];
				 //
				 	if(1<element_num) {
				 		phasesec[cycleNumsec]=block[SEC];
				 		cycleNumsec++;
				 		phasethi[cycleNumthi]=block[THI];
				 		cycleNumthi++;
					 }else{
						 phasesec[cycleNumsec]="";
						 cycleNumsec++;
						 phasethi[cycleNumthi]="";
					 	 cycleNumthi++;
					 }
				 	System.out.println("Automaton_phasefst["+cycleNum+"]: "+phasefst[cycleNum]+" Automaton_phasesec["+cycleNum+"]: "+phasesec[cycleNum]+" Automaton_phasethi["+cycleNum+"]: "+phasethi[cycleNum]);
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

	public boolean PhaseChecker(int num) {
		// TODO 自動生成されたメソッド・スタブ
	   	//indexOfは-1で含まれない
    	if(phasefst[num].indexOf("case")==0 || phasefst[num].indexOf("end")==0){return false;}
    	else if(phasefst[num].indexOf("case")==-1 && phasefst[num+1].indexOf("end")==-1 ) {
    		return PhasetoPhase(phasefst[num],phasesec[num]);
    	}else {
    		return false;
    	}
	}

	public boolean PhaseCheckerConst(int num) {
		// TODO 自動生成されたメソッド・スタブ
	   	//indexOfは-1で含まれない
    	if(phasefst[num].indexOf("case")==0 || phasefst[num].indexOf("end")==0){
    		return false;
    	}else if(phasefst[num].indexOf("case")==-1 && phasefst[num].indexOf("end")==-1 ) {
    		return PhasetoPhase(phasefst[num],phasesec[num]);
    	}else {
    		return false;
    	}
	}


	private int head = 0;
	public boolean PhasetoPhase(String n, String m) {
		head++;
	   	return graph.addEdge(head,n,m, EdgeType.DIRECTED);
	}

	public void createTree(String filename) {
		// TODO 自動生成されたメソッド・スタブ
    	Fileloader(filename);
//    	for(int k=0;k<cycleNum-1;k++){
//    		PhaseChecker(k);
//    	}
    	for(int k=0;k<cycleNumsec-1;k++){
    		PhaseCheckerConst(k);
    	}
	}

	public String nodeExChanger(String string) {
		// TODO 自動生成されたメソッド・スタブ
    	String node="";
    	for(int k=0;k<cycleNum;k++){
    		if(string == phasesec[k]){
    			node = phasefst[k];
    		}
    	}
    	return node;
	}

	public String edgeExChanger(String string) {
		// TODO 自動生成されたメソッド・スタブ
    	String edge="";
    	for(int k=0;k<cycleNum;k++){
    		if(string == phasesec[k]){
    			edge = phasethi[k];
    		}
    	}
    	return edge;
	}

	private int setEdgeNameFontSize(int num){
		edgeNameFontSize = num;
		return edgeNameFontSize;
	}

	private String setEdgeNameColor(String color){
		edgeNameColor = color;
		return edgeNameColor;
	}


}