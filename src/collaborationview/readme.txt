Buzé Collaboration View - client and server for working with remote projects

Unpack the zip contents to the Buzé program folder (typically 
%PROGRAMFILES%\Buze)

- Gear/Views/collaborationview.dll
  Collaboration client user interface plugin for Buzé.

- armserve.exe
  Win32 TCP server application binary for hosting Armstrong projects. The
  server can be compiled for Linux and OSX as well. armserve is kept in the
  Armstrong source code repository.


armserve.exe is hardcoded to listen for TCP connections on port 8834.

armserve.exe stores each project in a directory under %APPDATA%\armserve on
Windows and ~/.armserve on POSIX.

Multiple users can connect to and edit the same project, and the server keeps
all connected instances of Buzé in sync.


Conflict handling:
Conflicts are detected by handling foreign key violations in the armstrong
database. Whenever a user tries to make an edit causing a foreign key
violation, the whole edit is rolled back on both the server and the client
that tried to make the edit. 


Versioning:
armserve.exe requires all clients to use the same armz song database version as
armserve was compiled with. There is currently no checking in place, and mixing
versions could result in loss of data.

When upgrading armserve, projects are automatically upgraded to latest version,
requiring all clients to upgrade as well.

