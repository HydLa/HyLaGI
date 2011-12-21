package test.sakuplot.gui;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 * プロット画面のオプション設定ダイアログ。
 */
public class PlotOptionDialog extends JDialog
{
	private static final long serialVersionUID = 1L;
	
	private PlotPanel _parent;
	private JSlider _aspectSlider;
	private JLabel _aspectLabel;
	private ThumbnailButton _buttonLineColor = new ThumbnailButton("Line color", Color.BLUE);
	private ThumbnailButton _buttonBackColor = new ThumbnailButton("Background color", Color.WHITE);
	private int _aspectRatio = 5;
	private Color _lineColor = Color.BLUE;
	private Color _backColor = Color.WHITE;
	
	private Color _originalLineColor;
	private Color _originalBackColor;
	private int _originalAspectRatio;
	
	public PlotOptionDialog(PlotPanel parent)
	{
		_parent = parent;
		
		setTitle("Preferences");
		setSize(380, 140);
		setResizable(false);
		setLocationRelativeTo(null);
		setLayout(new FlowLayout());
		
		add(new JLabel("Aspect ratio:"));
		
		_aspectSlider = new JSlider(SwingConstants.HORIZONTAL, 1, 99, 50);
		_aspectSlider.setToolTipText("set the ratio (X-unit : Y-unit)");
		_aspectSlider.addChangeListener(new ChangeListener()
		{
			public void stateChanged(ChangeEvent e)
			{
				_aspectRatio = _aspectSlider.getValue();
				updatePresentations();
				applyProperties();
			}
		});
		add(_aspectSlider);
		
		_aspectLabel = new JLabel("50:50");
		_aspectLabel.setPreferredSize(new Dimension(60, 14));
		add(_aspectLabel);
		
		_buttonLineColor.setPreferredSize(new Dimension(160, 30));
		_buttonLineColor.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				ColorSelector selector = new ColorSelector();
				selector.setColor(_lineColor);
				selector.setFinishListener(new ColorSelector.SelectionFinishListener()
				{
					public void selectionFinished(Color selectedColor)
					{
						_lineColor = selectedColor;
						updatePresentations();
						applyProperties();
					}
				});
				selector.setVisible(true);
			}
		});
		add(_buttonLineColor);
		
		_buttonBackColor.setPreferredSize(new Dimension(160, 30));
		_buttonBackColor.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				ColorSelector selector = new ColorSelector();
				selector.setColor(_backColor);
				selector.setFinishListener(new ColorSelector.SelectionFinishListener()
				{
					public void selectionFinished(Color selectedColor)
					{
						_backColor = selectedColor;
						updatePresentations();
						applyProperties();
					}
				});
				selector.setVisible(true);
			}
		});
		add(_buttonBackColor);
		
		JButton buttonOk = new JButton("OK");
		buttonOk.setPreferredSize(new Dimension(90, 24));
		buttonOk.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				setVisible(false);
			}
		});
		add(buttonOk);
		
		JButton buttonCancel = new JButton("Cancel");
		buttonCancel.setPreferredSize(new Dimension(90, 24));
		buttonCancel.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				restoreProperties();
				setVisible(false);
			}
		});
		add(buttonCancel);
		
		addWindowListener(new WindowAdapter()
		{
			public void windowClosing(WindowEvent e)
			{
				restoreProperties();
			}
		});
		
		getRootPane().setDefaultButton(buttonOk);
		
		saveProperties();
		updatePresentations();
	}
	
	private void updatePresentations()
	{
		_aspectSlider.setValue(_aspectRatio);
		_aspectLabel.setText(_aspectRatio + ":" + (100 - _aspectRatio));
		_buttonLineColor.setColor(_lineColor);
		_buttonBackColor.setColor(_backColor);
		repaint();
	}
	
	private void applyProperties()
	{
		_parent.setLineColor(_lineColor);
		_parent.setBackgroundColor(_backColor);
		_parent.setAspectRatio(2.0 * _aspectRatio / 100.0);
		_parent.repaint();
	}
	
	private void saveProperties()
	{
		_lineColor = _originalLineColor = _parent.getLineColor();
		_backColor = _originalBackColor = _parent.getBackgroundColor();
		_aspectRatio = _originalAspectRatio = (int)(100.0 * _parent.getAspectRatio() / 2.0);
	}
	
	private void restoreProperties()
	{
		_parent.setLineColor(_originalLineColor);
		_parent.setBackgroundColor(_originalBackColor);
		_parent.setAspectRatio(2.0 * _originalAspectRatio / 100.0);
	}
}

/**
 * カラーパネル
 */
class ColorThumbnail extends JPanel
{
	private static final long serialVersionUID = 1L;
	
	private Color _c;
	
	public void setColor(Color c)
	{
		_c = c;
		repaint();
	}
	
	protected void paintComponent(Graphics g)
	{
		g.setColor(_c);
		g.fillRect(0, 0, getWidth(), getHeight());
	}
}

/**
 * 小さなカラーサムネイルをもつボタン
 */
class ThumbnailButton extends JButton
{
	private static final long serialVersionUID = 1L;
	
	private static final int M = 5; // margin
	private static final int TW = 30; // width of color thumbnail
	private static final int TH = 16; // height of color thumbnail
	
	private JLabel _label;
	private ColorThumbnail _colorThumbnail = new ColorThumbnail();
	
	public ThumbnailButton(String text, Color color)
	{
		super();
		setLayout(null);
		_label = new JLabel(text);
		add(_label);
		add(_colorThumbnail);
		setColor(color);
		
		addComponentListener(new ComponentAdapter()
		{
			public void componentResized(ComponentEvent e)
			{
				int w = getWidth(), h = getHeight();
				_label.setBounds(M, M, w - TW - 2 * M, h - 2 * M);
				_colorThumbnail.setBounds(w - TW - M, (h - TH) / 2, TW, TH);
			}
		});
	}
	
	public void setColor(Color color)
	{
		_colorThumbnail.setColor(color);
	}
}
