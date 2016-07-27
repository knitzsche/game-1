#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <math.h>
#include <cmath>
#include <ctime>
#include <numeric>

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
	int r = 5; // radius
	RGB rgb;
};

class  MovingCircle : public Circle {
public:
	Position prevP;
	double prevY;
};

class  Ripple : public Circle {
public:
    int expand_speed;
};

void addRipple(std::shared_ptr<std::vector<std::shared_ptr<Ripple>>> cs, int const& x = 20, int const& y = 80, int const& r = 100, int const& g = 200, int const& b = 200, int const& a = 200, int const& expand_speed = 5) {
    //cout << " in addCircle(). x: " << x << " y: " << y << endl;
	std::shared_ptr<Ripple> c = std::make_shared<Ripple>();
	c->p.x = x;
	c->p.y = y;
	c->rgb.r = r;
	c->rgb.g = g;
	c->rgb.b = b;
	c->rgb.a = a;
	c->expand_speed = expand_speed;
	cs->emplace_back(c);
}

void addCircle(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs, int x = 20, int y = 80, int r = 100, int g = 200, int b = 200, int a = 200) {
    //cout << " in addCircle(). x: " << x << " y: " << y << endl;
	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->p.x = x;
	c->p.y = y;
	c->rgb.r = r;
	c->rgb.g = g;
	c->rgb.b = b;
	c->rgb.a = a;
	cs->emplace_back(c);
} 

void addMovingCircle(std::shared_ptr<std::vector<std::shared_ptr<MovingCircle>>> cs) {
	std::shared_ptr<MovingCircle> c = std::make_shared<MovingCircle>();
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

void addMovingCirclePrevious(std::shared_ptr<std::vector<std::shared_ptr<MovingCircle>>> cs, std::shared_ptr<Gun> gun, double x, double y) {
	std::shared_ptr<MovingCircle> c = std::make_shared<MovingCircle>();
	c->p.x = gun->x;
	//factor down
	c->prevP.x = gun->x - ((gun->x - x)/4);
	c->p.y = gun->y;
	//factor down 
	c->prevP.y = gun->y - ((gun->y - y)/4);
	c->rgb.b = 20;
	c->rgb.g = 20;
	c->rgb.r = 200;
	c->rgb.a = 255;
	cs->emplace_back(c);
} 

void wrap(shared_ptr<Circle> t) {
	if (t->p.x > SCREEN_WIDTH) {
		t->p.x = 0;
	}
	if (t->p.y > SCREEN_HEIGHT) {
		t->p.y = 0;//TODO reconsider
	}
}

void moveCircle(std::shared_ptr<MovingCircle> t) {
	t->prevP.y = t->p.y;
	t->p.y = t->p.y+1;
	wrap(t);
}

void growRipple(std::shared_ptr<Ripple> t) {
	t->r = t->r + t->expand_speed;
	//wrap(t);
}

double getDistanceMove(shared_ptr<MovingCircle> c) {
	return sqrt(pow((c->p.x - c->prevP.x), 2) + pow((c->p.y - c->prevP.y), 2));
}

double getDistance(Position &p1, Position &p2) {
	return sqrt(pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2));
}

pair<Position, Position> getNextPosition(shared_ptr<MovingCircle> c) {
	Position p;
	Position prevP;
	double velocy = c->p.y - c->prevP.y;
	//int deltaY; deltaY =int((velocy) + (0.5 * g));
	double deltaY = velocy; // use this for straight
	//double deltaY = velocy + 0.5; // comment this in for trajectory
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

void moveCircleTrajectory(std::shared_ptr<MovingCircle> c) {
	double distance = getDistanceMove(c);
	//cout << c->p.x << "," << c->p.y << endl;
	Position p;
	pair<Position, Position> ps = getNextPosition(c);
	c->p.x = ps.first.x;
	c->p.y = ps.first.y;
	c->prevP.x = ps.second.x;
	c->prevP.y = ps.second.y;
	return;
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
	// vector of moving moving_circles
	std::shared_ptr<std::vector<std::shared_ptr<MovingCircle>>> moving_circles = std::make_shared<std::vector<std::shared_ptr<MovingCircle>>>();

	// vector of grid circles 
	std::shared_ptr<std::vector<std::shared_ptr<Circle>>> grid_circles = std::make_shared<std::vector<std::shared_ptr<Circle>>>();

	// vector of ripples circles 
	std::shared_ptr<std::vector<std::shared_ptr<Ripple>>> ripples = std::make_shared<std::vector<std::shared_ptr<Ripple>>>();

    int x_iters = int(SCREEN_WIDTH/20);
    int  y_iters = int(SCREEN_HEIGHT/20);
    //cout << "x_iters: " << x_iters  << endl;
    //cout << "y_itrd: " << y_iters << endl;
    int  grid = 20;
    for ( int x=0; x <= x_iters; ++x)
    {
        for (int y=0; y <= y_iters; ++y)
        {
            //cout << "x: " << x << endl;
            //cout << "y: " << y << endl;
            addCircle(grid_circles, x * grid, y * grid, 200,200,200, 50);        
        }
    }

	SDL_Event e;
	bool quit = false;
	int mx = gun->x - 40; int my = gun->y + 40;
	bool isFlash = false;
	int flashCount = -1;
	int move = -1;

	while (!quit){

		clock_t startTime = clock();
		//cout << "start time: " << startTime << endl;; 
		//cout << "clocks per sec: " << CLOCKS_PER_SEC << endl; 
		//1000000 clocks per second.
		//float ellapsedSecs = (float)startTime/CLOCKS_PER_SEC;
		//cout << "elapsed: " << ellapsedSecs << endl;; 

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
		        addRipple(ripples, e.button.x, e.button.y);		
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
					addMovingCirclePrevious(moving_circles, gun, mx, my);
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
        //render grid cricles
		for( std::shared_ptr<Circle> &c : *grid_circles ) {
			SDL_SetRenderDrawColor(renderer, c->rgb.b, c->rgb.g, c->rgb.r, c->rgb.a);
			int res = filledCircleRGBA(renderer, c->p.x, c->p.y, c->r, c->rgb.r, c->rgb.g, c->rgb.b, c->rgb.a);
			if (res == -1) 
				cout << "=========== render grid circles ERROR res: " << res << endl;
        }
        //grow and render ripple cricles
        for( std::shared_ptr<Ripple> &c : *ripples ) {
            growRipple(c);
            SDL_SetRenderDrawColor(renderer, c->rgb.b, c->rgb.g, c->rgb.r, c->rgb.a);
            int res = circleRGBA(renderer, c->p.x, c->p.y, c->r, c->rgb.r, c->rgb.g, c->rgb.b, c->rgb.a);
            if (res == -1) 
                cout << "=========== render ripple ERROR res: " << res << endl;
            //TODO: remove ripple from vector if out of view, that is if radius > distance from center to
            // each of four corners
        }
		// render gun
		SDL_SetRenderDrawColor( renderer, 200, 100, 200, 255 );
		SDL_RenderDrawLine(renderer, mx, my, gun->x, gun ->y);
		//filledCircleRGBA(renderer, gun->x, gun->y, 10, 200, 10, 10, target->rgb.a);

		//Render bullets
		shared_ptr<vector<shared_ptr<MovingCircle>>> newCircles = make_shared<vector<shared_ptr<MovingCircle>>>(); 
		for( std::shared_ptr<MovingCircle> &c : *moving_circles ) {
			SDL_SetRenderDrawColor( renderer, c->rgb.b, c->rgb.g, c->rgb.r, c->rgb.a);
			
			int res = filledCircleRGBA(renderer, c->p.x, c->p.y, c->r, c->rgb.r, c->rgb.g, c->rgb.b, c->rgb.a);
			if (res == -1) 
				cout << "=========== ERROR res: " << res << endl;
			moveCircleTrajectory(c);
			if (c->p.x < SCREEN_WIDTH && c->p.x > 0 && c->p.y < SCREEN_HEIGHT && c->p.y > 0 ){
				newCircles->emplace_back(c); // keep if still on screen
			}
		}
		moving_circles = newCircles;

		//moveCircleTrajectory(target);
		//moveCircle(target);
		//Update the screen
		SDL_RenderPresent(renderer);
		//SDL_Delay(10);
		clock_t endTime = clock();
		clock_t ellapsedTime = endTime - startTime;
		float ellapsed = (float)ellapsedTime/CLOCKS_PER_SEC;
		//cout << "elapsed time: " << ellapsed << endl;; 
		if (ellapsed < 0.0333) // 30 franmes per second
			SDL_Delay(0.0333 - ellapsed);
	}

	cleanup( renderer, window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}
