#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <math.h>
#include <cmath>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h> //install libsdl2-gfx-dev
#include "res_path.h"
#include "cleanup.h"

using namespace std;
const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

/*
 * Log an SDL error with some error message to the output stream of our choice
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}

void addRect(std::shared_ptr<std::vector<std::shared_ptr<SDL_Rect>>> rs) {

	std::shared_ptr<SDL_Rect> r = std::make_shared<SDL_Rect>();
	r->x = 0;
	r->y = 0;
	r->w = 50;
	r->h = 50;
	rs->emplace_back(r);
} 

struct Circle {
	double initX= 20;
	double initPrevX = 0;
	double initY = 80;
	double initPrevY = 105;
	double x;
	double prevX;
	double y;
	double prevY;
	int r = 5; // radius
};

void addCircle(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->x = c->initX;
	c->prevX = c->initPrevX;
	c->y = c->initY;
	c->prevY = c->initPrevY;
	cs->emplace_back(c);
} 


struct Gun {
	int x;
	int y;
};

struct Target {
	int x;
	int y;
	int radius;
	int red;
	int green;
	int blue;
	int alpha;
};

void addCirclePrevious(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs, std::shared_ptr<Gun> gun, double x, double y) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->x = gun->x;
	//factor down
	c->prevX = gun->x - ((gun->x - x)/4);
	c->y = gun->y;
	//factor down 
	c->prevY = gun->y - ((gun->y - y)/4);
	cs->emplace_back(c);
} 

void moveRectangle(std::shared_ptr<SDL_Rect> r) {
	r->x = r->x+2;
	r->y = r->y+2;
}

void wrap(shared_ptr<Target> t) {
	if (t->x > SCREEN_WIDTH) {
		t->x = 0;
	}
	if (t->y > SCREEN_HEIGHT) {
		t->y = 0;//TODO reconsider
	}

}

void moveCircle(std::shared_ptr<Target> t) {
	t->y = t->y+1;
	wrap(t);
}


struct Position {
	double x;
	double prevX;
	double y;
	double prevY;
};

Position getNextPosition(shared_ptr<Circle> c) {
	Position p;
	int g = 9.8;
 	int t = 1;
	double velocy = c->y - c->prevY;
	//int deltaY; deltaY =int((velocy) + (0.5 * g));
	double deltaY = velocy + 0.5;
	double projY = c->y + deltaY;
	double deltaX = c->x - c->prevX;
	//# set the new position
	p.prevX = c->x;
	p.prevY = c->y;
	p.x = c->x + deltaX;
	p.y = c->y + deltaY;
	// TODO wrap if needed
	return p;
}


void moveCircleTrajectory(std::shared_ptr<Circle> c) {
	Position p;
	p = getNextPosition(c);
	c->x = p.x;
	c->y = p.y;
	c->prevX = p.prevX;
	c->prevY = p.prevY;
	return;
}

bool hit(std::shared_ptr<Circle> b, std::shared_ptr<Target> t) {


	//see if bullet is inside square around target
	double halfR = t->radius/2;
	if (b->x > t->x - halfR && 
		b->x < t->x + halfR &&
		b->y > t->y - halfR &&
		b->y < t->y + halfR ) 
		return true;
	
	Position nextP = getNextPosition(b);


	// this only checks current pos. I need to check pos between
	// current and previous. one for each distance diameter of target
	// so calc distance travelled since previous
	// divide that by target diameter
	// get points along line for each diameter distance
	// and check them for hits
	
	int dofB;//distance bullet has travelled
	int deltaXofB;
	int deltaYofB;
	int numPoints;
	deltaXofB = abs(b->x - b->prevX);
	deltaYofB = abs(b->y - b->prevY);
	dofB = sqrt(pow(deltaXofB,2) + pow(deltaYofB,2));
	//cout << "dofb: " << dofB << endl;
	numPoints = dofB/t->radius;
	for (int i = numPoints; i >0; i--) {
		
		int x; int y; int d;
		x = abs(b->x - t->x);
		y = abs(b->y - t->y);
		d = sqrt(pow(x,2) + pow(y,2));
		//cout << "x: " << x << endl;
		//cout << "y: " << y << endl;
		//cout << "d: " << d << endl;
		if (d > t->radius) {
			return false;
		} else {
			//cout << "HIT. r: " << t->radius << endl;		
			//cout << "HIT. d: " << d << endl;		
			cout << "=== HIT" << endl;		
			return true;
		}
	} 
}

std::shared_ptr<Target> makeTarget(){
	std::shared_ptr<Target> target = std::make_shared<Target>();
	target->x = SCREEN_WIDTH-80;
	target->y = 10;
	target->radius = 20;
	target->red = 20;
	target->green = 20;
	target->blue = 200;
	target->alpha = 255;
	return target;
}

struct RGB {
	int r;
	int g;
	int b;
	int a;
};

int main(int, char**){
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	RGB bg;
	bg.r = 20; bg.g = 20; bg.b = 20; bg.a = 255;

	RGB flash;
	flash.r = 20; flash.g = 50; flash.b = 200; flash.a = 255;

	std::shared_ptr<Gun> gun = std::make_shared<Gun>();
	gun->x = 400;
	gun->y = SCREEN_HEIGHT - 200;

	// make a target
	std::shared_ptr<Target> target;
	target = makeTarget();

	//Setup our window and renderer, this time let's put our window in the center
	//of the screen
	SDL_Window *window = SDL_CreateWindow("Lesson 4", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	if (surface == nullptr){
		logSDLError(std::cout, "CreateSurface");
		cleanup(window, renderer);
		SDL_Quit();
		return 1;
	}

	// vector of dynamically created resctangles
	std::shared_ptr<std::vector<std::shared_ptr<SDL_Rect>>> rects = std::make_shared<std::vector<std::shared_ptr<SDL_Rect>>>();

	// vector of dynamically created resctangles
	std::shared_ptr<std::vector<std::shared_ptr<Circle>>> circles = std::make_shared<std::vector<std::shared_ptr<Circle>>>();

	SDL_Event e;
	bool quit = false;
	int mx = gun->x - 40; int my = gun->y + 40;
	bool isFlash = false;
	int flashCount = -1;
	int move = -1;
	while (!quit){
		SDL_SetRenderDrawColor( renderer, bg.r, bg.g, bg.b, 255 );
		SDL_RenderClear(renderer);
		while (SDL_PollEvent(&e)){
			// user closes the window
			if (e.type == SDL_QUIT){
				quit = true;
			}
			// user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN){
				//addRect(rects);
				//addCircle(circles);
				addCirclePrevious(circles, gun, e.button.x, e.button.y);
			}
			// user presses any key
			if (e.type == SDL_KEYDOWN){
				int amt = 8;
				//cout << "key: " << SDL_GetKeyName(e.key.keysym.sym) << endl;
				if (e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					mx -= amt;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					mx += amt;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_UP) {
					my -= amt;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					my += amt;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					addCirclePrevious(circles, gun, mx, my);
				} else if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					quit = true;
				}
			}
			/*
			if (e.type == SDL_MOUSEMOTION){
				//std::cout << " MOUSE MOTION. x: " << e.motion.x << std::endl;
				mx = e.motion.x; my = e.motion.y;
				//std::cout << " MM res: " << res << std::endl;
			}
			*/
		}

		// render gun
		SDL_SetRenderDrawColor( renderer, 200, 100, 200, 255 );
		SDL_RenderDrawLine(renderer, mx, my, gun->x, gun ->y);
		filledCircleRGBA(renderer, gun->x, gun->y, 10, 200, 10, 10, target ->alpha);

		//Render bullets
		move++;
		if (move = 3) { // every other loop iter
		move = -1;
			SDL_SetRenderDrawColor( renderer, 200, 200, 0, 255 );
			shared_ptr<vector<shared_ptr<Circle>>> newCircles = make_shared<vector<shared_ptr<Circle>>>(); 
			for( std::shared_ptr<Circle> &c : *circles ) {
				int result = filledCircleColor(renderer, c->x, c->y, c->r, 0xFF0000FF);
				moveCircleTrajectory(c);
				if (c->x < SCREEN_WIDTH && c->x > 0 && c->y < SCREEN_HEIGHT && c->y > 0 ){
					newCircles->emplace_back(c); // keep if still on screen
				}
			}
			circles = newCircles;
		}

		// render target
		if (isFlash) { 
			flashCount++;
			if (flashCount >= 30) {
				flashCount = -1;
				isFlash=false;
				target = makeTarget();
			}
			
			filledCircleRGBA(renderer, target->x, target->y, target->radius, flash.r-100, flash.g-100, flash.b, target ->alpha);
		} else {
			bool isHit;
			isHit = false;
			for (auto &c : *circles){
				if (hit(c, target)) {
					isHit = true;
				}
			}
			if (isHit) {
				isFlash = true;
				//std::cout << " a HIT!" << std::endl;
			} else {
				filledCircleRGBA(renderer, target->x, target->y, target->radius, target->red, target->green, target->blue, target ->alpha);
				//circleRGBA(renderer, target->x, target->y, target->radius, target->red, target->green, target->blue, target->alpha);
			}
		}
		moveCircle(target);
		//Update the screen
		SDL_RenderPresent(renderer);
		//SDL_Delay(10);
	}

	cleanup( renderer, window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}
