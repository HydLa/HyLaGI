package test.plotgraph;

import java.awt.*;
import javax.swing.*;
import java.awt.List;
import java.util.*;
import java.io.*;


import javax.swing.event.*;
import javax.swing.JSlider;
import java.awt.BorderLayout;
import java.awt.Container;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;


public class Sliderpanel extends JPanel{

	Slider slider = new Slider();

	public Sliderpanel(){
		setLayout(new GridLayout(4, 1));
		slider.height_slider();
		slider.width_slider();
	  add(slider.label1);
	  add(slider.slider1);
	  add(slider.label0);
	  add(slider.slider0);
	}

}
