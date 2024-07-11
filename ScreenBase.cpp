#ifndef SCREENBASE_H
#include<SDL2/SDL.h>
#include<SDL2/SDL_main.h>
#include<SDL2/SDL_ttf.h>
#include<stdexcept>
#include<cmath>
using namespace std;

class ScreenBase
{
    private:
        int Width;
        int Height;
        SDL_Window* Window;
        SDL_Renderer* Renderer;
    public:
        ScreenBase(int W, int H): Width(W), Height(H), Window(nullptr), Renderer(nullptr) {}
        virtual ~ScreenBase();

        void init();
        void clear();
        void update();
        void drawRect(int LeftUpX,int LeftUpY, int width, int height, SDL_Color color);
        void drawSqur(int LeftUpX, int LeftUpY, int side, SDL_Color color);
        void drawCircle(int CenterX, int CenterY, double radius, SDL_Color color);
        void drawFullCircle(int CenterX, int CenterY, double radius, SDL_Color color);
        void close();
        int getWidth() const {return Width;}
        int getHeight() const {return Height;}
        SDL_Renderer* getRenderer() const {return Renderer;}
};

ScreenBase::~ScreenBase()
{
    close();
}

void ScreenBase::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) throw pair(runtime_error("Video Initialization Failed."), SDL_GetError());
    if(TTF_Init() == -1) throw std::pair<std::runtime_error, const char*>(std::runtime_error("TTF Initialization Failed."), TTF_GetError());
    Window = SDL_CreateWindow("ASCII Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_SHOWN);
    if(Window == nullptr) throw pair(runtime_error("Window Creation Failed."), SDL_GetError());
    Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
    if(Renderer == nullptr) throw pair(runtime_error("Renderer Creation Failed."), SDL_GetError());
}

void ScreenBase::clear()
{
    SDL_SetRenderDrawColor(Renderer, 0,0,0, 255); // Draw pure black on screen
    SDL_RenderClear(Renderer);
}

void ScreenBase::update()
{
    SDL_RenderPresent(Renderer);
}

void ScreenBase::drawRect(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color)
{
    SDL_SetRenderDrawColor(Renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {LeftUpX, LeftUpY, width, height};
    SDL_RenderFillRect(Renderer, &rect);
}

void ScreenBase::drawCircle(int CenterX, int CenterY, double radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(Renderer, color.r, color.g, color.b, color.a);
    for(int x = -radius; x <= radius; x++)
    {
        int y = sqrt(radius*radius - x*x);
        SDL_RenderDrawPoint(Renderer, CenterX + x, CenterY + y);
        SDL_RenderDrawPoint(Renderer, CenterX + x, CenterY - y);
    }
}

void ScreenBase::drawFullCircle(int CenterX, int CenterY, double radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(Renderer, color.r, color.g, color.b, color.a);
    for(int x = -radius; x <= radius; x++)
    {
        int y = sqrt(radius*radius - x*x);
        SDL_RenderDrawLine(Renderer, CenterX + x, CenterY + y, CenterX + x, CenterY - y);
    }
}

void ScreenBase::close()
{
    if(Renderer != nullptr) SDL_DestroyRenderer(Renderer), Renderer = nullptr;
    if(Window != nullptr) SDL_DestroyWindow(Window), Window = nullptr;
    SDL_Quit();
}

void ScreenBase::drawSqur(int LeftUpX, int LeftUpY, int side, SDL_Color color)
{
    drawRect(LeftUpX, LeftUpY, side, side, color);
}
#endif