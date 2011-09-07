package com.amazon.ebook.booklet.msp;

import java.awt.event.KeyEvent;
import java.io.UnsupportedEncodingException;
import java.util.List;

import com.lab126.mario.c;
/*A D-Bus based interface to communicate with external input method
 * the interface name is com.tjw.uinput,
 * available signals and input values:
 * 	uhex:  int32[]  
 * 	utf8:  utf8 encoded string
 *  unicode: ucs2 encode string
 *  keypressed: modifier keycodes keychar
 *  keyreleased: modifier keycodes keychar
 */
//  extends class c so that our class can  register into kindle's
//  dbus message dispatch system
public class DbusEventHandler extends c {
	MyEventQueue myevq;
	public DbusEventHandler(MyEventQueue myevq) {
		// subscribe d-bus event com.tjw.uinput.*
		super("com.tjw.uinput");
		this.myevq=myevq;
	}	
	// event handler
	public void a(String interface_name, String signal, List values) {
		super.a(interface_name, signal, values);		
		try{
		String ustr=null;		
		if(signal.equals("uhex") && values.size()>0){
			char[] carray=new char[values.size()];
			for(int i=0;i<values.size();i++)
				carray[i]=(char) (((Integer) values.get(i)).intValue());
			ustr=new String(carray);
		}				
		else if(signal.equals("utf8")){
				String utf8= (String) values.get(0);
				try{
					ustr = new String(utf8.getBytes(), "UTF-8");
				}
				catch(UnsupportedEncodingException e){
					ustr="";
				}
		}
		else if(signal.equals("unicode")) ustr=((String) values.get(0));
		else if(signal.equals("keypressed") || signal.equals("keyreleased")){
			System.out.println("aaaa");
			int mod=0;
			char c=KeyEvent.CHAR_UNDEFINED;			
			int code=((Integer) values.get(0)).intValue();
			if(values.size()>1)	c=(char) ((Integer) values.get(1)).intValue();			
			if(values.size()>2)	mod=((Integer) values.get(2)).intValue();
			myevq.dispatchKeyEvent(
					signal.equals("keypressed") ? KeyEvent.KEY_PRESSED :KeyEvent.KEY_RELEASED 
					,code, c, mod);		
			return;
		}
		else return;
		if(ustr!=null){	
			for(int i=0;i< ustr.length();i++)
				myevq.dispatchUnicode(ustr.charAt(i));					
		}
		}
		catch(Exception ex){
			System.out.println("dbus exception:"+interface_name+" "+signal+" "+values);
		}
				
	}


}
