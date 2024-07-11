#include "Screen.cpp"
#include <vector>
#ifndef color_h
    #include "color.h"
#endif

// const double Tick = 1/20;
extern double Tick;
using namespace std;

struct Point
{
    double x;
    double y;
};

struct Vector
{
    double x;
    double y;

    friend Vector operator-(const Vector& Self)
    {
        return { -Self.x, -Self.y };
    }

    template <class T>
    friend Vector operator*(const Vector& Self, T Cof)
    {
        return { Self.x * Cof, Self.y * Cof };
    }
};

class ObjectBase
{
    public:
        virtual void draw(Screen& S) = 0;
        virtual void update() = 0;
        virtual bool handleCollision(ObjectBase* other) = 0;
        virtual ~ObjectBase() = default;
        enum class ObjectType: unsigned int
        {
            Ball = 1,
            Block = 2,
            Wall = 3,
            Paddle = 4,
        };
        virtual ObjectType getType() = 0;
        virtual const Point& getPos() const = 0;
        virtual Vector& getVelocity() = 0;
        double CollisionCof;
        ObjectType Type;
};
class Ball : public ObjectBase
{
    private:
        Point Center;
        Vector Velocity;
        double Radius;
        SDL_Color Color;
        Vector acceration;

    public:
        Ball(const Screen* scr,double x, double y, double r, double vx, double vy, double CC = 0.9, double Ax = 0, double Ay = 0, SDL_Color C = green);
        Ball(const Ball& other) : inscreen(other.inscreen)
        {
            Center = other.Center;
            Velocity = other.Velocity;
            Radius = other.Radius;
            Color = other.Color;
            CollisionCof = other.CollisionCof;
            Type = other.Type;
            acceration = other.acceration;
        }
        void draw(Screen& S) override;
        void update();
        void update(int signal);
        void update(Point NewP, Vector NewV);
        bool handleCollision(ObjectBase* other) override;
        int getRadius() const;
        ObjectType getType() override;
        const Point& getPos() const override;
        Vector& getVelocity() override;
        const Screen* inscreen;
};

class Block : public ObjectBase
{
    private:
        Point LeftUp;
        int Width;
        int Height;
        SDL_Color color;

    public:
        Block(const Screen& scr,double x, double y, int w, int h, double CC, SDL_Color C = blue);

        void draw(Screen& screen) override;
        void update() override;
        bool handleCollision(ObjectBase* other) override;
        ObjectType getType() override;
        const Point& getPos() const override;
        int getWidth();
        int getHeight();
        Vector& getVelocity() override; // Empty implementation
        const Screen& inscreen;
};

class Paddle : public ObjectBase
{
    private:
        Point LeftUp;
        int Width;
        int Height;
        double Speed; // Only x direction
        double acceration;
        SDL_Color color;

    public:
        Paddle(const Screen& scr,double x, double y, double w, double h, double CC = 0.5, double A = 0, SDL_Color C = skyblue);

        void draw(Screen& screen) override;
        void update() override;
        bool handleCollision(ObjectBase* other) override;
        ObjectType getType() override;
        const Point& getPos() const override;
        int getWidth();
        int getHeight();
        double getSpeed();
        void setSpeed(double v);
        Vector& getVelocity() override; // Empty implementation
        const Screen& inscreen;
        double getAcceration() const {return acceration;}
};

class Wall : public ObjectBase
{
    private:
        Point LeftUp;
        int Width;
        int Height;
        SDL_Color color;

    public:
        Wall(const Screen& scr,double x, double y, double w, double h, double CC, SDL_Color C = red);

        void draw(Screen& screen) override;
        void update() override;
        bool handleCollision(ObjectBase* other) override;
        ObjectType getType() override;
        const Point& getPos() const override;
        int getWidth();
        int getHeight();
        Vector& getVelocity() override; // Empty implementation
        const Screen& inscreen;
};

// Implementations

Ball::Ball(const Screen* scr,double x, double y, double r, double vx, double vy, double CC, double Ax, double Ay, SDL_Color C)
    : Center({x, y}), Radius(r), Velocity({vx, vy}), acceration({Ax, Ay}), Color(C), inscreen(scr)
{
    CollisionCof = CC;
    Type = ObjectType::Ball;
}

void Ball::draw(Screen& S)
{
    S.drawFullCircle(Center.x, Center.y, Radius, Color);
}

void Ball::update(int signal)
{
    Center.x += signal * Tick * Velocity.x;
    Center.y += signal * Tick * Velocity.y;
    Velocity.y += acceration.y * Tick;
    Velocity.x += acceration.x * Tick;
}

void Ball::update()
{
    
    Center.x += Tick * Velocity.x;
    Center.y += Tick * Velocity.y;
    Velocity.y += acceration.y * Tick;
    Velocity.x += acceration.x * Tick;
}

void Ball::update(Point NewP, Vector NewV)
{
    if(NewP.x != -1 && NewP.y != -1)
        Center = NewP;
    Velocity = NewV;
}

bool Ball::handleCollision(ObjectBase* other)
{
    if(other->getType() == ObjectType::Ball)
    {
        // Do nothing. Let's just pretend that balls don't collide with each other.
    }
    else if(other->getType() == ObjectType::Block)
    {
        Block *block = dynamic_cast<Block*>(other);
        Point LU = block->getPos();
        int W = block->getWidth();
        int H = block->getHeight();
        if (Center.x + Radius > LU.x && Center.x - Radius < LU.x + W 
            && Center.y + Radius > LU.y && Center.y - Radius < LU.y + H) // Collision with Block occur
        {
            if(Center.x < LU.x || Center.x > LU.x + W)
                Velocity.x = -Velocity.x * ((CollisionCof + block->CollisionCof) / 2);
            else
                Velocity.y = -Velocity.y * ((CollisionCof + block->CollisionCof) / 2);
            return true;
        }
    }
    else if(other->getType() == ObjectType::Wall) // Collision with Wall occur
    {
        Wall *wall = dynamic_cast<Wall*>(other);
        Point LU = wall->getPos();
        int W = wall->getWidth();
        int H = wall->getHeight();
        if (Center.x + Radius > LU.x && Center.x - Radius < LU.x + W 
            && Center.y + Radius > LU.y && Center.y - Radius < LU.y + H)
        {
            if(Center.x < LU.x || Center.x > LU.x + W)
                Velocity.x = -Velocity.x * ((CollisionCof + wall->CollisionCof) / 2),
                Center.x = (Center.x < LU.x) ? LU.x - Radius : LU.x + W + Radius;
            else
                Velocity.y = -Velocity.y * ((CollisionCof + wall->CollisionCof) / 2),
                Center.y = (Center.y < LU.y) ? LU.y - Radius : LU.y + H + Radius;
            return true;
        }
    }
    else if(other->getType() == ObjectType::Paddle) // Collision with Paddle occur
    {
        Paddle *paddle = dynamic_cast<Paddle*>(other);
        Point LU = paddle->getPos();
        int W = paddle->getWidth();
        int H = paddle->getHeight();
        double Speed = paddle->getSpeed();
        if (Center.x + Radius > LU.x && Center.x - Radius < LU.x + W 
            && Center.y + Radius > LU.y && Center.y - Radius < LU.y + H)
        {
            Velocity.y = -Velocity.y;
            Velocity.x += Speed * paddle->CollisionCof;
            return true;
        }
    }
    return false;
}

int Ball::getRadius() const
{
    return Radius;
}

ObjectBase::ObjectType Ball::getType()
{
    return Type;
}

const Point& Ball::getPos() const
{
    return Center;
}

Vector& Ball::getVelocity()
{
    return Velocity;
}

// Block Class Implementation

Block::Block(const Screen& scr,double x, double y, int w, int h, double CC, SDL_Color C)
    : LeftUp({x, y}), Width(w), Height(h), color(C), inscreen(scr)
{
    Type = ObjectType::Block;
    CollisionCof = CC;
}

void Block::draw(Screen& screen)
{
    screen.drawBlock(LeftUp.x, LeftUp.y, Width, Height, color);
}

void Block::update()
{
    // Empty implementation
}

bool Block::handleCollision(ObjectBase* other)
{
    if(other->getType() == ObjectType::Ball)
    {
        Ball *ball = dynamic_cast<Ball*>(other);
        Point LU = getPos();
        int W = getWidth();
        int H = getHeight();
        if (ball->getPos().x + ball->getRadius() > LU.x && ball->getPos().x - ball->getRadius() < LU.x + W 
            && ball->getPos().y + ball->getRadius() > LU.y && ball->getPos().y - ball->getRadius() < LU.y + H) // Collision with Block occur
        {
            return true;
        }
    }
    return false;
}

ObjectBase::ObjectType Block::getType()
{
    return Type;
}

const Point& Block::getPos() const
{
    return LeftUp;
}

int Block::getWidth()
{
    return Width;
}

int Block::getHeight()
{
    return Height;
}

Vector& Block::getVelocity()
{
    // Empty implementation
}

// Paddle Class Implementation

Paddle::Paddle(const Screen& scr,double x, double y, double w, double h, double CC, double A, SDL_Color C)
    : LeftUp({x, y}), Width(w), Height(h), Speed(0), acceration(A), color(C), inscreen(scr)
{
    Type = ObjectType::Paddle;
    CollisionCof = CC;
}

void Paddle::draw(Screen& screen)
{
    screen.drawPaddle(LeftUp.x, LeftUp.y, Width, Height, color);
}

void Paddle::update()
{
    LeftUp.x += Speed * Tick;
    //Speed += acceration * Tick;
    if(LeftUp.x < 0) LeftUp.x = 0;
    if(LeftUp.x + Width > inscreen.getWidth()) LeftUp.x = inscreen.getWidth() - Width;
    Speed *= 0.5;
}

bool Paddle::handleCollision(ObjectBase* other)
{
    // Empty implementation
    return false;
}

ObjectBase::ObjectType Paddle::getType()
{
    return Type;
}

const Point& Paddle::getPos() const
{
    return LeftUp;
}

int Paddle::getWidth()
{
    return Width;
}

int Paddle::getHeight()
{
    return Height;
}

double Paddle::getSpeed()
{
    return Speed;
}

void Paddle::setSpeed(double v)
{
    Speed = v;
}

Vector& Paddle::getVelocity()
{
    // Empty implementation
}

// Wall Class Implementation

Wall::Wall(const Screen& scr,double x, double y, double w, double h, double CC, SDL_Color C)
    : LeftUp({x, y}), Width(w), Height(h), color(C), inscreen(scr)
{
    Type = ObjectType::Wall;
    CollisionCof = CC;
}

void Wall::draw(Screen& screen)
{
    screen.drawWall(LeftUp.x, LeftUp.y, Width, Height, color);
}

void Wall::update()
{
    // Empty implementation
}

Vector& Wall::getVelocity()
{
    // Empty implementation
}

bool Wall::handleCollision(ObjectBase* other)
{
    // Do nothing;
    return false;
}

ObjectBase::ObjectType Wall::getType()
{
    return Type;
}

const Point& Wall::getPos() const
{
    return LeftUp;
}

int Wall::getWidth()
{
    return Width;
}

int Wall::getHeight()
{
    return Height;
}
