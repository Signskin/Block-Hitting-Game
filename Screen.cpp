#ifndef SCREEN_H
#define SCREEN_H
#include "ScreenBase.cpp"
#include "color.h"
#include<iostream>
using namespace std;

class Screen:public ScreenBase
{
    public:
        Screen(int W, int H): ScreenBase(W, H) {init();}
        void drawText(int LeftUpX, int LeftUpY, string text, int FontSize, SDL_Color color, bool center = false);
        void drawWall(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color);
        void drawPaddle(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color);
        void drawBall(int CenterX, int CenterY, int radius, SDL_Color color);
        void drawBlock(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color);
        void drawBlock(int LeftUpX, int LeftUpY, int Side, SDL_Color color);
        void drawStatusBar(int, double);
        void clearStatusBar();
        bool isInSafeRegion(int x, int y)
        {
            return x>=0 and y>=0 and x<getWidth() and y<getHeight()*0.9; // Exclude status bar
        }
};

void Screen::drawText(int LeftUpX, int LeftUpY, string text, int FontSize, SDL_Color color, bool center)
{
    TTF_Font* font = TTF_OpenFont("./Fonts/arial.ttf", FontSize); // Support Only English Characters
    if (font == nullptr) throw pair(runtime_error("Font Loading Failed."), TTF_GetError());
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (surface == nullptr) throw pair(runtime_error("Surface Creation Failed."), TTF_GetError());
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(getRenderer(), surface);
    if (texture == nullptr) throw pair(runtime_error("Texture Creation Failed."), SDL_GetError());

    SDL_Rect rect;
    rect.w = surface->w;
    rect.h = surface->h;

    if (center) 
    {
        rect.x = (getWidth() - rect.w) / 2;
        //rect.y = getHeight() - rect.h / 2;
        rect.y = LeftUpY;
    } 
    else 
    {
        rect.x = LeftUpX;
        rect.y = LeftUpY;
    }

    SDL_RenderCopy(getRenderer(), texture, nullptr, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}


void Screen::drawWall(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color)
{
    drawRect(LeftUpX, LeftUpY, width, height, color);
}

void Screen::drawPaddle(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color)
{
    drawRect(LeftUpX, LeftUpY, width, height, color);
}

void Screen::drawBall(int CenterX, int CenterY, int radius, SDL_Color color)
{
    drawFullCircle(CenterX, CenterY, radius, color);
}

void Screen::drawBlock(int LeftUpX, int LeftUpY, int width, int height, SDL_Color color)
{
    drawRect(LeftUpX, LeftUpY, width, height, color);
}

void Screen::drawBlock(int LeftUpX, int LeftUpY, int Side, SDL_Color color)
{
    drawSqur(LeftUpX, LeftUpY, Side, color);
}

void Screen::clearStatusBar()
{
    SDL_SetRenderDrawColor(getRenderer(), 0, 0, 0, 255);
    SDL_Rect statusBarRect = {0, static_cast<int>(getHeight() * 0.9), getWidth(), static_cast<int>(getHeight() * 0.1)};
    SDL_RenderFillRect(getRenderer(), &statusBarRect);

}

void Screen::drawStatusBar(int remainingBlocks, double timeElapsed)
{
    clearStatusBar();

    SDL_Color textColor = {255, 255, 255, 255};
    string timeText = "Time: " + to_string(timeElapsed) + " seconds";
    drawText(0.15*getWidth(), 0.9*getHeight(), timeText, 24,textColor);

    string blocksText = "Remaining Blocks: " + to_string(remainingBlocks);
    drawText(0.6*getWidth(), 0.9*getHeight(), blocksText, 24, textColor);

    string helpText = "Pause/Resume(ESC) Restart(R) PaddleLeft(<-) PaddleRight(->) Quit(Q)";
    drawText(0.3*getWidth(), 0.94*getHeight(), helpText, 18,textColor, true);
}
#endif