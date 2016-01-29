#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h> //install libsdl2-gfx-dev
#include "res_path.h"
#include "cleanup.h"


const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

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
	int x;
	int prevX;
	int y;
	int prevY;
	int r; // radius
};

void addCircle(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->x = 20;
	c->prevX = 0;
	c->y = 80;
	c->prevY = 105;
	c->r = 10;
	cs->emplace_back(c);
} 

void addCirclePrevious(std::shared_ptr<std::vector<std::shared_ptr<Circle>>> cs, int x, int y) {

	std::shared_ptr<Circle> c = std::make_shared<Circle>();
	c->x = 20;
	c->prevX = x;
	c->y = 80;
	c->prevY = y;
	c->r = 10;
	cs->emplace_back(c);
} 


void moveRectangle(std::shared_ptr<SDL_Rect> r) {
	r->x = r->x+2;
	r->y = r->y+2;
}

void moveCircle(std::shared_ptr<Circle> c) {
	c->x = c->x+2;
	c->y = c->y+2;
}

void moveCircleTrajectory(std::shared_ptr<Circle> c) {
	int g; g = 9.8;
 	int t; t = 1;
	int velocy; velocy = c->y - c->prevY;

	//int deltaY; deltaY =int((velocy) + (0.5 * g));
	int deltaY; deltaY =int ((velocy) + (0.25 * g));
	int projY; projY = c->y + deltaY;
	int deltaX; deltaX = c->x - c->prevX;

	//# set the new position
	c->prevX = c->x;
	c->prevY = c->y;
	c->x = c->x + deltaX;
	c->y = c->y + deltaY;
	// TODO wrap if needed
	return;
}

int main(int, char**){
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

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

	// a rect
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 32;
	rect.h = 32;

	// vector of dynamically created resctangles
	std::shared_ptr<std::vector<std::shared_ptr<SDL_Rect>>> rects = std::make_shared<std::vector<std::shared_ptr<SDL_Rect>>>();

	// vector of dynamically created resctangles
	std::shared_ptr<std::vector<std::shared_ptr<Circle>>> circles = std::make_shared<std::vector<std::shared_ptr<Circle>>>();

	SDL_Event e;
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&e)){
			// user closes the window
			if (e.type == SDL_QUIT){
				quit = true;
			}
			// user presses any key
			if (e.type == SDL_KEYDOWN){
				quit = true;
			}
			// user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN){
				//addRect(rects);
				//addCircle(circles);
				addCirclePrevious(circles, e.button.x, e.button.y);
			}
		}

		SDL_SetRenderDrawColor( renderer, 20, 20, 20, 255 );
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor( renderer, 200, 0, 200, 255 );
		for( std::shared_ptr<SDL_Rect> &r : *rects ) {
			moveRectangle(r);
			SDL_RenderFillRect( renderer, r.get() );
		}
		SDL_SetRenderDrawColor( renderer, 200, 200, 0, 255 );
		for( std::shared_ptr<Circle> &c : *circles ) {
			moveCircleTrajectory(c);
			int result = filledCircleColor(renderer, c->x, c->y, c->r, 0xFF0000FF);

		}
		rect.x++;
		rect.y++;
		SDL_RenderFillRect( renderer, &rect );

		//Update the screen
		SDL_RenderPresent(renderer);
	}

	cleanup( renderer, window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}
