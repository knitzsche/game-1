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

void addCirclePrevious(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs, std::shared_ptr<Gun> gun, double x, double y) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
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

void moveCircle(std::shared_ptr<Circle> t) {
	t->prevP.y = t->p.y;
	t->p.y = t->p.y+1;
	wrap(t);
}

double getDistanceMove(shared_ptr<Circle> c) {
	return sqrt(pow((c->p.x - c->prevP.x), 2) + pow((c->p.y - c->prevP.y), 2));
}

double getDistance(Position &p1, Position &p2) {
	return sqrt(pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2));
}

pair<Position, Position> getNextPosition(shared_ptr<Circle> c) {
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

void moveCircleTrajectory(std::shared_ptr<Circle> c) {
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

bool hit(std::shared_ptr<Circle> b, std::shared_ptr<Circle> t) {
	//see if bullet is inside square around target
	/*
	double halfR = t->r/2;
	if (b->p.x > t->p.x - halfR && 
		b->p.x < t->p.x + halfR &&
		b->p.y > t->p.y - halfR &&
		b->p.y < t->p.y + halfR ) {

		return true;
	}
	*/

	//Position nextP = getNextPosition(b);

	// Pa is initial position of bullet
	// Pb is initial position of target
	// Pab = Pa - Pb which is position of bullet minus target - VECTOR
	// Va is velocity of bullet, which is really the distance to next position, since v = d/t and t=1
	// Vb is velocity of target, as above
	// Vab = Va - Vb which is velocity of bullet minus targeti - a VECTOR
	// a = ( Vab * Vab ) where that is dot product
	// b = 2 (Pab * Vab) a dot product
	// c = Pab * Pab - (Ra -Rb ) ^ 2 cntains a dot product
	

	//Position bNext = b->p;
	Position bNext = getNextPosition(b).first;
	Position tNext = getNextPosition(t).first;
	//Position tNext = t->p;

	cout << "bNext x,y: "<< bNext.x << "," << bNext.y << endl;
	cout << "tNext x,y: "<< tNext.x << "," << tNext.y << endl;
	Position Pab;
	//Pab.x = abs(b->p.x - bNext.x);
	//Pab.y = abs(t->p.y - tNext.y);
	Pab.x = abs(b->p.x - t->p.x);
	Pab.y = abs(b->p.y - t->p.y);
	vector<double> Pab_v = {Pab.x, Pab.y};

	//Position bNext = b->p;
	//Position tNext = t->p;

	// if bullet next is not passed target x, no hit
	if (bNext.x < tNext.x)
		return false;

	Position Vab;
	Vab.x = abs(bNext.x - tNext.x);
	Vab.y = abs(bNext.y - tNext.y);
	vector<double> Vab_v = {Vab.x, Vab.y};

	int a = inner_product(Vab_v.begin(), Vab_v.end(), Vab_v.begin(), 0);
	int b_ = 2 * (inner_product(Pab_v.begin(), Pab_v.end(), Vab_v.begin(), 0));
	int c = inner_product(Pab_v.begin(), Pab_v.end(), Pab_v.begin(), 0) - pow(b->r + t->r, 2); 

	double discrim = pow(b_, 2) - (4 * a * c);

	bool hit = false;
	if (discrim > 0) { 
		cout << " POS: " << discrim << endl;
		hit = true;
	} else if (discrim ==  0) { 
		cout << "ZERO: " << discrim << endl;
		hit = true;
	}
	
	if (hit)
		return true;
	else
		return false;	




	//double Va = getDistanceMove(b);
	//double Vb = getDistanceMove(t);
	
/*
	vector<int> Va = {b->prevP.x, b->prevP.y, b->p.x, b->p.y};
	vector<int> Vb = {t->prevP.x, t->prevP.y, t->p.x, t->p.y};
*/

	//double Vab = inner_product(Vb.begin(), Vb.end(), ;
	//Vab = inner_product
		

	//a = inner_product(

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
	numPoints = dofB/t->r;
	for (int i = numPoints; i >0; i--) {
		int x; int y; int d;
		x = abs(b->p.x - t->p.x);
		y = abs(b->p.y - t->p.y);
		d = sqrt(pow(x,2) + pow(y,2));
		//cout << "x: " << x << endl;
		//cout << "y: " << y << endl;
		//cout << "d: " << d << endl;
		if (d > t->r) {
			return false;
		} else {
			//cout << "HIT. r: " << t->radius << endl;		
			//cout << "HIT. d: " << d << endl;		
			cout << "=== HIT" << endl;		
			return true;
		}
	} 
}

std::shared_ptr<Circle> makeTarget(){
	std::shared_ptr<Circle> target = std::make_shared<Circle>();
	target->p.x = SCREEN_WIDTH-80;
	target->p.y = 10;
	target->prevP.x = target->p.x;
	target->prevP.y = target->p.y-1;
	target->r = 20;
	target->rgb.r = 20;
	target->rgb.g = 20;
	target->rgb.b = 200;
	target->rgb.a = 255;
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
	std::shared_ptr<Circle> target;
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
		filledCircleRGBA(renderer, gun->x, gun->y, 10, 200, 10, 10, target->rgb.a);

		//Render bullets
		shared_ptr<vector<shared_ptr<Circle>>> newCircles = make_shared<vector<shared_ptr<Circle>>>(); 
		for( std::shared_ptr<Circle> &c : *circles ) {
			SDL_SetRenderDrawColor( renderer, c->rgb.b, c->rgb.g, c->rgb.r, c->rgb.a);
			
			int res = filledCircleRGBA(renderer, c->p.x, c->p.y, c->r, c->rgb.r, c->rgb.g, c->rgb.b, c->rgb.a);
			if (res == -1) 
				cout << "=========== ERROR res: " << res << endl;
			moveCircleTrajectory(c);
			if (c->p.x < SCREEN_WIDTH && c->p.x > 0 && c->p.y < SCREEN_HEIGHT && c->p.y > 0 ){
				newCircles->emplace_back(c); // keep if still on screen
			}
		}
		circles = newCircles;

		// render target
		if (isFlash) { 
			flashCount++;
			if (flashCount >= 30) {
				flashCount = -1;
				isFlash=false;
				target = makeTarget();
			}
			
			filledCircleRGBA(renderer, target->p.x, target->p.y, target->r, flash.r-100, flash.g-100, flash.b, target->rgb.a);
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
				filledCircleRGBA(renderer, target->p.x, target->p.y, target->r, flash.r-100, flash.g-100, flash.b, target->rgb.a);
				//std::cout << " a HIT!" << std::endl;
			} else {
				filledCircleRGBA(renderer, target->p.x, target->p.y, target->r, target->rgb.r, target->rgb.g, target->rgb.b, target->rgb.a);
			}
		}
		moveCircleTrajectory(target);
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
