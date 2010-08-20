#ifndef INCLUDE_AL_WINDOWGL_HPP
#define INCLUDE_AL_WINDOWGL_HPP

/*
 *	OpenGL window
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include <vector>
#include <string>

namespace al{

// can redefine, but should be at least 4
#ifndef AL_MOUSE_MAX_BUTTONS
#define AL_MOUSE_MAX_BUTTONS 4
#endif






/// Constants of keyboard keys.
namespace Key{
	enum t{
		
		// Standard ASCII non-printable characters
		Enter		=3,		/**< */
		BackSpace	=8,		/**< */
		Tab			=9,		/**< */
		Return		=13,	/**< */
		Escape		=27,	/**< */
		Delete		=127,	/**< */
			
		// Non-standard, but common keys.
		F1=256, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, 
		Insert, Left, Up, Right, Down, PageDown, PageUp, End, Home,
	};
}

namespace DisplayMode{
	enum t{
		SingleBuf	= 1<<0,		/**< Single-buffered */
		DoubleBuf	= 1<<1,		/**< Double-buffered */
		StereoBuf	= 1<<2,		/**< Do left-right stereo buffering */
		AccumBuf	= 1<<3,		/**< Use accumulation buffer */
		AlphaBuf	= 1<<4,		/**< Use alpha buffer */
		DepthBuf	= 1<<5,		/**< Use depth buffer */
		StencilBuf	= 1<<6,		/**< Use stencil buffer */
		Multisample = 1<<7,		/**< Multisampling support */
		DefaultBuf	= DoubleBuf|AlphaBuf|DepthBuf /**< Default display mode */
	};
	inline t operator| (const t& a, const t& b){ return t(int(a) | int(b)); }
	inline t operator& (const t& a, const t& b){ return t(int(a) & int(b)); }
}

namespace Cursor{
	enum t{
		None		= 0,
		Pointer		= 1,
		CrossHair,
	};
}


using namespace DisplayMode;


class Keyboard{
public:

	/// Constructor.
	Keyboard();
	
	int key() const;		///< Returns key code (non-shifted character) of last key event.
	int keyAsNumber() const;///< Returns decimal number correlating to key code
	bool alt() const;		///< Whether an alt key is down.
	bool caps() const;		///< Whether capslock is down.
	bool ctrl() const;		///< Whether a ctrl key is down.
	bool meta() const;		///< Whether a meta (e.g. windows, apple) key is down.
	bool shift() const;		///< Whether a shift key is down.
	bool down() const;		///< Whether last event was button down.
	bool isNumber() const;	///< Whether key is a number key
	bool key(int k) const;	///< Whether the last key was 'k'.

	void alt  (bool state);	///< Set alt key state.
	void caps (bool state);	///< Set alt key state.
	void ctrl (bool state);	///< Set ctrl key state.
	void meta (bool state);	///< Set meta key state.
	void shift(bool state);	///< Set shift key state.

	void print();			///< Print keyboard state to stdout.

private:
	friend class WindowImpl;

	int	mKeycode;			// last key event key number
	bool mDown;				// last key event state (pressed or released)
	bool mModifiers[5];		// Modifier key state array (shift, alt, ctrl, caps, meta)
	
	void setKey(int k, bool v);
};


class Mouse{
public:
	enum{
		Left	= 0,
		Middle	= 1,
		Right	= 2,
		Extra	= 3
	};
	
	Mouse();
	
	int x() const;
	int y() const;
	
	int button() const;
	bool down() const;
	bool down(int button) const;
	bool left() const;
	bool middle() const;
	bool right() const;
	
private:
	friend class WindowImpl;

	int mX, mY;							// x,y positions
	int mButton;						// most recent button changed
	int mBX[AL_MOUSE_MAX_BUTTONS];		// button down xs
	int mBY[AL_MOUSE_MAX_BUTTONS];		// button down ys
	bool mB[AL_MOUSE_MAX_BUTTONS];		// button states
	
	void button(int b, bool v);
	void position(int x, int y);
};

/// TODO: rename to Window

/// OpenGL Window interface
class WindowGL {
public:

	struct Dim{
		Dim(double w_, double h_, double l_=0, double t_=0): l(l_), t(t_), w(w_), h(h_){}
		Dim(double v=0): l(0), t(0), w(v), h(v){}
		double l,t,w,h;
	};

	WindowGL();
	virtual ~WindowGL();
	
	void create(
		const Dim& dim,
		const std::string title,
		double fps=40,
		DisplayMode::t mode = DisplayMode::DefaultBuf
	);
	
	void destroy();
	
	virtual void onCreate(){}
	virtual void onDestroy(){}
	virtual void onFrame(){}
	virtual void onResize(int w, int h){}
	virtual void onVisibility(bool v){}
	
	virtual void onMouseDown(const Mouse& m){}
	virtual void onMouseDrag(const Mouse& m){}
	virtual void onMouseMove(const Mouse& m){}
	virtual void onMouseUp(const Mouse& m){}
	
	virtual void onKeyDown(const Keyboard& k){}
	virtual void onKeyUp(const Keyboard& k){}
	
	bool cursorHide() const;
	Dim dimensions() const;
	bool enabled(DisplayMode::t v) const;				///<
	bool fullScreen() const;
	double fps() const;									///< Returns frames/second (requested)
	double avgFps() const;								///< Returns frames/second (running average)
	double spf() const { return 1./fps(); }				///< Returns seconds/frame
	const std::string& title() const;
	bool visible() const;
	const Keyboard& keyboard(){ return mKeyboard; }
	const Mouse& mouse(){ return mMouse; }

	void doFrame();										///< Calls onFrame() and swaps buffers

	WindowGL& cursor(Cursor::t v);						///< Set cursor type
	WindowGL& cursorHide(bool v);						///< Set cursor hiding
	WindowGL& cursorHideToggle();						///< Toggle cursor hiding
	WindowGL& dimensions(const Dim& v);
	WindowGL& fps(double v);							///< Set frames/second
	
	/// Set fullscreen mode
	
	/// This will make the window go fullscreen without borders and,
	/// if posssible, without changing the display resolution.
	WindowGL& fullScreen(bool on);
	WindowGL& fullScreenToggle();
	WindowGL& hide();
	WindowGL& iconify();
	WindowGL& makeActive();
	WindowGL& show();
	WindowGL& title(const std::string& v);

	/// Destroy all created windows
	static void destroyAll();

	static void startLoop();
	static void stopLoop();

private:
	friend class WindowImpl;

	class WindowImpl * mImpl;
	Keyboard mKeyboard;
	Mouse mMouse;
	
	void init();
};


//struct TimedFunction{
//	virtual ~TimedFunction(){}
//	void operator()(float ms);
//	virtual void onExecute(){}
//};


} // al::

#endif
