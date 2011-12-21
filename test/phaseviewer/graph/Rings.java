package test.phaseviewer.graph;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Point2D;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import test.phaseviewer.gui.MainFrame;
import test.phaseviewer.gui.StartButton;

import edu.uci.ics.jung.algorithms.layout.PolarPoint;
import edu.uci.ics.jung.algorithms.layout.RadialTreeLayout;
import edu.uci.ics.jung.visualization.Layer;
import edu.uci.ics.jung.visualization.VisualizationServer;

//radialëÄçÏÇÃÇ∆Ç´ÇÃâ~Çï\é¶Ç∑ÇÈÉNÉâÉX
public class Rings extends Tree implements VisualizationServer.Paintable {

    public RadialTreeLayout<String,Integer> radialLayout;
    Collection<Double> depths;

    public Rings(DrawGraph drawgraph){
    	radialLayout = drawgraph.getRadLayout();
    	depths = getDepths();
    }

    @SuppressWarnings("static-access")
	private Collection<Double> getDepths() {
            Set<Double> depths = new HashSet<Double>();
            Map<String,PolarPoint> polarLocations = radialLayout.getPolarLocations();
            for(String v :MainFrame.getControlpanel().getStartbutton().getDrawgraph().getCreategraph().getgraph().getVertices()) {
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
                            Shape shape = //StartButton.
                            		MainFrame.controlpanel.getStartbutton().getDrawgraph().getCreategraph().getvv().
                            		getRenderContext().getMultiLayerTransformer().getTransformer(Layer.LAYOUT).transform(ellipse);
                            g2d.draw(shape);
                    }
            }

            public boolean useTransform() {
                    return true;
            }
}