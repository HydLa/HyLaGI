package test.gnuplot;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.FlowLayout;
import java.io.*;

import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;
import javax.imageio.ImageIO;

import test.Env;
import test.FrontEnd;

public class GnuplotPanel extends JPanel implements Runnable{
		private JPanel superpanel = new JPanel(new FlowLayout());
		private ImageIcon icon;
		private String message;
		private String plotType;
		public static boolean output=false;
		public static boolean ThDimension=false;
		public static boolean PanelRatio=false;

		public GnuplotPanel(){
			setLayout(new BorderLayout());
			setBackground(Color.white);
			SwingUtilities.invokeLater(	new GnuplotPanel(icon, message)	);
		}

		public GnuplotPanel(ImageIcon icon,String message) {
			this.icon = icon;
			this.message = message;
		}

		public void restart(String filename){
			clearPanel();
			start(filename);
			repaintPanel();
		}

		private void clearPanel(){
			superpanel.removeAll();
		}

		private void start(String filename){
			try {
				Process p = new ProcessBuilder(Env.get("GNUPLOT_PATH"),"-").start();
				BufferedInputStream in = new BufferedInputStream(p.getInputStream());
				PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(p.getOutputStream())));
				BufferedReader err = new BufferedReader(new InputStreamReader(p.getErrorStream()));
				StringWriter log = new StringWriter();
				out.println("set terminal png");

//３次元グラフ・・なぜか起動しない
				out.println((ThDimension?"splot":"plot")+" "+"\""+filename+"\""+" "+Env.get("GNUPLOT_OPTION")+" w l ");
				System.out.println((ThDimension?"splot":"plot")+" "+"\""+filename+"\""+" "+Env.get("GNUPLOT_OPTION")+" w l ");

				if(PanelRatio){
					out.println("set size square");
				}

				if(output) {
					out.println("set terminal png color");
					out.println("set output"+"\"out.png\"");
					out.println("replot");
				}else{
					out.println("set output");
				}
				out.close();
				ImageIcon icon = new ImageIcon(ImageIO.read(in));
				in.close();
				while(true){
					int c = err.read();
					if(c == -1) break;
					System.out.print((char)c);
					log.append((char)c);
				}
				err.close();
				try {
					p.waitFor();
				} catch (InterruptedException e) {
					// TODO 自動生成された catch ブロック
					e.printStackTrace();
				}
				superpanel.add(new JLabel(icon), BorderLayout.CENTER);
				superpanel.add(new JTextArea(message,4,40), BorderLayout.SOUTH);
				add(superpanel,BorderLayout.CENTER);
				add(superpanel);
				superpanel.setVisible(true);
			} catch (IOException e) {
				// TODO 自動生成された catch ブロック
				e.printStackTrace();
			}
		}

		private void repaintPanel(){
			superpanel.validate();
		}

		public void run() {

		}
}
