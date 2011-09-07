package com.amazon.ebook.booklet.msp;

import java.awt.Component;
import java.awt.Toolkit;
import java.awt.event.KeyEvent;
import java.io.UnsupportedEncodingException;
import java.util.List;

import com.amazon.kindle.kindlet.ui.helper.AppContextBridge;
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
//  extends class c so that our class can be registered into kindle's
//  dbus message dispatch system
public class DBusEventHandler extends c {
	private Component defaultSource = null;

	class DefaultEvent extends KeyEvent {
		private static final long serialVersionUID = 1685297068658195797L;

		public DefaultEvent(int id, int modifiers, int keycode, char keychar) {
			super(defaultSource, id, System.currentTimeMillis(), modifiers,
					keycode, keychar);
		}

		public DefaultEvent(char keychar) {
			this(KeyEvent.KEY_TYPED, 0, KeyEvent.VK_UNDEFINED, keychar);
		}
	}

	class KeyEventRaiser implements Runnable {
		String ustr = null;
		KeyEvent keyevent = null;

		public KeyEventRaiser(KeyEvent event) {
			keyevent = event;
		}

		public KeyEventRaiser(String s) {
			ustr = s;
		}

		private void postEvent(KeyEvent ke) {
			Toolkit.getDefaultToolkit().getSystemEventQueue().postEvent(ke);
		}

		public void run() {
			if (ustr != null) {
				for (int i = 0; i < ustr.length(); i++)
					postEvent(new DefaultEvent(ustr.charAt(i)));
			} else
				postEvent(keyevent);

		}

	};

	private void postEvent(KeyEventRaiser re) {
		try {
			AppContextBridge bridge = AppContextBridge.getBridge();
			if (bridge != null)
				bridge.run(re);
		} catch (Exception ex) {
		}
		try {
			re.run();
		} catch (Exception ex) {
		}
	}

	public DBusEventHandler(Component src) {
		// subscribe d-bus event com.tjw.uinput.*
		super("com.tjw.uinput");
		this.defaultSource = src;
	}

	// event handler
	public void a(String interface_name, String signal, List values) {
		super.a(interface_name, signal, values);
		try {
			String ustr = null;
			if (signal.equals("uhex") && values.size() > 0) {
				char[] carray = new char[values.size()];
				for (int i = 0; i < values.size(); i++)
					carray[i] = (char) (((Integer) values.get(i)).intValue());
				ustr = new String(carray);
			} else if (signal.equals("utf8")) {
				String utf8 = (String) values.get(0);
				try {
					ustr = new String(utf8.getBytes(), "UTF-8");
				} catch (UnsupportedEncodingException e) {
					ustr = "";
				}
			} else if (signal.equals("unicode"))
				ustr = ((String) values.get(0));
			else if (signal.equals("keypressed")
					|| signal.equals("keyreleased")) {
				int mod = 0;
				char c = KeyEvent.CHAR_UNDEFINED;
				int code = ((Integer) values.get(0)).intValue();
				if (values.size() > 1)
					c = (char) ((Integer) values.get(1)).intValue();
				if (values.size() > 2)
					mod = ((Integer) values.get(2)).intValue();
				postEvent(new KeyEventRaiser(new DefaultEvent(signal
						.equals("keypressed") ? KeyEvent.KEY_PRESSED
						: KeyEvent.KEY_RELEASED, mod, code, c)));
				return;
			} else
				return;
			if (ustr != null)
				postEvent(new KeyEventRaiser(ustr));
		} catch (Exception ex) {
			System.out.println("dbus exception:" + interface_name + " "
					+ signal + " " + values);
		}

	}
}
