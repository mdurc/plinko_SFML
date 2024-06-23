#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
using namespace std;

#define SIZE 700
#define GRAVITY 0.1f
#define BOUNCE_DAMPING 0.7f
#define FRAME_RATE 600

typedef struct{
    sf::CircleShape ball;
    sf::Vector2f vel;
    float amount;
} Ball;

typedef struct{
    sf::RectangleShape block;
    float score;
} Block;



float balance = 100.0;


void constrain(float& a, float min, float max, int sign){
    float b = std::abs(a);
    if (b < min)
        a = min * sign;
    else if (b > max)
        a = max * sign;
}

bool checkCollision(const sf::CircleShape& pin, Ball& b) {
    sf::CircleShape& ball = b.ball;
    sf::Vector2f ballPos = ball.getPosition();
    float ballRadius = ball.getRadius();

    sf::Vector2f pinPos = pin.getPosition();
    float pinRadius = pin.getRadius();

    // distance formula to find dist between the radii
    float distance = sqrt(pow(ballPos.x - pinPos.x, 2) + pow(ballPos.y - pinPos.y, 2));
    if (distance <= ballRadius + pinRadius) {
        // collision

        // move ball out of collision
        float overlap = ballRadius + pinRadius - distance;
        float collisionAngle = atan2(ballPos.y - pinPos.y, ballPos.x - pinPos.x);
        float moveX = overlap * cos(collisionAngle);
        float moveY = overlap * sin(collisionAngle);
        ball.move(moveX, moveY);

        // find new velocity based on collision angle
        float speed = sqrt(pow(b.vel.x, 2) + pow(b.vel.y, 2));
        float newVelocityX = speed * cos(collisionAngle);
        float newVelocityY = speed * sin(collisionAngle);

        // update ball velocity
        b.vel.x = newVelocityX*BOUNCE_DAMPING;
        b.vel.y = newVelocityY*BOUNCE_DAMPING;

        constrain(b.vel.x, 0.8f, 1.0f, (b.vel.x>0)?1:-1);
        constrain(b.vel.y, 0.8f, 1.0f, (b.vel.y>0)?1:-1);

        return 1;
    }
    return 0;
}

bool checkCollision(Block& block, Ball& b) {
    sf::Vector2f ballPos = b.ball.getPosition();
    float ballRadius = b.ball.getRadius();
    sf::Vector2f blockPos = block.block.getPosition();
    sf::Vector2f blockSize = block.block.getSize();

    if (ballPos.x + ballRadius >= blockPos.x &&
            ballPos.x - ballRadius <= blockPos.x + blockSize.x &&
            ballPos.y + ballRadius >= blockPos.y &&
            ballPos.y - ballRadius <= blockPos.y + blockSize.y) {

        balance += b.amount*block.score;

        //respawn the ball at the top
        b.ball.setFillColor(sf::Color(0, 0, 0, 255));
        //float randomX = SIZE / 2 + 25 - rand() % 51;
        //b.ball.setPosition(randomX, 10);
        //b.vel.y = 0;
        //b.vel.x = 0;
        return 1;
    }
    return 0;
}

float generateScore(int index, int num_blocks) {
    float max_score;
    if(num_blocks <= 8.0f){
        max_score = 9.0f;
    }else if(num_blocks < 18.0f){
        max_score = 128.0f;
    }else{
        max_score = 1000.0f;
    }
    float min_score = 0.1f;

    // find the closest side and then square root the distance times
    int dist = std::min(index, num_blocks-index);
    for(;dist;--dist){
        if(max_score<10){
            max_score/=10;
        }else{
            max_score=sqrt(max_score);
        }
    }

    return std::max(max_score,min_score);
}

void spawnBall(vector<Ball>& balls, string amt){
    float amount;
    try{
        amount = std::stof(amt);
    }catch(const exception& e){
        cerr << "Invalid Price" << endl;
        return;
    }
    if(amount > balance){
        cout << "Not enough funds" << endl;
        return;
    }
    int radius = 5;
    Ball ball = {sf::CircleShape(radius,9), sf::Vector2f(0,0), amount};
    float randomX = SIZE/2 + 25 -rand()%51;
    ball.ball.setPosition(randomX, 10-radius*2);
    balls.push_back(ball);
    balance-=amount;
}

int main(){
    srand(static_cast<unsigned int>(time(0)));

    sf::Font font;
    if(!font.loadFromFile("mono.ttf")){
        cerr << "ERROR: Cannot find file \"mono.ttf\"" << endl;
        exit(1);
    }

    sf::RenderWindow window;
    window.create(sf::VideoMode(SIZE, SIZE), "gambler");

    window.setPosition(sf::Vector2i(700, 100));


    // ADD BUTTON:
    sf::Text moneyText("Add 100", font, 15);
    moneyText.setPosition(50, 110);
    sf::RectangleShape addMoneyButton(sf::Vector2f(30, 20));
    addMoneyButton.setFillColor(sf::Color(50, 50, 90));
    addMoneyButton.setPosition(125, 110);



    sf::Text priceText("Enter Price:", font, 16);
    priceText.setPosition(50, 50);

    sf::Text inputText("", font, 16);
    inputText.setPosition(150, 50);
    inputText.setFillColor(sf::Color::Red);


    int num_rows = 20;
    vector<sf::CircleShape> pins;
    for(int row=2;row<num_rows;++row){
        for(int col=0;col<=row;++col){
            sf::CircleShape pin(3, 5);
            float x = SIZE / 2 + (col - row/2.0f) * 32;
            float y = 25 + row * 32;
            pin.setPosition(x, y);
            pins.push_back(pin);
        }
    }

    // color scheme for blocks
    vector<sf::Color> colorScheme;
    colorScheme.push_back(sf::Color(255, 100, 100));
    colorScheme.push_back(sf::Color(255, 180, 180));
    colorScheme.push_back(sf::Color(255, 220, 220));
    colorScheme.push_back(sf::Color(255, 200, 100));
    colorScheme.push_back(sf::Color(255, 255, 100));
    colorScheme.push_back(sf::Color(255, 200, 100));
    colorScheme.push_back(sf::Color(255, 220, 220));
    colorScheme.push_back(sf::Color(255, 180, 180));
    

    vector<Block> blocks;
    int num_blocks = num_rows-1;
    float block_width = 28;
    float block_height = block_width - 6;
    for (int i = 0; i < num_blocks; ++i) {
        sf::RectangleShape block(sf::Vector2f(block_width, block_height));
        block.setFillColor(colorScheme[i % colorScheme.size()]);
        block.setPosition(SIZE/2 + (i-num_rows/2.0f)*32+22, 15+num_rows*32);
        Block b = {block, generateScore(i,num_blocks-1)};
        blocks.push_back(b);
    }

    vector<Ball> balls;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }else if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                if (addMoneyButton.getGlobalBounds().contains(mousePosF)) {
                    balance+=100.0f;
                }
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode >= 48 && event.text.unicode <= 57) {
                    // typed a number for the price amount
                    inputText.setString(inputText.getString() + static_cast<char>(event.text.unicode));
                } else if (event.text.unicode == 8) {
                    // Backspace handling
                    std::string currentString = inputText.getString();
                    if (!currentString.empty()) {
                        currentString.pop_back();
                        inputText.setString(currentString);
                    }
                }else if (event.text.unicode == 10){
                    // Attempting to add a ball at the current inputted price
                    string amount = inputText.getString();
                    spawnBall(balls, amount);
                }
            }
        }


        sf::sleep(sf::seconds(1.0f / FRAME_RATE));
        window.clear(sf::Color(0, 0, 30));

        int pz=pins.size();
        int bz=balls.size();
        for(int i=0;i<bz;++i){
            balls[i].vel.y += GRAVITY;
            balls[i].ball.move(balls[i].vel.x, balls[i].vel.y);
            window.draw(balls[i].ball);
            bool flag = false;
            for(int k=0;k<num_blocks;++k){
                if(checkCollision(blocks[k],balls[i])){
                    flag = true;
                    balls.erase(balls.begin() + i);
                    break;
                }
            }
            if(flag){ continue; }
            for(int j=0;j<pz;++j){
                if(checkCollision(pins[j], balls[i])){
                    break;
                }
            }
        }
        for(int i=0;i<pz;++i){
            window.draw(pins[i]);
        }

        for(int i=0;i<num_blocks;++i) {

            string score = to_string(blocks[i].score);
            sf::Text score_text(score.substr(0,score.find('.')+2), font, 12);
            score_text.setFillColor(sf::Color::Black);

            sf::Vector2f blockPos = blocks[i].block.getPosition();
            score_text.setPosition(blockPos.x + 2, blockPos.y + 2);

            window.draw(blocks[i].block);
            window.draw(score_text);
        }

        string bal = to_string(balance);
        sf::Text balance_text("Current Balance: " + bal.substr(0,bal.find('.')+3), font, 25);
        balance_text.setFillColor(sf::Color::White);
        window.draw(balance_text);


        window.draw(addMoneyButton);
        window.draw(moneyText);
        window.draw(priceText);
        window.draw(inputText);
        window.display();

    }

    return 0;
}
