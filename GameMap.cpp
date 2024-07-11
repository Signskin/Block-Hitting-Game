double Tick = 1.0/100.0; // Unit: s
int Interval = Tick*1000; // Unit: ms
#include<unordered_set>
#include<set>
#include<list>
#include "Objects.cpp"
#ifndef LETTERS_H
    #include "Letters.h"
#endif
#include<stdexcept>
#include<random>
#include<algorithm>
using namespace std;

int getRandom_i(int min, int max)
{
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}
double getRandom_d(double min, double max)
{
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<double> dis(min, max);
    return dis(gen);
}

class Timing
{
    private:
        int StartTime;
        int PauseStartTime;
        int TotalPauseTime;
        bool isend;
        int EndTime;
    public:
        Timing(): StartTime(0), PauseStartTime(0), TotalPauseTime(0), EndTime(0), isend(false)  {}
        void reset()
        {
            StartTime = 0;
            PauseStartTime = 0;
            TotalPauseTime = 0;
            isend = 0;
        }
        void start()
        {
            StartTime = SDL_GetTicks();
        }
        void pause()
        {
            PauseStartTime = SDL_GetTicks();
        }
        void resume()
        {
            if(isend) return;
            TotalPauseTime += SDL_GetTicks() - PauseStartTime;
        }
        void terminate()
        {
            if(!isend) EndTime = SDL_GetTicks();
            isend = true;
        }
        double getTimeElapsed(bool isnowpause = false)
        {
            if(isend) return static_cast<int>((EndTime - StartTime - TotalPauseTime)/10) / 100.0;
            if(isnowpause) return static_cast<int>((PauseStartTime - StartTime - TotalPauseTime - (SDL_GetTicks() - PauseStartTime))/10) / 100.0;
            return static_cast<int>((SDL_GetTicks() - StartTime - TotalPauseTime)/10) / 100.0;
        }
};

class Game
{
    friend class GameInit;
    private:
        Screen screen;
        list<Ball> balls; // Seperate Balls from Objects
        vector<ObjectBase*> Objects; // Objects otherthan Balls
        bool isRunning = false;
        bool isPaused = false;
        bool isWin = false;
        bool isLose = false;
        bool isinit = false;
        Timing timer;
        void handleCollision()
        {
            unordered_set<ObjectBase*> toDelete;
            for(auto& ball: balls)
            {
                for(auto* obj: Objects)
                {
                    bool flag = ball.handleCollision(obj);
                    if(obj->getType() == ObjectBase::ObjectType::Block and flag) toDelete.insert(obj);
                }
            }
            for(auto Iter = Objects.begin(); Iter != Objects.end();)
            {
                if(toDelete.find(*Iter) != toDelete.end())
                {
                    delete *Iter;
                    Iter = Objects.erase(Iter);
                }
                else ++Iter;
            }
        }
        bool insafe(const Ball& ball)
        {
            return ball.getPos().x + ball.getRadius() >= 0 and ball.getPos().x - ball.getRadius() <= screen.getWidth() and ball.getPos().y + ball.getRadius() >= 0 and ball.getPos().y - ball.getRadius() <= 0.9 * screen.getHeight();
        }
        void handleOutofRange() // Mainly Balls
        {
            for(auto Iter = balls.begin(); Iter != balls.end();)
            {
                if(insafe(*Iter) == false)
                {
                    Iter = balls.erase(Iter);
                }
                else ++Iter;
            }
        }
        void render();
        void handleInput();
        void clear()
        {
            screen.clear();
            for(auto obj: Objects) delete obj;
            Objects.clear();
            balls.clear();
        }
        int TickCount;
        
        void BeforeStart()
        {
            
        }
        bool JudgeWin()
        {
            for(auto& obj: Objects) if(obj->getType() == ObjectBase::ObjectType::Block) return false;
            return true;
        }
        void Win()
        {
            //screen.drawText(300, 200,"You Win!",50,  white);
            timer.terminate();
            isWin = JudgeWin();
        }
        bool JudgeLost()
        {
            return balls.empty();
        }
        void Lost()
        {
            timer.terminate();
            isLose = JudgeLost();
        }
    public:
        void reset();
        Game(int width, int height): screen(width, height), isRunning(false), TickCount(0), timer() {}
        ~Game()
        {
            for(auto& obj: Objects) delete obj;
        }
        void addObject(ObjectBase* obj)
        {
            if(obj->getType() == ObjectBase::ObjectType::Ball) balls.push_back(dynamic_cast<Ball&>(*obj));
            Objects.push_back(obj);
        }
        void update()
        {
            //cout<<"Class Game function update called.\n";
            for(auto& obj: Objects) obj->update();
            for(auto& ball: balls) ball.update();
            handleCollision();
            handleOutofRange();
        }
        void draw()
        {
            screen.clear();
            for(auto& obj: Objects) obj->draw(screen);
            for(auto& ball: balls) ball.draw(screen);
            int remainingBlocks = 0; 
            for(auto& obj: Objects) if(obj->getType() == ObjectBase::ObjectType::Block) ++remainingBlocks;
            double timeElapsed = timer.getTimeElapsed(isPaused);
            screen.drawStatusBar(remainingBlocks, timeElapsed);
            screen.update();
        }
        void run()
        {
            int nextTick = SDL_GetTicks() + Interval;
            reset();
            isinit = true;
            isRunning = true;
            // TickCount = SDL_GetTicks(); //?
            SDL_Event event;
            while(isRunning)
            {
                if(isWin or isLose)
                {
                    SDL_Event event;
                    while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_KEYDOWN) {
                            switch (event.key.keysym.sym) {
                                case SDLK_r:  // Restart
                                    reset();
                                    break;
                                case SDLK_q:  // Quit
                                    isRunning = false;
                                    break;
                            }
                        }
                    }
                    screen.clear();
                    if(isWin)
                        screen.drawText(250, 200, "You Win!", 50, white, true),
                        screen.drawText(250, 250, "All blocks are eliminated!", 40, white, true);
                    else if(isLose)
                        screen.drawText(250, 200, "You Lose!", 50, red, true),
                        screen.drawText(250, 250, "All balls are lost!", 40, red, true);
                    std::string timeText = "Used Time: " + std::to_string(timer.getTimeElapsed()) + " seconds";
                    screen.drawText(250, 300, timeText, 40,  skyblue, true);
                    screen.drawText(250, 400, "Press R to Restart, Q to Quit", 20, aquagreen, true);
                    screen.update();
                    SDL_Delay(Interval);
                }
                else
                while(SDL_PollEvent(&event))
                {
                    if(event.type == SDL_QUIT) isRunning = false;
                    else if(event.type == SDL_KEYDOWN)
                    {
                        auto key = event.key.keysym.sym;
                        if(key == SDLK_ESCAPE)
                        {
                            if(isPaused)
                            {
                                isPaused = false;
                                timer.resume();
                            }
                            else
                            {
                                isPaused = true;
                                timer.pause();
                            }
                        } 
                        else if(key == SDLK_r) // Restart
                        {
                            // for(auto& obj: Objects) delete obj;
                            // Objects.clear();
                            // balls.clear();
                            // isPaused = false;
                            // timer.start();
                            reset();
                        }
                        else if(key == SDLK_s) // Settings
                        {
                            // Settings
                        }
                        else if(key == SDLK_LEFT) // PaddleLeft
                        {
                            for(auto& obj: Objects) if(obj->getType() == ObjectBase::ObjectType::Paddle)
                            {
                                auto* p = dynamic_cast<Paddle*>(obj);
                                p->setSpeed(p->getSpeed()-p->getAcceration());
                            } 
                        }
                        else if(key == SDLK_RIGHT) // PaddleRight
                        {
                            for(auto& obj: Objects) if(obj->getType() == ObjectBase::ObjectType::Paddle)
                            {
                                auto* p = dynamic_cast<Paddle*>(obj);
                                p->setSpeed(p->getSpeed()+p->getAcceration());
                            } 
                        }
                        else if(key == SDLK_q) // Quit
                        {
                            isRunning = false;
                        }
                    }
                }
                if(!isPaused)
                {
                    if(SDL_GetTicks() > nextTick)
                    {
                        if(JudgeWin()) Win();
                        else if(JudgeLost()) Lost();
                        else
                        {
                            update();
                            draw();
                            nextTick += Interval;
                        }
                        
                    }
                }
                else
                {
                    //timer.pause();
                    screen.drawText(300, 200, "Game Paused", 50, red, true);
                    screen.drawText(300, 250, "Press ESC to Resume", 40, red, true);
                    screen.update();
                    SDL_Delay(Interval);
                }
            }
        }
        const Screen& getScreen() const {return screen;}
};

class GameInit
{
    public:
        enum class Difficulty {Easy, Medium, Hard, Custom} Diff;
        GameInit(Difficulty diff = Difficulty::Easy): Diff(diff) {}
        Game Init(string Word = "Hello World!")
        {
            switch(Diff)
            {
                case Difficulty::Easy: return (InitEasy());
                case Difficulty::Medium: return (InitMedium());
                case Difficulty::Hard: return (InitHard());
                case Difficulty::Custom: return (InitCustom(Word));
            }
            return InitEasy();
        }
    private:
        Game InitEasy()
        {
            int holesize = 150;
            int Width = 800;
            int Height = 600;
            Game game(Width, Height);
            game.addObject(new Wall(game.getScreen(),0, 0, 800, 20, 1, red)); // Top wall
            game.addObject(new Wall(game.getScreen(), 0, 0, 20, 530, 1, red)); // Left wall
            game.addObject(new Wall(game.getScreen(),780, 0, 20, 530, 1, red)); // Right wall
            game.addObject(new Wall(game.getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
            game.addObject(new Paddle(game.getScreen(),275, 450, 150, 20, 1.5, 80, green));
            //game.addObject(new Ball(400, 300, 8, 0, -200, 0.9, 0, -100, skyblue));
            game.balls.push_back(Ball(&game.getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));
            // for(int i = 0; i < 3; ++i)
            // {
            //     for(int j = 0; j < 5; ++j)
            //     {
            //         game.addObject(new Block(game.getScreen(),150 + i * 100, 50 + j * 50, 30, 30, 1, yellow));
            //     }
            // }
            // game.addObject(new Block(game.getScreen(),300, 50, 30, 30, 1, yellow));
            
            set<pair<double, double>> points;
            int BlockCnt = getRandom_i(20, 30);
            for(int i = 0; i < BlockCnt; ++i)
            {
                here:
                int x = getRandom_i(1, 24);
                int y = getRandom_i(1, 9);
                if(points.find({x, y}) != points.end()) goto here;
                points.insert({x, y});
                game.addObject(new Block(game.getScreen(), x*30, y*30, 30, 30, 1, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}));
            }
            return game;
        }
        Game InitMedium()
        {
            int holesize = 80;
            int Width = 800;
            int Height = 600;
            Game game(Width, Height);
            game.addObject(new Wall(game.getScreen(),0, 0, 800, 20, 1, red)); // Top wall
            game.addObject(new Wall(game.getScreen(), 0, 0, 20, 310, 1, red)); // Left wall
            game.addObject(new Wall(game.getScreen(),780, 0, 20, 310, 1, red)); // Right wall
            game.addObject(new Wall(game.getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
            game.addObject(new Paddle(game.getScreen(),275, 450, 150, 20, 1.5, 140, green));
            //game.addObject(new Ball(400, 300, 8, 0, -200, 0.9, 0, -100, skyblue));
            game.balls.push_back(Ball(&game.getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));
            // for(int i = 0; i < 3; ++i)
            // {
            //     for(int j = 0; j < 5; ++j)
            //     {
            //         game.addObject(new Block(game.getScreen(),150 + i * 100, 50 + j * 50, 30, 30, 1, yellow));
            //     }
            // }
            // game.addObject(new Block(game.getScreen(),300, 50, 30, 30, 1, yellow));
            
            set<pair<double, double>> points;
            int BlockCnt = getRandom_i(40, 50);
            for(int i = 0; i < BlockCnt; ++i)
            {
                here: 
                int x = getRandom_i(1, 24);
                int y = getRandom_i(1, 9);
                if(points.find({x, y}) != points.end()) goto here;
                points.insert({x, y});
                game.addObject(new Block(game.getScreen(), x*30, y*30, 30, 30, 1, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}));
            }
            return game;
        }
        Game InitHard()
        {
            int holesize = 80;
            int Width = 800;
            int Height = 600;
            Game game(Width, Height);
            game.addObject(new Wall(game.getScreen(),0, 0, 800, 20, 1, red)); // Top wall
            game.addObject(new Wall(game.getScreen(), 0, 0, 20, 310, 1, red)); // Left wall
            game.addObject(new Wall(game.getScreen(),780, 0, 20, 310, 1, red)); // Right wall
            game.addObject(new Wall(game.getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
            //game.addObject(new Wall(game.getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
            game.addObject(new Paddle(game.getScreen(),275, 450, 150, 20, 1.5, 250, green));
            //game.addObject(new Ball(400, 300, 8, 0, -200, 0.9, 0, -100, skyblue));
            game.balls.push_back(Ball(&game.getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));
            // for(int i = 0; i < 3; ++i)
            // {
            //     for(int j = 0; j < 5; ++j)
            //     {
            //         game.addObject(new Block(game.getScreen(),150 + i * 100, 50 + j * 50, 30, 30, 1, yellow));
            //     }
            // }
            // game.addObject(new Block(game.getScreen(),300, 50, 30, 30, 1, yellow));
            
            set<pair<double, double>> points;
            int BlockCnt = getRandom_i(40, 50);
            for(int i = 0; i < BlockCnt; ++i)
            {
                here: 
                int x = getRandom_i(1, 24);
                int y = getRandom_i(1, 9);
                if(points.find({x, y}) != points.end()) goto here;
                points.insert({x, y});
                game.addObject(new Block(game.getScreen(), x*30, y*30, 30, 30, 1, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}));
            }
            return game;
        }
        Game InitCustom(std::string Word = "Hello World!") 
        {
            Word.erase(std::remove_if(Word.begin(), Word.end(), [](char c) {
                return !std::isalnum(c) && c != ' ' && c!= '!';
            }), Word.end());

            if (Word.size() > 30) {
                Word = Word.substr(0, 12);
            }

            int Width = 800;
            int Height = 600;
            Game game(Width, Height);

            int BlockSize = std::max(16, std::min(32, static_cast<int>(800 / Word.size()))) * 0.23;

            int holesize = 80;
            game.addObject(new Wall(game.getScreen(),0, 0, 800, 20, 1, red)); // Top wall
            game.addObject(new Wall(game.getScreen(), 0, 0, 20, 530, 1, red)); // Left wall
            game.addObject(new Wall(game.getScreen(),780, 0, 20, 530, 1, red)); // Right wall
            game.addObject(new Wall(game.getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
            game.addObject(new Wall(game.getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
            game.addObject(new Paddle(game.getScreen(),275, 450, 150, 20, 1.5, 80, green));
            game.balls.push_back(Ball(&game.getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));

            int startX = 30;
            int startY = 200;
            for (char c : Word) {
                const int (*pixelData)[8] = getCharacterData(c);

                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        if (pixelData[i][j]) {
                            game.addObject(new Block(game.getScreen(), startX + j * BlockSize, startY + i * BlockSize, BlockSize, BlockSize, 1, yellow));
                        }
                    }
                }
                startX += BlockSize * 9;
            }

            return game;
        }
};

int main(int, char**)
{
    try
    {
        // GameInit gameInit(GameInit::Difficulty::Easy);
        // Game game = gameInit.Init();
        Game game(800, 600);
        game.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(pair<runtime_error, const char*>& e)
    {
        cerr << e.first.what() << " " << e.second << endl;
    }
    catch(const char* e)
    {
        cerr << e << endl;
    }
        
    return 0;
}

void Game::reset()
{
    int easyPosY = 200;
    int mediumPosY = 250;
    int hardPosY = 300;
    int customPosY = 350;
    int selectedOption = 0;
    
    bool selectingDifficulty = true;
    SDL_Event event;

    while(selectingDifficulty)
    {
        screen.clear();
        here:
        SDL_Color Green = green;
        SDL_Color White = white;
        
        screen.drawText(250, 100, "Welcome to Block Cracking Game!", 48, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}, true);
        screen.drawText(250, 150, "Please select a difficulty level:", 40, white, true);
        screen.drawText(250, easyPosY, "Easy", 40, selectedOption == 0 ? Green : White, true);
        screen.drawText(250, mediumPosY, "Medium", 40, selectedOption == 1 ? Green : White, true);
        screen.drawText(250, hardPosY, "Hard", 40, selectedOption == 2 ? Green : White, true);
        screen.drawText(250, customPosY, "Custom", 40, selectedOption == 3 ? Green : White, true);
        screen.update();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                selectingDifficulty = false;
                isRunning = false;
                if(!isinit) exit(0);
                return;
            }
            else if (event.type == SDL_KEYDOWN) 
            {
                switch (event.key.keysym.sym) 
                {
                    case SDLK_UP:
                        selectedOption = (selectedOption - 1 + 4) % 4;
                        break;
                    case SDLK_DOWN:
                        selectedOption = (selectedOption + 1) % 4;
                        break;
                    case SDLK_RETURN:
                        selectingDifficulty = false;
                        break;
                    case SDLK_ESCAPE:
                        selectingDifficulty = false;
                        if(!isinit) exit(0);
                        isPaused = true;
                        timer.pause();
                        return;
                }
            }
        }

        SDL_Delay(Interval);
    }

    string customText = "Hello World!";
    if (selectedOption == 3) 
    {
        char inputText[13] = "Hello World!";
        SDL_StartTextInput();
        bool enteringText = true;
        
        while (enteringText) {
            screen.clear();
            screen.drawText(250, 200, "Enter custom text:", 40, white, true);
            if (strlen(inputText) > 0)
            screen.drawText(250, 250, inputText, 40, white, true);
            screen.drawText(250, 300, "(Max 12 characters, including A-Z, a-z, 1-9, !, space)", 20, white, true);
            screen.update();

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT) 
                {
                    enteringText = false;
                    //isRunning = false;
                    if(!isinit) exit(0);
                    return;
                }
                else if (event.type == SDL_KEYDOWN) 
                {
                    if (event.key.keysym.sym == SDLK_RETURN) 
                    {
                        enteringText = false;
                    } 
                    else if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(inputText) > 0) 
                    {
                        inputText[strlen(inputText) - 1] = '\0';
                    } 
                    else if (event.key.keysym.sym == SDLK_ESCAPE) 
                    {
                        enteringText = false;
                        selectingDifficulty = true;
                        goto here;
                    }
                }
                else if (event.type == SDL_TEXTINPUT) 
                {
                    if (strlen(inputText) < 12) 
                    {
                        strcat(inputText, event.text.text);
                    }
                }
            }

            SDL_Delay(Interval);
        }

        SDL_StopTextInput();
        customText = string(inputText);
    }
    // Game game(800, 600);
    for(auto obj: Objects) delete obj;
    Objects.clear();
    balls.clear();

    if(selectedOption == 0)
    {
        int holesize = 150;
        int Width = 800;
        int Height = 600;
        addObject(new Wall(getScreen(),0, 0, 800, 20, 1, red)); // Top wall
        addObject(new Wall(getScreen(), 0, 0, 20, 530, 1, red)); // Left wall
        addObject(new Wall(getScreen(),780, 0, 20, 530, 1, red)); // Right wall
        addObject(new Wall(getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
        addObject(new Wall(getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
        addObject(new Wall(getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
        addObject(new Paddle(getScreen(),275, 450, 150, 20, 1.5, 80, green));
        //game.addObject(new Ball(400, 300, 8, 0, -200, 0.9, 0, -100, skyblue));
        balls.push_back(Ball(&getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));
        // for(int i = 0; i < 3; ++i)
        // {
        //     for(int j = 0; j < 5; ++j)
        //     {
        //         game.addObject(new Block(game.getScreen(),150 + i * 100, 50 + j * 50, 30, 30, 1, yellow));
        //     }
        // }
        // game.addObject(new Block(game.getScreen(),300, 50, 30, 30, 1, yellow));
        
        set<pair<double, double>> points;
        int BlockCnt = getRandom_i(20, 30);
        for(int i = 0; i < BlockCnt; ++i)
        {
            here1:
            int x = getRandom_i(1, 24);
            int y = getRandom_i(1, 9);
            if(points.find({x, y}) != points.end()) goto here1;
            points.insert({x, y});
            addObject(new Block(getScreen(), x*30, y*30, 30, 30, 1, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}));
        }
    }
    else if(selectedOption == 1)
    {
        int holesize = 80;
        int Width = 800;
        int Height = 600;
        addObject(new Wall(getScreen(),0, 0, 800, 20, 1, red)); // Top wall
        addObject(new Wall(getScreen(), 0, 0, 20, 310, 1, red)); // Left wall
        addObject(new Wall(getScreen(),780, 0, 20, 310, 1, red)); // Right wall
        addObject(new Wall(getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
        addObject(new Wall(getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
        addObject(new Wall(getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
        addObject(new Paddle(getScreen(),275, 450, 150, 20, 1.5, 140, green));
        //game.addObject(new Ball(400, 300, 8, 0, -200, 0.9, 0, -100, skyblue));
        balls.push_back(Ball(&getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));
        // for(int i = 0; i < 3; ++i)
        // {
        //     for(int j = 0; j < 5; ++j)
        //     {
        //         game.addObject(new Block(game.getScreen(),150 + i * 100, 50 + j * 50, 30, 30, 1, yellow));
        //     }
        // }
        // game.addObject(new Block(game.getScreen(),300, 50, 30, 30, 1, yellow));
        
        set<pair<double, double>> points;
        int BlockCnt = getRandom_i(40, 50);
        for(int i = 0; i < BlockCnt; ++i)
        {
            here2: 
            int x = getRandom_i(1, 24);
            int y = getRandom_i(1, 9);
            if(points.find({x, y}) != points.end()) goto here2;
            points.insert({x, y});
            addObject(new Block(getScreen(), x*30, y*30, 30, 30, 1, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}));
        }
    }
    else if(selectedOption == 2)
    {
        int holesize = 80;
        int Width = 800;
        int Height = 600;
        addObject(new Wall(getScreen(),0, 0, 800, 20, 1, red)); // Top wall
        addObject(new Wall(getScreen(), 0, 0, 20, 310, 1, red)); // Left wall
        addObject(new Wall(getScreen(),780, 0, 20, 310, 1, red)); // Right wall
        addObject(new Wall(getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
        addObject(new Wall(getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
        //game.addObject(new Wall(game.getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
        addObject(new Paddle(getScreen(),275, 450, 150, 20, 1.5, 250, green));
        //game.addObject(new Ball(400, 300, 8, 0, -200, 0.9, 0, -100, skyblue));
        balls.push_back(Ball(&getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));
        // for(int i = 0; i < 3; ++i)
        // {
        //     for(int j = 0; j < 5; ++j)
        //     {
        //         game.addObject(new Block(game.getScreen(),150 + i * 100, 50 + j * 50, 30, 30, 1, yellow));
        //     }
        // }
        // game.addObject(new Block(game.getScreen(),300, 50, 30, 30, 1, yellow));
        
        set<pair<double, double>> points;
        int BlockCnt = getRandom_i(40, 50);
        for(int i = 0; i < BlockCnt; ++i)
        {
            here3: 
            int x = getRandom_i(1, 24);
            int y = getRandom_i(1, 9);
            if(points.find({x, y}) != points.end()) goto here3;
            points.insert({x, y});
            addObject(new Block(getScreen(), x*30, y*30, 30, 30, 1, (SDL_Color){getRandom_i(0, 255), getRandom_i(0, 255), getRandom_i(0, 255), 255}));
        }
    }
    else if(selectedOption == 3)
    {
        string Word = customText;
        Word.erase(std::remove_if(Word.begin(), Word.end(), [](char c) {
                return !std::isalnum(c) && c != ' ' && c!= '!';
            }), Word.end());

            if (Word.size() > 30) {
                Word = Word.substr(0, 12);
            }

            int Width = 800;
            int Height = 600;

            int BlockSize = std::max(16, std::min(32, static_cast<int>(800 / Word.size()))) * 0.23;

            int holesize = 80;
            addObject(new Wall(getScreen(),0, 0, 800, 20, 1, red)); // Top wall
            addObject(new Wall(getScreen(), 0, 0, 20, 530, 1, red)); // Left wall
            addObject(new Wall(getScreen(),780, 0, 20, 530, 1, red)); // Right wall
            addObject(new Wall(getScreen(),0 , 300, Width/2-holesize/2, 20, 1, aquagreen));
            addObject(new Wall(getScreen(), Width/2+holesize/2, 300, Width/2-holesize/2, 20, 1, aquagreen));
            addObject(new Wall(getScreen(),0 , 520, 800, 10, 1, aquagreen)); // Bottom Line
            addObject(new Paddle(getScreen(),275, 450, 150, 20, 1.5, 80, green));
            balls.push_back(Ball(&getScreen(), 400, 300, 8, 0, 400, 1, 5, 0, skyblue));

            int startX = 30;
            int startY = 200;
            for (char c : Word) {
                const int (*pixelData)[8] = getCharacterData(c);

                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        if (pixelData[i][j]) {
                            addObject(new Block(getScreen(), startX + j * BlockSize, startY + i * BlockSize, BlockSize, BlockSize, 1, yellow));
                        }
                    }
                }
                startX += BlockSize * 9;
            }
    }
    // this->clear();
    

    isRunning = true;
    isPaused = false;
    timer.reset();
    timer.start();
    isWin = false;
    isLose = false;
}
