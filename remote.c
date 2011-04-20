#include "remote.h"
#include "common.h"
#include "helper.h"

#include <vdr/remote.h>


cDBusMessageRemote::cDBusMessageRemote(cDBusMessageRemote::eAction action, DBusConnection* conn, DBusMessage* msg)
:cDBusMessage(conn, msg)
,_action(action)
{
}

cDBusMessageRemote::~cDBusMessageRemote(void)
{
}

void cDBusMessageRemote::Process(void)
{
  switch (_action) {
    case dmrEnable:
      Enable();
      break;
    case dmrDisable:
      Disable();
      break;
    case dmrStatus:
      Status();
      break;
    }
}

void cDBusMessageRemote::Enable(void)
{
  cRemote::SetEnabled(true);
  cDBusHelper::SendReply(_conn, _msg, 250, "Remote control enabled");
}

void cDBusMessageRemote::Disable(void)
{
  cRemote::SetEnabled(false);
  cDBusHelper::SendReply(_conn, _msg, 250, "Remote control disabled");
}

void cDBusMessageRemote::Status(void)
{
  int enabled = 0;
  if (cRemote::Enabled())
     enabled = 1;
  DBusMessage *reply = dbus_message_new_method_return(_msg);
  DBusMessageIter args;
  dbus_message_iter_init_append(reply, &args);

  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &enabled))
     esyslog("dbus2vdr: %s.Status: out of memory while appending the status", DBUS_VDR_TIMER_INTERFACE);

  dbus_uint32_t serial = 0;
  if (!dbus_connection_send(_conn, reply, &serial))
     esyslog("dbus2vdr: %s.Status: out of memory while sending the reply", DBUS_VDR_TIMER_INTERFACE);
  dbus_message_unref(reply);
}


cDBusDispatcherRemote::cDBusDispatcherRemote(void)
:cDBusMessageDispatcher(DBUS_VDR_REMOTE_INTERFACE)
{
}

cDBusDispatcherRemote::~cDBusDispatcherRemote(void)
{
}

cDBusMessage *cDBusDispatcherRemote::CreateMessage(DBusConnection* conn, DBusMessage* msg)
{
  if ((conn == NULL) || (msg == NULL))
     return NULL;

  const char *object = dbus_message_get_path(msg);
  if ((object == NULL) || (strcmp(object, "/Remote") != 0))
     return NULL;

  if (dbus_message_is_method_call(msg, DBUS_VDR_REMOTE_INTERFACE, "Enable"))
     return new cDBusMessageRemote(cDBusMessageRemote::dmrEnable, conn, msg);

  if (dbus_message_is_method_call(msg, DBUS_VDR_REMOTE_INTERFACE, "Disable"))
     return new cDBusMessageRemote(cDBusMessageRemote::dmrDisable, conn, msg);

  if (dbus_message_is_method_call(msg, DBUS_VDR_REMOTE_INTERFACE, "Status"))
     return new cDBusMessageRemote(cDBusMessageRemote::dmrStatus, conn, msg);

  return NULL;
}

bool          cDBusDispatcherRemote::OnIntrospect(DBusMessage *msg, cString &Data)
{
  if (strcmp(dbus_message_get_path(msg), "/Remote") != 0)
     return false;
  Data =
  "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
  "       \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
  "<node>\n"
  "  <interface name=\""DBUS_VDR_REMOTE_INTERFACE"\">\n"
  "    <method name=\"Enable\">\n"
  "      <arg name=\"replycode\" type=\"i\" direction=\"out\"/>\n"
  "      <arg name=\"message\"   type=\"s\" direction=\"out\"/>\n"
  "    </method>\n"
  "    <method name=\"Disable\">\n"
  "      <arg name=\"replycode\" type=\"i\" direction=\"out\"/>\n"
  "      <arg name=\"message\"   type=\"s\" direction=\"out\"/>\n"
  "    </method>\n"
  "    <method name=\"Status\">\n"
  "      <arg name=\"status\"    type=\"b\" direction=\"out\"/>\n"
  "    </method>\n"
  "  </interface>\n"
  "</node>\n";
  return true;
}