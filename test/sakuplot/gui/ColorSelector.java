package test.sakuplot.gui;

import java.awt.Color;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class ColorSelector extends JDialog implements ChangeListener
{
	private static final long serialVersionUID = 1L;
	
	private int _r;
	private int _g;
	private int _b;
	
	private JSlider _sliderR = new JSlider(0, 255);
	private JSlider _sliderG = new JSlider(0, 255);
	private JSlider _sliderB = new JSlider(0, 255);
	private JSpinner _spinR = new JSpinner(new SpinnerNumberModel(0, 0, 255, 1));
	private JSpinner _spinG = new JSpinner(new SpinnerNumberModel(0, 0, 255, 1));
	private JSpinner _spinB = new JSpinner(new SpinnerNumberModel(0, 0, 255, 1));
	private ColorSamplePanel _csp = new ColorSamplePanel();
	
	private SelectionFinishListener _finishListener = null;
	
	public interface SelectionFinishListener
	{
		public void selectionFinished(Color selectedColor);
	}
	
	public ColorSelector()
	{
		setTitle("Color");
		setModalityType(ModalityType.APPLICATION_MODAL);
		setSize(490, 230);
		setResizable(false);
		setLocationRelativeTo(null);
		setLayout(null);
		
		_sliderR.addChangeListener(this);
		_sliderG.addChangeListener(this);
		_sliderB.addChangeListener(this);
		_spinR.addChangeListener(this);
		_spinG.addChangeListener(this);
		_spinB.addChangeListener(this);
		
		_sliderR.setBounds(100, 20, 300, 24);
		JPanel bar_r = new GradationBar(Color.RED);
		bar_r.setBounds(105, 45, 290, 8);
		_spinR.setBounds(410, 20, 50, 22);
		
		_sliderG.setBounds(100, 60, 300, 24);
		JPanel bar_g = new GradationBar(Color.GREEN);
		bar_g.setBounds(105, 85, 290, 8);
		_spinG.setBounds(410, 60, 50, 22);
		
		_sliderB.setBounds(100, 100, 300, 24);
		JPanel bar_b = new GradationBar(Color.BLUE);
		bar_b.setBounds(105, 125, 290, 8);
		_spinB.setBounds(410, 100, 50, 22);
		
		add(_sliderR);
		add(_sliderG);
		add(_sliderB);
		add(_spinR);
		add(_spinG);
		add(_spinB);
		add(bar_r);
		add(bar_g);
		add(bar_b);
		
		_csp.setBounds(10, 30, 80, 80);
		add(_csp);
		
		JButton button_ok = new JButton("OK");
		button_ok.setBounds(275, 160, 90, 24);
		button_ok.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				if (_finishListener != null)
				{
					_finishListener.selectionFinished(getColor());
				}
				setVisible(false);
			}
		});
		button_ok.setDefaultCapable(true);
		add(button_ok);
		
		JButton button_cancel = new JButton("Cancel");
		button_cancel.setBounds(375, 160, 90, 24);
		button_cancel.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				setVisible(false);
			}
		});
		add(button_cancel);
		
		getRootPane().setDefaultButton(button_ok);
		
		setColor(Color.BLACK);
	}
	
	public Color getColor()
	{
		return new Color(_r, _g, _b);
	}
	
	public void setColor(Color color)
	{
		_r = color.getRed();
		_g = color.getGreen();
		_b = color.getBlue();
		updateProperties();
	}
	
	public void setFinishListener(SelectionFinishListener callback)
	{
		_finishListener = callback;
	}
	
	private void updateProperties()
	{
		_sliderR.setValue(_r);
		_spinR.setValue(_r);
		_sliderG.setValue(_g);
		_spinG.setValue(_g);
		_sliderB.setValue(_b);
		_spinB.setValue(_b);
		_csp.setColor(getColor());
		repaint();
	}
	
	public void stateChanged(ChangeEvent e)
	{
		Object sender = e.getSource();
		
		if (sender == _sliderR)
		{
			_r = _sliderR.getValue();
			_spinR.setValue(_r);
		}
		else if (sender == _sliderG)
		{
			_g = _sliderG.getValue();
			_spinG.setValue(_g);
		}
		else if (sender == _sliderB)
		{
			_b = _sliderB.getValue();
			_spinB.setValue(_b);
		}
		else if (sender == _spinR)
		{
			_r = (Integer)_spinR.getValue();
			_sliderR.setValue(_r);
		}
		else if (sender == _spinG)
		{
			_g = (Integer)_spinG.getValue();
			_sliderG.setValue(_g);
		}
		else if (sender == _spinB)
		{
			_b = (Integer)_spinB.getValue();
			_sliderB.setValue(_b);
		}
		_csp.setColor(new Color(_r, _g, _b));
		repaint();
	}
}

class GradationBar extends JPanel
{
	private static final long serialVersionUID = 1L;
	
	private Color _c;
	
	public GradationBar(Color c)
	{
		_c = c;
	}
	
	protected void paintComponent(Graphics g)
	{
		GradientPaint gp = new GradientPaint(0, 0, Color.BLACK, getWidth(), getHeight(), _c);
		((Graphics2D)g).setPaint(gp);
		g.fillRect(0, 0, getWidth(), getHeight());
	}
}

class ColorSamplePanel extends JPanel
{
	private static final long serialVersionUID = 1L;
	
	private Color _c;
	
	public void setColor(Color c)
	{
		_c = c;
	}
	
	protected void paintComponent(Graphics g)
	{
		Graphics2D g2 = (Graphics2D)g;
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setColor(_c);
		g.fillRoundRect(0, 0, getWidth(), getHeight(), 20, 20);
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_DEFAULT);
	}
}
