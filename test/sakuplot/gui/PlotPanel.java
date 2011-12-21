package test.sakuplot.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import javax.swing.JPanel;

import test.sakuplot.exp.DefaultExpressionEnv;
import test.sakuplot.exp.ExpressionEnv;
import test.sakuplot.model.CompoundGraph;
import test.sakuplot.model.EpsGraphics;
import test.sakuplot.model.GraphModel;
import test.sakuplot.model.VecD;

/**
 * 一連のプロット曲線。
 */
final class Plot implements Iterable<VecD>
{
	public String caseText;
	public String ipText;

	private List<VecD> _points = new ArrayList<VecD>();

	public void clear()
	{
		_points.clear();
	}

	public void add(VecD point)
	{
		_points.add(point);
	}

	public int count()
	{
		return _points.size();
	}

	public VecD get(int index)
	{
		return _points.get(index);
	}

	public Iterator<VecD> iterator()
	{
		return _points.iterator();
	}
}

public class PlotPanel extends JPanel
{
	private static final long serialVersionUID = 1L;

	private List<String> _caseTips = new ArrayList<String>();
	private List<String> _ipTips = new ArrayList<String>();
	private List<List<Plot>> _plots = new ArrayList<List<Plot>>();
	private Color _lineColor = Color.BLUE;
	private Color _lineHotColor = Color.RED;
	private Color _backColor = Color.WHITE;
	private double _toScale = 100.0;
	private double _curScale = 100.0;
	private double _aspectRatio = 1.0; // 縦の長さに対する横の長さの割合（合わせて 2.0 であることに注意）
	private VecD _toCenter = new VecD();
	private VecD _curCenter = new VecD();
	private boolean _antialias;
	private boolean _drag;
	private Point _mouse;
	private boolean _hoverIP = false; // highlight IP instead of CASE.
	private int _hoverIPIndex = -1;
	private int _hoverCaseIndex = -1;
	private boolean[] _outerFlags = new boolean[4]; // left, top, right, bottom

	private boolean _animating = false;

	public PlotPanel()
	{
		addMouseListener(new MouseAdapter()
		{
			public void mousePressed(MouseEvent e)
			{
				if (e.isPopupTrigger())
				{
					PlotPopupMenu popup = new PlotPopupMenu(PlotPanel.this);
					popup.show(PlotPanel.this, e.getX(), e.getY());
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
					PlotPopupMenu popup = new PlotPopupMenu(PlotPanel.this);
					popup.show(PlotPanel.this, e.getX(), e.getY());
				}
				_drag = false;
			}
		});

		addMouseMotionListener(new MouseMotionAdapter()
		{
			public void mouseMoved(MouseEvent e)
			{
				double dist = 10 * 10;
				int x = e.getX();
				int y = e.getY();
				_hoverCaseIndex = -1;
				Plot hoverPlot = null;
				for (int caseIndex = 0; caseIndex < _plots.size(); caseIndex++)
				{
					List<Plot> plotOneCase = _plots.get(caseIndex);
					for (int ipIndex = 0; ipIndex < plotOneCase.size(); ipIndex++)
					{
						Plot plot = plotOneCase.get(ipIndex);
						for (VecD v : plot)
						{
							VecD pp = modelToView(v).sub(x, y);
							double d = pp.square();
							if (d < dist)
							{
								dist = d;
								hoverPlot = plot;
								_hoverCaseIndex = caseIndex;
								_hoverIPIndex = ipIndex;
							}
						}
					}
				}
				_hoverIP = e.isShiftDown();
				if (_hoverCaseIndex != -1)
				{
					if (_hoverIP)
					{
						//setToolTipText(_ipTips.get(_hoverIPIndex));
						setToolTipText(hoverPlot.ipText);
					}
					else
					{
						//setToolTipText(_caseTips.get(_hoverCaseIndex));
						setToolTipText(hoverPlot.caseText);
					}
				}
				else
				{
					setToolTipText(null);
				}
				repaint();
			}

			public void mouseDragged(MouseEvent e)
			{
				if (_drag)
				{
					Point m = e.getPoint();
					int dx = m.x - _mouse.x;
					int dy = m.y - _mouse.y;
					_toCenter = _curCenter = _curCenter.add(dx, dy);
					_mouse = m;
					repaint();
				}
			}
		});

		addMouseWheelListener(new MouseWheelListener()
		{
			public void mouseWheelMoved(MouseWheelEvent e)
			{
				double r = e.getWheelRotation() < 0 ? 1.2 : 0.8;
				_toScale *= r;
				if (_toScale < 1.0)
				{
					_toScale = 1.0;
				}
				else if (5000.0 < _toScale)
				{
					_toScale = 5000.0;
				}
				else
				{
					Point m = e.getPoint();
					_toCenter = _toCenter.sub(m.x, m.y).scale(r).add(m.x, m.y);
				}

				startAnimation();
				repaint();
			}
		});
	}

	/**
	 * グラフの曲線を描画する色を取得します。
	 * @return 曲線の色
	 */
	public Color getLineColor()
	{
		return _lineColor;
	}

	/**
	 * グラフの曲線を描画する色を設定します。
	 * @param color 曲線の色
	 */
	public void setLineColor(Color color)
	{
		_lineColor = color;
		repaint();
	}

	/**
	 * グラフの背景色を取得します。
	 * @return 背景色
	 */
	public Color getBackgroundColor()
	{
		return _backColor;
	}

	/**
	 * グラフの背景色を設定します。
	 * @param color 背景色
	 */
	public void setBackgroundColor(Color color)
	{
		_backColor = color;
		repaint();
	}

	/**
	 * グラフのアスペクト比を取得します。
	 * @return アスペクト比
	 */
	public double getAspectRatio()
	{
		return _aspectRatio;
	}

	/**
	 * グラフのアスペクト比を設定します。
	 * <p><b>前提条件:</b> <code>0 < ratio && ratio < 2</code> が <code>true</code> である</p>
	 * @param ratio アスペクト比
	 * @throws IllegalArgumentException 0 以下か 2 以上の値を設定しようとした場合
	 */
	public void setAspectRatio(double ratio) throws IllegalArgumentException
	{
		if (ratio <= 0 || 2 <= ratio)
			throw new IllegalArgumentException("out of range: " + ratio);

		_aspectRatio = ratio;
		repaint();
	}

	public void setCenter(int x, int y)
	{
		_curCenter = new VecD(x, y);
		_toCenter = new VecD(x, y);
	}

	public void setScale(double scale)
	{
		_curScale = scale;
		_toScale = scale;
	}

	public void setAntialias(boolean f)
	{
		_antialias = f;
		repaint();
	}

	private VecD modelToView(VecD pt)
	{
		double t = _aspectRatio / 2.0;
		return pt.scale(_curScale*t, -_curScale*(1-t)).add(_curCenter);
	}

	private VecD modelToViewImmediate(VecD pt)
	{
		double t = _aspectRatio / 2.0;
		return pt.scale(_toScale*t, -_toScale*(1-t)).add(_toCenter);
	}

	private VecD modelToEps(VecD pt)
	{
		double t = _aspectRatio / 2.0;
		return pt.scale(_curScale*t, _curScale*(1-t)).add(new VecD(_curCenter.x, getHeight() - _curCenter.y));
	}

	@SuppressWarnings("unused")
	private VecD viewToModel(VecD pt)
	{
		double t = _aspectRatio / 2.0;
		return pt.sub(_curCenter).scale(1.0 / (_curScale*t), 1.0 / (_curScale*(1-t))).scale(1, -1);
	}

	protected void paintComponent(Graphics g)
	{
		synchronized (this)
		{
			g.setColor(_backColor);
			g.fillRect(0, 0, getWidth(), getHeight());

			g.setColor(Color.GRAY);

			g.drawString(String.format("mag = %.2f", _curScale), 10, 20);

			// axises
			Point pt = modelToView(new VecD()).toPoint();
			g.drawLine(0, pt.y, getWidth(), pt.y);
			g.drawLine(pt.x, 0, pt.x, getHeight());

			// try to plot ticks
			boolean f = drawTicks(g,   0.1, 20);
			if (!f) f = drawTicks(g,   1.0, 20);
			if (!f) f = drawTicks(g,  10.0, 20);
			if (!f) f = drawTicks(g, 100.0, 20);

			Graphics2D g2 = (Graphics2D)g;
			if (_antialias)
			{
				g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			}

			Arrays.fill(_outerFlags, false);

			// plot
			g.setColor(_lineColor);
			for (int caseIndex = 0; caseIndex < _plots.size(); caseIndex++)
			{
				if (!_hoverIP && caseIndex == _hoverCaseIndex) continue;

				List<Plot> plotOneCase = _plots.get(caseIndex);
				for (int j = 0; j < plotOneCase.size(); j++)
				{
					if (_hoverIP && caseIndex == _hoverCaseIndex && j == _hoverIPIndex) continue;
					drawPlot(g, plotOneCase.get(j));
				}
			}

			if (_hoverCaseIndex != -1)
			{
				g.setColor(_lineHotColor);
				List<Plot> plotOneCase = _plots.get(_hoverCaseIndex);
				if (_hoverIP)
				{
					drawPlot(g, plotOneCase.get(_hoverIPIndex));
				}
				else
				{
					for (int j = 0; j < plotOneCase.size(); j++)
					{
						drawPlot(g, plotOneCase.get(j));
					}
				}
			}

			int cx = getWidth() / 2, cy = getHeight() / 2;
			int[] af = { -1, 0, 1, 0 };
			for (int i = 0; i < 4; i++)
			{
				if (_outerFlags[i])
				{
					drawTriangle(g, cx + af[i] * (cx - 2), cy + af[(i + 3) % 4] * (cy - 2), i);
				}
			}

			if (_antialias)
			{
				g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_DEFAULT);
			}
		}
	}

	private void drawPlot(Graphics g, Plot plot)
	{
		int count = plot.count();

		if (count < 2)
		{
			return;
		}

		VecD p1 = plot.get(0);
		Point pt1 = modelToView(p1).toPoint();

		for (int i = 1; i < count; i++)
		{
			if (pt1.x < 0)				_outerFlags[0] = true;
			if (pt1.y < 0)				_outerFlags[1] = true;
			if (getWidth() < pt1.x)		_outerFlags[2] = true;
			if (getHeight() < pt1.y)	_outerFlags[3] = true;

			VecD p2 = plot.get(i);
			Point pt2 = modelToView(p2).toPoint();

			if (VecD.isValid(p1) && VecD.isValid(p2))
			{
				if (pt1.distanceSq(pt2) > 10)
				if (VecD.distanceSq(p1, p2) < 10 * 10)
				{
					g.drawLine(pt1.x, pt1.y, pt2.x, pt2.y);
				}
			}

			if (pt1.distanceSq(pt2) > 10)
			{
				p1 = p2;
				pt1 = pt2;
			}
		}
	}

	private boolean drawTicks(Graphics g, double d, int minPixels)
	{
		minPixels = Math.max(minPixels, 1);

		VecD p1 = modelToView(new VecD(d, 0));
		VecD p2 = modelToView(new VecD(0, d));
		VecD ticks = p1.sub(p2);

		boolean flag = false;

		//if (Math.min(ticks.x, ticks.y) < minPixels)
		//{
		//	return false;
		//}

		if (ticks.x >= minPixels)
		{
			int start = (int)Math.floor((double)(-_curCenter.x) / ticks.x);
			int end = (int)Math.ceil((double)(getWidth() - _curCenter.x) / ticks.x);
			for (int x = start; x <= end; x++)
			{
				Point pt = modelToView(new VecD(x * d, 0)).toPoint();
				g.drawLine(pt.x, pt.y - 4, pt.x, pt.y + 4);
				g.drawString(String.format("%.1f", x * d), pt.x + 1, pt.y - 2);
			}
			flag = true;
		}

		if (ticks.y >= minPixels)
		{
			int start = (int)Math.floor((double)(_curCenter.y - getHeight()) / ticks.y);
			int end = (int)Math.ceil((double)(_curCenter.y) / ticks.y);
			for (int y = start; y <= end; y++)
			{
				if (y != 0)
				{
					Point pt = modelToView(new VecD(0, y * d)).toPoint();
					g.drawLine(pt.x - 4, pt.y, pt.x + 4, pt.y);
					g.drawString(String.format("%.1f", y * d), pt.x + 2, pt.y - 1);
				}
			}
			flag = flag && true;
		}
		else
		{
			flag = false;
		}
		return flag;
	}

	/**
	 * 直角二等辺三角形を描画します。
	 * パラメータで指定される座標 (topX, topY) は、この三角形において直角をもつ頂点の座標を表します。
	 * @param g グラフィックコンテキスト
	 * @param topX 頂点の X 座標
	 * @param topY 頂点の Y 座標
	 * @param direction 方向（左、上、右、下をそれぞれ 0, 1, 2, 3 で表します）
	 */
	private void drawTriangle(Graphics g, int topX, int topY, int direction)
	{
		final int w = 8;
		if (0 <= direction && direction < 4)
		{
			int a = new int[] {  1,  1, -1, -1 }[direction];
			int b = new int[] { -1,  1,  1, -1 }[direction];
			int[] xPoints = { topX, topX + a * w, topX - b * w };
			int[] yPoints = { topY, topY + b * w, topY + a * w };

			g.setColor(new Color(0, 192, 255));
			g.fillPolygon(xPoints, yPoints, 3);
			g.setColor(Color.WHITE);
			g.drawPolygon(xPoints, yPoints, 3);
		}
	}

	/**
	 * 表示領域の位置とスケールを初期状態に戻します。
	 */
	public void resetDisplayArea()
	{
		_toScale = 100.0;
		_toCenter = _toCenter.sub(modelToViewImmediate(new VecD()).sub(new VecD(getWidth() / 2, getHeight() / 2)));
		startAnimation();
	}

	/**
	 * 描画されているグラフ全体が見えるようにスケールと中心点を調整します。
	 * グラフ範囲が大きすぎるとかえって見づらくなるかも知れません。
	 */
	public void setDisplayAreaToVisible()
	{
		VecD min = new VecD(Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY);
		VecD max = new VecD(Double.NEGATIVE_INFINITY, Double.NEGATIVE_INFINITY);
		for (List<Plot> casePlot : _plots)
		{
			for (Plot plot : casePlot)
			{
				for (VecD v : plot)
				{
					if (v.x < min.x) min.x = v.x;
					if (max.x < v.x) max.x = v.x;
					if (v.y < min.y) min.y = v.y;
					if (max.y < v.y) max.y = v.y;
				}
			}
		}

		if (!min.isValid() || !max.isValid()) return;

		VecD diag = modelToViewImmediate(max).sub(modelToViewImmediate(min));
		double size = Math.max(Math.abs(diag.x), Math.abs(diag.y)) + 10;
		int areaSize = Math.min(getWidth(), getHeight());
		_toScale *= areaSize / size;
		_toCenter = _toCenter.sub(modelToViewImmediate(min.add(max).div(2)).sub(getWidth() / 2, getHeight() / 2));
		startAnimation();
	}

	/**
	 * 複合グラフのリストを設定し、プロットします。
	 * @param compoundGraphList 複合グラフのリスト
	 */
	public void setCompoundGraph(List<CompoundGraph> compoundGraphList)
	{
		ExpressionEnv env = new DefaultExpressionEnv();

		_plots.clear();
		_caseTips.clear();
		_ipTips.clear();
		for (CompoundGraph graphs : compoundGraphList)
		{
			_caseTips.add(graphs.getName());
			List<Plot> plotOneCase = new ArrayList<Plot>();
			for (GraphModel g : graphs.getGraphList())
			{
				_ipTips.add(g.toString());

				Plot plot = new Plot();
				plot.caseText = graphs.getName();
				plot.ipText = g.toString();
				double min = g.getMinT(), max = g.getMaxT(), h = 0.001;
				for (int i = 0; min + i * h <= max; i++)
				{
					double t = min + i * h;
					env.setVar("t", t);

					double x = g.getExpressionX().getValue(env);
					double y = g.getExpressionY().getValue(env);

					plot.add(new VecD(x, y));
				}
				plotOneCase.add(plot);
			}
			_plots.add(plotOneCase);
		}
		repaint();
	}

	/**
	 * EPS 形式のグラフィックデータを作成します。
	 * @return EPS グラフィックデータ
	 */
	public EpsGraphics createEps()
	{
		int margin = 10;
		EpsGraphics eps = new EpsGraphics(getWidth() + 2 * margin, getHeight() + 2 * margin);

		eps.setLineWidth(0.25);

		// axises
		VecD axis = modelToEps(new VecD());
		eps.moveTo(margin, axis.y);
		eps.lineTo(eps.getWidth() - margin, axis.y);
		eps.moveTo(axis.x, margin);
		eps.lineTo(axis.x, eps.getHeight() - margin);
		eps.stroke();

		eps.setLineWidth(0.5);

		Rectangle bounds = new Rectangle(margin, margin, eps.getWidth() - margin, eps.getHeight() - margin);

		// plot
		for (List<Plot> plotCase : _plots)
		{
			for (Plot plot : plotCase)
			{
				if (plot.count() < 2) continue;

				VecD p1 = plot.get(0);
				VecD v1 = modelToEps(p1);

				boolean moved = false;
				for (int i = 1; i < plot.count(); i++)
				{
					VecD p2 = plot.get(i);
					VecD v2 = modelToEps(p2);

					if (v1.distanceSq(v2) > 4 * 4) // 表示座標の間隔をある程度広く
					{
						if (bounds.contains(v1.x, v1.y) || bounds.contains(v2.x, v2.y))
						{
							if (VecD.isValid(p1) && VecD.isValid(p2))
							{
								if (VecD.distanceSq(p1, p2) < 10 * 10)
								{
									if (!moved)
									{
										eps.moveTo(v1.x, v1.y);
										moved = true;
									}
									eps.lineTo(v2.x, v2.y);
								}
							}
						}
						else
						{
							moved = false;
						}
						p1 = p2;
						v1 = v2;
					}
				}
			}
			eps.stroke();
		}

		return eps;
	}

	private void startAnimation()
	{
		synchronized (this)
		{
			if (!_animating)
			{
				_animating = true;
				new Thread()
				{
					public void run()
					{
						while (_animating)
						{
							updateAnimation();
							try
							{
								Thread.sleep(20);
							}
							catch (InterruptedException e)
							{
								e.printStackTrace();
							}
						}
					}
				}.start();
			}
		}
	}

	private void updateAnimation()
	{
		synchronized (this)
		{
			_curScale += 0.3 * (_toScale - _curScale);
			_curCenter = _curCenter.add(_toCenter.sub(_curCenter).scale(0.3));
			if (Math.abs(_toScale - _curScale) < 1.0e-3 && _curCenter.distanceSq(_toCenter) < 1.0e-3)
			{
				_curScale = _toScale;
				_curCenter = _toCenter;
				_animating = false;
			}
		}
		repaint();
	}
}
