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

struct Position {
	double x;
	double y;
};

struct RGB {
	int r;
	int g;
	int b;
	int a;
};

struct Circle {
	Position p;
	Position prevP;
	double prevY;
	int r = 5; // radius
	RGB rgb;
};

void addCircle(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->p.x = 20;
	c->prevP.x = 0;
	c->p.y = 80;
	c->prevP.y = 105;
	cs->emplace_back(c);
} 

struct Gun {
	int x;
	int y;
};

struct Target {
	Position p;
	Position prevP;
	//int x;
	//int y;
	int radius;
	int red;
	int green;
	int blue;
	int alpha;
};

void addCirclePrevious(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs, std::shared_ptr<Gun> gun, double x, double y) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->p.x = gun->x;
	//factor down
	c->prevP.x = gun->x - ((gun->x - x)/4);
	c->p.y = gun->y;
	//factor down 
	c->prevP.y = gun->y - ((gun->y - y)/4);
	c->rgb.b = 200;
	c->rgb.g = 20;
	c->rgb.r = 20;
	c->rgb.a = 255;
	cs->emplace_back(c);
} 

void wrap(shared_ptr<Target> t) {
	if (t->p.x > SCREEN_WIDTH) {
		t->p.x = 0;
	}
	if (t->p.y > SCREEN_HEIGHT) {
		t->p.y = 0;//TODO reconsider
	}
}

void moveCircle(std::shared_ptr<Target> t) {
	t->prevP.y = t->p.y;
	t->p.y = t->p.y+1;
	wrap(t);
}

double getDistanceMove(shared_ptr<Circle> c) {
	return sqrt(pow((c->p.x - c->prevP.x), 2) + pow((c->p.y - c->prevP.y), 2));
}

pair<Position, Position> getNextPosition(shared_ptr<Circle> c) {
	Position p;
	Position prevP;
	int g = 9.8;
 	int t = 1;
	double velocy = c->p.y - c->prevP.y;
	//int deltaY; deltaY =int((velocy) + (0.5 * g));
	double deltaY = velocy + 0.5;
	double projY = c->p.y + deltaY;
	double deltaX = c->p.x - c->prevP.x;
	//# set the new position
	prevP.x = c->p.x;
	prevP.y = c->p.y;
	p.x = c->p.x + deltaX;
	p.y = c->p.y + deltaY;
	
	pair<Position, Position> ps (p, prevP);
	// TODO wrap if needed
	return ps;
}

void moveCircleTrajectory(std::shared_ptr<Circle> c) {
	double distance = getDistanceMove(c);
	cout << "x: " << c->p.x << " y: "<< c->p.y << " Distance: " << distance<< endl;
	Position p;
	pair<Position, Position> ps = getNextPosition(c);
	c->p.x = ps.first.x;
	c->p.y = ps.first.y;
	c->prevP.x = ps.second.x;
	c->prevP.y = ps.second.y;
	return;
}

bool hit(std::shared_ptr<Circle> b, std::shared_ptr<Target> t) {

	//see if bullet is inside square around target
	double halfR = t->radius/2;
	if (b->p.x > t->p.x - halfR && 
		b->p.x < t->p.x + halfR &&
		b->p.y > t->p.y - halfR &&
		b->p.y < t->p.y + halfR ) {

		return true;
	}
	
	//Position nextP = getNextPosition(b);


	// a = ( Vab * Vab )
	// b = 2 (Pab * Vab)
	// c = Pab * Pab - (Ra -Rb ) ^ 2


	


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
	deltaXofB = abs(b->p.x - b->prevP.x);
	deltaYofB = abs(b->p.y - b->prevP.y);
	dofB = sqrt(pow(deltaXofB,2) + pow(deltaYofB,2));
	//cout << "dofb: " << dofB << endl;
	numPoints = dofB/t->radius;
	for (int i = numPoints; i >0; i--) {
		int x; int y; int d;
		x = abs(b->p.x - t->p.x);
		y = abs(b->p.y - t->p.y);
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
	target->p.x = SCREEN_WIDTH-80;
	target->p.y = 10;
	target->radius = 20;
	target->red = 20;
	target->green = 20;
	target->blue = 200;
	target->alpha = 255;
	return target;
}

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
			shared_ptr<vector<shared_ptr<Circle>>> newCircles = make_shared<vector<shared_ptr<Circle>>>(); 
			for( std::shared_ptr<Circle> &c : *circles ) {
				SDL_SetRenderDrawColor( renderer, c->rgb.b, c->rgb.g, c->rgb.r, c->rgb.a);
				int result = filledCircleColor(renderer, c->p.x, c->p.y, c->r, 0xFF0000FF);
				moveCircleTrajectory(c);
				if (c->p.x < SCREEN_WIDTH && c->p.x > 0 && c->p.y < SCREEN_HEIGHT && c->p.y > 0 ){
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
			
			filledCircleRGBA(renderer, target->p.x, target->p.y, target->radius, flash.r-100, flash.g-100, flash.b, target ->alpha);
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
				filledCircleRGBA(renderer, target->p.x, target->p.y, target->radius, target->red, target->green, target->blue, target ->alpha);
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
