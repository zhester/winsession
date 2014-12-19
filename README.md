Windows Session Tool
====================

This project is intended to provide a very simple tool that helps to restore a
Windows session with as much automation and configuration as is reasonable.  A
_session_ is defined as the positions and dimensions of a set of application
windows that are present on the desktop.  Internal details/views of each
window may be customized (e.g. the directory of a Windows Explorer window).

There are two components to the session tool.

1. A configuration helper that helps to expose and record information about
   your current session.
2. A session restoration tool.  This is a small executable that is usually run
   from a shortcut with the name of a session passed to it.

Configuration
-------------

The session tool stores its configuration in the user's `%HOME%` directory.
The configuration file is a JSON-encoded file that can be managed manually
(using a text editor), or through the configuration helper.  The configuration
has several top-level sections to allow the user to customize its behavior and
to manage each window session.

### Configuration File Details

The following shows an example configuration with a single named session that
launches a single Windows Explorer window.

    {
        "config"   : {},
        "restore"  : {},
        "sessions" : {
            "filemanager" : [
                {
                    "name"      : "Windows Explorer",
                    "rectangle" : [ 0, 0, 1024, 768 ],
                    "command"   : "C:\\Windows\\explorer.exe",
                    "arguments" : [ "%UserProfile%" ]
                }
            ]
        }
    }

Restoration
-----------

The restoration tool is what _restores_ a session.  This program is run from
anywhere (a shortcut, command line, the "Run" box, etc.).  If no argument is
passed, and there is no default session name specified in the configuration,
the restoration tool starts the configuration helper.  When it is passed a
valid session name, it will attempt to re-create the configured session to the
best of its abilities.

My intent is to allow the creation of Windows shortcuts that can be placed in
the user's "Startup" program group.  This would allow a person to have a set
of windows that are automatically started and positioned each time they log
into their desktop.

Given the example configuration file above, launching the session involves
calling the restoration tool with the name of the session as follows.

    winsession.exe filemanager

This will launch the configured window, and place it in its configured
position with the specified dimensions.  Furthermore, it will pass the
special `%UserProfile%` directory to the program to open it in the user's home
directory.

Of course, this is the simplest example, and doesn't really help as much a
regular Windows shortcut.  The difference here is the `filemanager`
configuration item contains a list of windows that are launched.  Multiple
windows with various dimensions and arguments can be given here.

Additionally, the configuration can contain multiple sessions--each with their
own name.  The user can then create multiple shortcuts to individual sessions
and decide which ones to launch.

The restoration tool can also be passed "globbing" expressions to launch a set
of sessions in one command.

Launch all configured sessions at once:

    winsession.exe *

Launch all configured sessions whose names begin with "dev".

    winsession.exe dev*

