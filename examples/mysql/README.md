### AlloSystem/examples/mysql ###

This example shows how to query a MySQL database from an AlloSystem project. It
uses a special file called `flags.txt` to tell the build system to include and
link the [mysql++][] library. You must execute `install_dependencies.sh` to install
[mysql++][] and generate a `flags.txt` for your system.

+ Files:
  * `mysql.cpp` is the source code for this example.
  * [Inconsolata.otf][] is a nice font.
  * `install_dependencies.sh` installs [mysql++][] and generates `flags.txt`.
  * `flags.txt` (generated) tells the build system to include and link [mysql++][].

[mysql++]: http://tangentsoft.net/mysql++/
[Inconsolata.otf]: http://levien.com/type/myfonts/inconsolata.html
