#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
static bool name_registered=false;
static DBusConnection* conn=NULL;
static void init_dbus() {
   DBusError err;
   dbus_error_init(&err);
   conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
   if (dbus_error_is_set(&err)) {
      	fprintf(stderr, "Connection Error (%s)\n", err.message);
   }
   dbus_error_free(&err);
}
static void register_name(){
   int ret;
   DBusError err;
   dbus_error_init(&err);
   ret = dbus_bus_request_name(conn, "com.tjw.chewing", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
   if (dbus_error_is_set(&err)) {
      fprintf(stderr, "Name Error (%s)\n", err.message);
      dbus_error_free(&err);
   }
   if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
      perror("dbus: not the owner?");     
   }
   name_registered=true;
}
void sendutf8(char *utf8str)
{
   DBusMessage* msg;
   DBusMessageIter args;
   dbus_uint32_t serial = 0;
   DBusError err;
   if(conn==NULL) init_dbus();
   if(conn==NULL) {
		perror("dbus: no connection\n");
		return;}
   if(!name_registered)
	register_name();
   dbus_error_init(&err);
   msg = dbus_message_new_signal("/default", "com.tjw.uinput", "utf8");
   if (NULL == msg)
   {
      fprintf(stderr, "Message Null\n");
      return;
   }
   dbus_message_iter_init_append(msg, &args);
   if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &utf8str)) {
      fprintf(stderr, "Out Of Memory!\n");
      return;
   }
   // send the message and flush the connection
   if (!dbus_connection_send(conn, msg, &serial)) {
      fprintf(stderr, "Out Of Memory!\n");
      return;
   }
   //dbus_connection_flush(conn);
   // free the message
   dbus_message_unref(msg);
}

void sendkey(int pressed, int keycode, int keychar, int mod)
{
   DBusMessage* msg;
   DBusMessageIter args;
   dbus_uint32_t serial = 0;
   DBusError err;
   if(conn==NULL) init_dbus();
   if(conn==NULL) {
		perror("dbus: no connection\n");
		return;}
   if(!name_registered)
	register_name();
   dbus_error_init(&err);
   msg = dbus_message_new_signal("/default", "com.tjw.uinput", pressed ? "keypressed" : "keyreleased");
   if (NULL == msg)
   {
      fprintf(stderr, "Message Null\n");
      return;
   }
   dbus_message_iter_init_append(msg, &args);
   if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &keycode)) {
      fprintf(stderr, "Out Of Memory!\n");
      return;
   }
   if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &keychar)) {
      fprintf(stderr, "Out Of Memory!\n");
      return;
   }
   if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &mod)) {
      fprintf(stderr, "Out Of Memory!\n");
      return;
   }
   if (!dbus_connection_send(conn, msg, &serial)) {
      fprintf(stderr, "Out Of Memory!\n");
      return;
   }
   dbus_message_unref(msg);
}
