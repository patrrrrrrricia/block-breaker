#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <SDL3/SDL.h>
#include "glm/glm.hpp"
#include "circle.h"

//dimensiuni ferestra - trebuie sa fie 800x800
constexpr int WINDOW_WIDTH{800};
constexpr int WINDOW_HEIGHT{800};

//vitezele obiectelor - VITEZE NORMALE
constexpr float BALL_SPEED{300.0f};
constexpr float PADDLE_SPEED{400.0f};

//variabile SDL
SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};
SDL_Event currentEvent;
SDL_Color backgroundColor{50, 50, 50, 255};

bool quit{false};
float displayScale{1.0f};

//pt delta time - necesar pt miscare fluida
Uint64 startTime, endTime, elapsedTime;

//starile jocului
enum GameState { PLAYING, WON, LOST };
enum BallState { ON_PADDLE, LAUNCHED };

GameState gameState = PLAYING;
BallState ballState = ON_PADDLE;

//structura pt dreptunghiuri (paleta si caramizi)
struct Rectangle {
    glm::vec2 pos;
    float width;
    float height;
    SDL_Color color;
    SDL_Color targetColor; // CULOAREA ȚINTĂ PENTRU ANIMAȚIE
    SDL_Color startColor;  // CULOAREA DE ÎNCEPUT PENTRU ANIMAȚIE
    int lives;  //vieti pt caramizi
    bool alive;
    Uint32 lastHitTime; // TIMPUL CAND A FOST LOVITĂ ULTIMA DATĂ
    bool isAnimating; // DACA ESTE IN CURS DE ANIMATIE

    void draw(SDL_Renderer* renderer) {
        if (!alive) return;

        //SDL deseneaza de la coltul stanga-sus, dar pozitia noastra e in centru
        SDL_FRect tmpRect;
        tmpRect.x = pos.x - width / 2.0f;
        tmpRect.y = pos.y - height / 2.0f;
        tmpRect.w = width;
        tmpRect.h = height;

        //deseneaza dreptunghiul umplut
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &tmpRect);

        //contur mai intunecat pt efect 3D
        SDL_SetRenderDrawColor(renderer, color.r / 2, color.g / 2, color.b / 2, color.a);
        SDL_RenderRect(renderer, &tmpRect);
    }
};

//obiectele jocului
Rectangle paddle;
Circle ball;
std::vector<Rectangle> bricks;

//functie pt detectia coliziunii cerc-dreptunghi
//algoritmul: gaseste cel mai apropiat punct de pe dreptunghi fata de centrul cercului
bool checkCircleRectCollision(Circle c, Rectangle r) {
    if (!r.alive) return false;

    //gaseste cel mai apropiat punct
    float closestX = glm::clamp(c.pos.x, r.pos.x - r.width/2.0f, r.pos.x + r.width/2.0f);
    float closestY = glm::clamp(c.pos.y, r.pos.y - r.height/2.0f, r.pos.y + r.height/2.0f);

    //calculeaza distanta
    float distX = c.pos.x - closestX;
    float distY = c.pos.y - closestY;
    float distSquared = distX * distX + distY * distY;

    //verifica daca e coliziune
    return distSquared < (c.radius * c.radius);
}

//initializeaza jocul la starea de start
void initGame() {
    //setup paleta - roz deschis, jos
    paddle.pos = glm::vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 50.0f);
    paddle.width = 100.0f;
    paddle.height = 25.0f;
    paddle.color = SDL_Color{255, 200, 220, 255}; // Roz deschis
    paddle.lives = 1;
    paddle.alive = true;

    //setup minge - albă, pe paleta
    ball.radius = 10.0f;
    ball.pos = glm::vec2(paddle.pos.x, paddle.pos.y - paddle.height/2.0f - ball.radius);
    ball.color = SDL_Color{255, 255, 255, 255}; // Alb
    ball.dir = glm::vec2(0.0f, -1.0f); //sus
    ball.speed = BALL_SPEED;

    //creaza caramizile - 3 randuri de cate 17 in degrade de roz
    bricks.clear();
    int numBricks = 17;
    float brickWidth = WINDOW_WIDTH / static_cast<float>(numBricks);
    float brickHeight = 20.0f;
    float startY = 100.0f + brickHeight / 2.0f;

    for (int row = 0; row < 3; row++) {
        for (int i = 0; i < numBricks; i++) {
            Rectangle brick;
            brick.width = brickWidth;
            brick.height = brickHeight;
            brick.pos = glm::vec2(i * brickWidth + brickWidth/2.0f, startY + row * brickHeight);
            brick.lastHitTime = 0;
            brick.isAnimating = false;

            if (row == 0) {
                // Roz închis (sus) - 1 viață
                brick.color = SDL_Color{200, 100, 150, 255};
                brick.targetColor = SDL_Color{200, 100, 150, 255};
                brick.startColor = SDL_Color{200, 100, 150, 255};
                brick.lives = 1;
            } else if (row == 1) {
                // Roz mediu (mijloc) - 2 vieți
                brick.color = SDL_Color{230, 150, 180, 255};
                brick.targetColor = SDL_Color{230, 150, 180, 255};
                brick.startColor = SDL_Color{230, 150, 180, 255};
                brick.lives = 2;
            } else {
                // Roz deschis (jos) - 3 vieți
                brick.color = SDL_Color{255, 200, 220, 255};
                brick.targetColor = SDL_Color{255, 200, 220, 255};
                brick.startColor = SDL_Color{255, 200, 220, 255};
                brick.lives = 3;
            }

            brick.alive = true;
            bricks.push_back(brick);
        }
    }

    //reset starile
    gameState = PLAYING;
    ballState = ON_PADDLE;
}

//initializeaza fereastra SDL
bool initWindow() {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    } else {
        //pt ecrane HiDPI/Retina
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "Block Breaker - CodeRun6 HARD",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        } else {
            SDL_SetRenderScale(renderer, displayScale, displayScale);
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                                   backgroundColor.a);
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

//proceseaza evenimentele de la tastatura si sistem
void processEvents() {
    while (SDL_PollEvent(&currentEvent)) {
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_SPACE:
                    //lanseaza mingea doar daca e pe paleta
                    if (gameState == PLAYING && ballState == ON_PADDLE) {
                        ballState = LAUNCHED;
                        ball.dir = glm::vec2(0.0f, -1.0f);
                    }
                    break;
                case SDLK_R:
                    //restart daca jocul s-a terminat
                    if (gameState != PLAYING) {
                        initGame();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//misca paleta cu sagetile
void processMovement() {
    if (gameState != PLAYING) return;

    const bool* keys = SDL_GetKeyboardState(nullptr);

    //delta time: viteza * timp = distanta
    float moveSpeed = PADDLE_SPEED * (elapsedTime / 1000.0f);

    if (keys[SDL_SCANCODE_LEFT]) {
        paddle.pos.x -= moveSpeed;
        //nu iesi din ecran pe stanga
        if (paddle.pos.x - paddle.width/2.0f < 0) {
            paddle.pos.x = paddle.width/2.0f;
        }
    }

    if (keys[SDL_SCANCODE_RIGHT]) {
        paddle.pos.x += moveSpeed;
        //nu iesi din ecran pe dreapta
        if (paddle.pos.x + paddle.width/2.0f > WINDOW_WIDTH) {
            paddle.pos.x = WINDOW_WIDTH - paddle.width/2.0f;
        }
    }
}

//animatie si fizica mingii
void animate() {
    if (gameState != PLAYING) return;

    float deltaTime = elapsedTime / 1000.0f;

    if (ballState == ON_PADDLE) {
        //mingea urmareste paleta
        ball.pos.x = paddle.pos.x;
        ball.pos.y = paddle.pos.y - paddle.height/2.0f - ball.radius;
    } else {
        //miscarea mingii: pozitie = pozitie + directie * viteza * timp
        ball.pos += ball.dir * ball.speed * deltaTime;

        //bounce de peretele stang
        if (ball.pos.x - ball.radius < 0) {
            ball.pos.x = ball.radius;
            ball.dir.x = -ball.dir.x;
        }

        //bounce de peretele drept
        if (ball.pos.x + ball.radius > WINDOW_WIDTH) {
            ball.pos.x = WINDOW_WIDTH - ball.radius;
            ball.dir.x = -ball.dir.x;
        }

        //bounce de peretele de sus
        if (ball.pos.y - ball.radius < 0) {
            ball.pos.y = ball.radius;
            ball.dir.y = -ball.dir.y;
        }

        //verifica daca mingea a cazut jos - LOSE
        if (ball.pos.y - ball.radius > WINDOW_HEIGHT) {
            gameState = LOST;
            paddle.color = SDL_Color{255, 0, 0, 255}; //paleta devine rosie
            return;
        }

        //coliziune cu paleta - bounce bazat pe punctul de contact
        if (checkCircleRectCollision(ball, paddle)) {
            //calculeaza unde a lovit mingea pe paleta
            //-1.0 = stanga completa, 0.0 = centru, 1.0 = dreapta completa
            float hitPos = (ball.pos.x - paddle.pos.x) / (paddle.width / 2.0f);

            //limitam la [-1, 1] ca sa nu depasim daca mingea vine din unghi ciudat
            hitPos = glm::clamp(hitPos, -1.0f, 1.0f);

            //mapam de la [-1, 1] la [-80, 80] grade
            float angle = hitPos * 80.0f;
            float angleRad = glm::radians(angle);

            //directie in sus cu unghi: sin pt X, -cos pt Y (sus = negativ)
            ball.dir = glm::vec2(sin(angleRad), -cos(angleRad));
            ball.dir = glm::normalize(ball.dir);

            //repozitioneaza mingea deasupra paletei ca sa nu ramana stuck
            ball.pos.y = paddle.pos.y - paddle.height/2.0f - ball.radius;
        }

        //coliziuni cu caramizile
        bool collisionOccurred = false;
        for (auto& brick : bricks) {
            if (brick.alive && checkCircleRectCollision(ball, brick)) {
                brick.lives--; //pierde o viata

                //SETĂM CULOAREA ȚINTĂ PENTRU ANIMAȚIE
                if (brick.lives == 2) {
                    brick.targetColor = SDL_Color{230, 150, 180, 255};  //roz mediu
                } else if (brick.lives == 1) {
                    brick.targetColor = SDL_Color{200, 100, 150, 255};  //roz închis
                } else if (brick.lives <= 0) {
                    brick.alive = false;  //distruge caramida
                }

                //Pornim animația doar dacă cărămida nu a fost distrusă
                if (brick.alive) {
                    brick.startColor = brick.color; // Salvează culoarea de start
                    brick.lastHitTime = SDL_GetTicks();
                    brick.isAnimating = true;
                }

                //bounce simplu - inverseaza Y
                ball.dir.y = -ball.dir.y;
                collisionOccurred = true;

                // Nu mai punem break aici pentru a permite mai multe coliziuni pe frame
            }
        }

        // Dacă a avut loc o coliziune, schimbăm direcția mingei o singură dată
        if (collisionOccurred) {
            // Direcția a fost deja schimbată în buclă
        }

        //ANIMAȚIE CULORI LENTĂ - schimbare graduală a culorilor după lovitură
        Uint32 currentTime = SDL_GetTicks();
        for (auto& brick : bricks) {
            if (brick.alive && brick.isAnimating) {
                Uint32 timeSinceHit = currentTime - brick.lastHitTime;

                // Dacă a trecut mai puțin de 2000ms (2 SECUNDE) de la lovitură, facem tranziția graduală
                if (timeSinceHit < 2000) {
                    float progress = timeSinceHit / 2000.0f; // 0-1

                    // Interpolare liniară între culoarea de start și culoarea țintă
                    brick.color.r = static_cast<Uint8>(
                        brick.startColor.r + (brick.targetColor.r - brick.startColor.r) * progress
                    );
                    brick.color.g = static_cast<Uint8>(
                        brick.startColor.g + (brick.targetColor.g - brick.startColor.g) * progress
                    );
                    brick.color.b = static_cast<Uint8>(
                        brick.startColor.b + (brick.targetColor.b - brick.startColor.b) * progress
                    );
                } else {
                    // Animația s-a terminat, setăm culoarea finală
                    brick.color = brick.targetColor;
                    brick.isAnimating = false;
                }
            }
        }

        //verifica conditia de WIN - toate caramizile distruse
        bool allBricksDestroyed = true;
        for (const auto& brick : bricks) {
            if (brick.alive) {
                allBricksDestroyed = false;
                break;
            }
        }

        if (allBricksDestroyed) {
            gameState = WON;
            paddle.color = SDL_Color{0, 255, 0, 255}; //paleta devine verde
        }
    }
}

//deseneaza toate obiectele
void drawFrame() {
    //clear background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //deseneaza caramizile
    for (auto& brick : bricks) {
        brick.draw(renderer);
    }

    //deseneaza paleta
    paddle.draw(renderer);

    //deseneaza mingea (dispare cand castigi)
    if (gameState != WON) {
        ball.draw(renderer);
    }

    //update fereastra
    SDL_RenderPresent(renderer);
}

//elibereaza resursele SDL
void cleanup() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

//main loop
int main() {
    //initializare
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    initGame();
    SDL_zero(currentEvent);

    //game loop principal
    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();    //input
        processMovement();  //miscare paleta
        animate();          //fizica si coliziuni
        drawFrame();        //desenare

        endTime = SDL_GetTicks();
        elapsedTime = endTime - startTime; //delta time pt urmatorul frame
    }

    cleanup();
    return 0;
}