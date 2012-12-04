CSS: style0.css

Versor-with-AlloCore
===

This document describes installation of libvsr for integration with AlloCore.  For an introduction to Versor itself use please see the README.md file in the VSR repo

* [Download](#download)
* [Installation](#installation)

---

Download
---

Download Versor to the same directory where you have AlloSystem, Gamma, GLV, etc.  Assuming that directory is /code:

	cd code
	git clone git://github.com/wolftype/vsr.git

To incorporate it into AlloSystem, you do not need to make Versor from here.  If you have, in order to test it on your system, you might want to clean it before
installing via AlloSystem's makefile

	cd vsr
	make clean
	cd ..

---

Installation
---

Compile versor library FROM THE ALLOSYSTEM DIRECTORY.  This will install it to AlloSystem/build folder

	cd AlloSystem
	make vsr
	make allovsr

This will take a while to compile the first time round.  You may want to close memory hogging applications first (like xcode), though its unclear if that makes a difference.
Thereafter, the precompiled header (.gch file) that is created and installed should speed things up drastically.  It is a really big PCH file, 650mb or so!  
  
Any thoughts on shrinking that PCH would be welcome.  For now, though, it seems to have no side effect except to take up some space on your laptop.
You can always delete the file (with the expense of much longer compile times).

There are currently a lot of warnings that get thrown around during compilation.  I'm aware of them and need to abolish them . . . Doesn't effect performance for now though.

Try one of the examples:

	cd allovsr
	make examples/interface.cpp

[**BUILT-IN INTERFACE**]  
|                              | 	                                          |  
Key                            | Response  
------------------------------ | ------------------------------------------  
`~`                            | Toggle full screen.  
`SHIFT` + `Arrow Keys`         | Navigate the camera  in x and z directions.  
`SHIFT`+`CONTROL`+`Arrow Keys` | Navigate the camera in Y direction  
`OPTION`+`Arrow Keys`          | Spin the model view around.  
`G`                            | Grab and move Element with Mouse  
`R`                            | Rotate Element with Mouse  
`S`                            | Scale Element with Mouse  
`Q`                            | Let Go of All Selected Elements  


Email me with problems / questions / concerns / discussion items / ideas 


 