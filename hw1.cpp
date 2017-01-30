//Modified By: Robert Ripley
//Date: 1-6-17


//Author: Gordon Griesel
//Date: 2014-Present
//All Rights Resevered
//cs3350 Spring 2017 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 5000
#define GRAVITY 0.01
#define rnd() (float)rand() / (float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Bubbler {
	Shape box;
	bool  isOn;
};

struct Game {
	Shape box;
	Bubbler bubbler;
	Particle particle[5000];
	int n;
	Game() {n=0;}
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
bool checkBubblerClick(Game* game, int x, int y);
void check_mouse(XEvent *e, Game *game);
int  check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void makeParticle(Game *game, int x, int y);

bool isOn;

int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	Bubbler bubbler;
	game.bubbler = bubbler;
	game.n=0;

	//init Bubbler
	game.bubbler.box.width = 25;
	game.bubbler.box.height = 10;
	game.bubbler.box.center.x = 120 + 5*65;
	game.bubbler.box.center.y = 500 - 5*60 + WINDOW_HEIGHT/3;

	//declare a box shape
	game.box.width = 100;
	game.box.height = 10;
	game.box.center.x = 120 + 5*65;
	game.box.center.y = 500 - 5*60;

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}

		if (isOn) {
			makeParticle(&game, 120 + 5*65, 500 - 5*60 +WINDOW_HEIGHT/3);		
		}

		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = -.05;
	p->velocity.x =  (rnd() * 0.05);
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			//makeParticle(game, e->xbutton.x, y);
			checkBubblerClick(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		//int y = WINDOW_HEIGHT - e->xbutton.y;
		for (int i = 0; i < 10; i++) {
			//makeParticle(game, e->xbutton.x, y);
		}

		if (++n < 10)
			return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.



	}
	return 0;
}

void movement(Game *game)
{

    for (int i = 0; i < game->n; i++) {

	Particle *p;

	if (game->n <= 0)
		return;

	p = &game->particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;

	//check for collision with shapes...
	//Shape *s;

	//check for off-screen
	if (p->s.center.y < 0.0) {
		std::cout << "off screen" << std::endl;
		game->particle[i] = game->particle[game->n-1];
		game->n -= 1;
	}

	//Check for Box Collide
	int xSpot   = p->s.center.x;
	int ySpot   = p->s.center.y; 
	int boxY    = game->box.center.y;
	int boxX    = game->box.center.x;
	int bHeight = 10;
	int bWidth  = 100;	

	if (xSpot > boxX-bWidth && xSpot < boxX+bWidth) {

		if (ySpot > boxY-bHeight && ySpot < boxY+bHeight) {
			p->velocity.y = -p->velocity.y/10;
		}

	}

	//Apply Gravity
	p->velocity.y -= .0001;

    }

}

bool checkBubblerClick(Game *game, int x, int y) {
	
	float boxX, boxY, width, height;   
	boxX   = game->bubbler.box.center.x;
	boxY   = game->bubbler.box.center.y;
	width  = game->bubbler.box.width;
	height = game->bubbler.box.height; 

	if (x > boxX - width && x < boxX + width) {

		if (y > boxY - height && y < boxY + height) {

			if (isOn) {
				std::cout << "Turn Off \n";
				isOn = false;
			}

			else {
				std::cout << "Turn On \n";
				isOn = true;
			}

			return true;
		}
	
	}

	return false;

}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	s = &game->box;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();

	//draw box
	Shape *s1;
	glColor3ub(90,0,90);
	s1 = &game->bubbler.box;
	glPushMatrix();
	glTranslatef(s1->center.x, s1->center.y, s1->center.z);
	w = s1->width;
	h = s1->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();

	//draw all particles here
	glPushMatrix();
	glColor3ub(150,160,220);

    for (int i = 0; i < game->n; i++) {

	Vec *c = &game->particle[i].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);

    }

	glEnd();
	glPopMatrix();
}



