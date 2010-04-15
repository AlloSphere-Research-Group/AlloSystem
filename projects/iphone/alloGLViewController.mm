    //
//  alloGLView.m
//  allo
//
//  Created by Graham Wakefield on 4/14/10.
//  Copyright 2010 UCSB. All rights reserved.
//

#import "alloGLViewController.h"

al::Graphics gl(al::GraphicsBackend::None);

@implementation alloGLViewController

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	al::setBackendOpenGLES1(&gl);
}

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

- (void)drawView; //:(alloGLView *)view
{
	NSLog(@".");
	
	gl.begin(gl.TRIANGLES);
		for (float x = -1; x<1.; x+=0.1) {
		for (float y = -1; y<1.; y+=0.1) {
		for (float z = -1; z<1.; z+=0.1) {
			gl.color(x+1, y+1, z+1);
			gl.vertex(x, y, z);
		}}}
	gl.end();
	
	
//    static      GLfloat rot = 0.0;
//	
//	glColor4f(0.0, 0.0, 0.0, 0.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	
//	rot += 0.5;
//	
//    glLoadIdentity();
//	glClearColor(r, g, b, 1.0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	
//	//glRotatef(rot, 0., 0., 1.);
//	
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glColor4f(1.0, 1.0, 1.0, 1.0);
//    glVertexPointer(3, GL_FLOAT, 0, vertices);
//    glDrawArrays(GL_LINE_LOOP, 0, numvertices*3);
//    glDisableClientState(GL_VERTEX_ARRAY);
    
}

-(void)setupView:(alloGLView*)view
{
//	const GLfloat zNear = 0.01, zFar = 1000.0, fieldOfView = 45.0; 
//	GLfloat size; 
//	glEnable(GL_DEPTH_TEST);
//	glMatrixMode(GL_PROJECTION); 
//	size = zNear * tanf(DEGREES_TO_RADIANS(fieldOfView) / 2.0); 
//	CGRect rect = view.bounds; 
//	glFrustumf(-size, size, -size / (rect.size.width / rect.size.height), size / 
//			   (rect.size.width / rect.size.height), zNear, zFar); 
//	glViewport(0, 0, rect.size.width, rect.size.height);  
//	glMatrixMode(GL_MODELVIEW);
//	
//	glLoadIdentity(); 
//	
	
}

@end
