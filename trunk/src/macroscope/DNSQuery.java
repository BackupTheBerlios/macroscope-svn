import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.applet.Applet;

//import javax.naming.Context;
//import javax.naming.InitialContext;
//import javax.naming.NamingException;

public class DNSQuery extends java.applet.Applet implements MouseMotionListener {
  private boolean resolved_;
  private Label txt_;
  
  public void mouseMoved(MouseEvent evt){
    if( !resolved_ ){
      try {
        InetSocketAddress addr = new InetSocketAddress(txt_.getText(),0);
        txt_.setText(addr.getHostName());
        resolved_ = true;
      }
      catch( Exception e ){
      }
    }
  }
  public void mouseDragged(MouseEvent evt){
  }
  
  public void init(){
    resolved_ = false;
    setLayout(new BorderLayout());
    add(txt_ = new Label(getParameter("name"),Label.CENTER),BorderLayout.CENTER);
    txt_.addMouseMotionListener(this);
    txt_.setBackground(Color.white);
    resize(50,20);
  }
  
  public void start(){
  }

  public void paint(Graphics g){
  }

  public static void main(String args[]){
    DNSQuery applet = new DNSQuery();
    applet.init();
    applet.start();
  }
}
