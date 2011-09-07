/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.amazon.ebook.booklet.msp;

import java.awt.*;

/*import java.awt.event.KeyEvent;
 import java.util.*;
 import com.amazon.kindle.kindlet.ui.helper.AppContextBridge;
 */
public class MyEventQueue extends EventQueue {
	/*
	 * Implementation of an ISO 14755-like unicode input method For example, in
	 * order to input the character u20ac, hold on alt-shift, and type u20ac. It
	 * is impossible to enter alt-shift-u via kindle dxg's keypad. However, the
	 * keycode sequence can be write to /dev/input/event0.
	 */
	/*
	 * private ArrayList uinput; private Component defaultSource=null; class
	 * RaiseKeyeventRunnable implements Runnable{ KeyEvent keyevent; public
	 * RaiseKeyeventRunnable(KeyEvent uevent){ keyevent=uevent; } public void
	 * run() { System.out.println("invoke "+Thread.currentThread( ));
	 * Toolkit.getDefaultToolkit().getSystemEventQueue().postEvent(keyevent);
	 * System
	 * .out.println(KeyboardFocusManager.getCurrentKeyboardFocusManager()); }
	 * 
	 * }; public MyEventQueue(Component widget) { uinput=new ArrayList();
	 * uinput.clear(); this.defaultSource=widget; }
	 * 
	 * 
	 * private void handleKey(KeyEvent event){ switch(event.getID()){ case
	 * KeyEvent.KEY_PRESSED: if(uinput.size()==0){ if(event.isAltDown() &&
	 * event.isShiftDown() && event.getKeyCode() == KeyEvent.VK_U){
	 * uinput.add(new Character('U')); event.consume(); return; } } else { char
	 * c=event.getKeyChar(); if( event.isAltDown() && event.isShiftDown() &&
	 * (('0'<=c && c<='9') ||('A'<=c && c<='F')) ){ if(uinput.size()<5){
	 * uinput.add(new Character(c)); return; } } else{ uinput.clear(); }
	 * 
	 * } break; case KeyEvent.KEY_RELEASED: case KeyEvent.KEY_TYPED:
	 * if(uinput.size()>0){ if(uinput.size()==5 &&
	 * event.getKeyCode()==KeyEvent.VK_ALT){ super.dispatchEvent(event); char[]
	 * chars=new char[4]; for(int i=1;i<5;i++) chars[i-1]=((Character)
	 * uinput.get(i)).charValue(); int hexv=Integer.parseInt(new
	 * String(chars),16); char uchar=(char) hexv; uinput.clear();
	 * this.dispatchUnicode(uchar); return; } Character uchr=(Character)
	 * uinput.get(uinput.size()-1); if(event.getKeyChar()==uchr.charValue()){
	 * if(uinput.size()==5 && event.getID()==KeyEvent.KEY_TYPED) {
	 * event.consume(); return; } }
	 * 
	 * } break; } super.dispatchEvent(event);
	 * 
	 * }
	 * 
	 * protected void dispatchEvent(AWTEvent event) { // Event Monitor
	 * System.out.println("dispatchEvent: "+event); switch(event.getID()){ case
	 * KeyEvent.KEY_PRESSED: case KeyEvent.KEY_RELEASED: case
	 * KeyEvent.KEY_TYPED: handleKey((KeyEvent) event); break; default:
	 * super.dispatchEvent(event); } }
	 */
}
