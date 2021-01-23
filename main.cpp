#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>
#include <cstring>
#include <string>

class MapElement{
private:
    sf::Texture* texture;
    bool Bomb;
public:
    MapElement(std::string path){
        this->texture = new sf::Texture();
        this->texture->loadFromFile(path);
    }

    void setBomb(bool Bomb){
        this->Bomb = Bomb;
    }

    bool isBomb(){
        return this->Bomb;
    }

    MapElement(){
        this->texture = new sf::Texture();
    }

    sf::Texture* getTexture(){
        return this->texture;
    }


    void setTexture(std::string path){
        this->texture->loadFromFile(path);
    }
};

class Bomb:public MapElement{
public:
    Bomb():MapElement("textures/bombset.png"){
        setBomb(true);
    };
};

class Number:public MapElement{
private:
    int number;
public:
    Number(int number):MapElement(){
        this->number = number;
        std::string path1 = "textures/";
        std::string path3 = "set.png";
        std::string path2 = std::to_string(number);
        setTexture(path1 + path2 + path3);
        setBomb(false);
    };

};

class Flag:public MapElement{
public:
    Flag():MapElement("textures/flagset.png"){
        setBomb(false);
    }
};

class Cell{
private:
    int x,y,size_v;
    bool Clicked = false, flaged = false;
    sf::Color color;
    MapElement* mapElement = NULL;
public:
    Cell(int x, int y, int size_v){
        this->x = x;
        this->y = y;
        this->size_v = size_v;
        this->color = sf::Color(119, 196, 24);
    };

    void draw(sf::RenderWindow* window){
        sf::RectangleShape baseRectangle(sf::Vector2f(this->size_v-2, this->size_v-2));
        baseRectangle.setPosition(this->x*this->size_v,this->y*this->size_v);
        baseRectangle.setFillColor(this->color);
        window->draw(baseRectangle);
        if((this->isClicked() || this->isFlaged()) && this->mapElement != NULL){
            drawElements(window);
        }
    };

    void drawElements(sf::RenderWindow* window){
        sf::Sprite sprite;
        sprite.setPosition(this->getX(), this->getY());
        sf::Texture* elementTexture;
        if(!this->isFlaged()) elementTexture = this->mapElement->getTexture();
        else  {
            elementTexture = new sf::Texture();
            elementTexture->loadFromFile("textures/flagset.png");
        }
        sprite.setTexture(*elementTexture);
        sprite.setPosition(this->x*this->size_v +2, this->y*this->size_v+2);
        window->draw(sprite);

    }

    MapElement* getMapElement(){
        return this->mapElement;
    }

    void setMapElement(MapElement* mapElement){
        this->mapElement = mapElement;
    }

    int getX(){
        return this->x;
    }

    int getY(){
        return this->y;
    }

    bool isClicked(){
        return this->Clicked;
    }

    void setClicked(bool isClicked){
        this->Clicked = isClicked;
    }

    bool isFlaged(){
        return this->flaged;
    }

    void setFlaged(bool flaged){
        this->flaged = flaged;
    }

    void setColor(int r, int g, int b){
        this->color = sf::Color(r,g,b);

    }
};

class Map{
private:
    int width;  //szerokosc w komorkach
    int height; //wysokosc w komorkach
    int cellSize;
    std::map<std::pair<int,int>,Cell*> cellDict;   //slownik bedzie na podst wektora zwracal nam komorke
    int bombs;
public:
    Map(int width,int height,int cellSize, int bombs){
        this-> height = height;
        this-> width = width;
        this->cellSize = cellSize;
        for(int x = 0; x<width; x++){
            for(int y = 0; y<height; y++){
                this->cellDict.insert(std::make_pair(std::make_pair(x,y),new Cell(x,y,cellSize)));
            };
        };
        this->bombs = bombs;
        this->initiateBombs(bombs);
        this->initiateNumbers();
    };

    void initiateBombs(int quantity){
        while(quantity--){
            int x = rand() % width;
            int y = rand() % height;
            Cell* cell = this->cellDict.find(std::make_pair(x,y))->second;
            while(cell->getMapElement() != NULL && cell->getMapElement()->isBomb()){
                int x = rand() % width;
                int y = rand() % height;
                cell = this->cellDict.find(std::make_pair(x,y))->second;
            }
            cell->setMapElement(new Bomb());
        }
    }

    void initiateNumbers(){
        int neighbursCoords[16] = {-1,0,-1,-1,0,-1,1,-1,1,0,1,1,0,1,-1,1};
        std::map<std::pair<int, int>, Cell*>::iterator it;
        for (it = this->getCellDict()->begin(); it != this->getCellDict()->end(); it++){
            int numberOfBombs = 0;
            Cell* cell = it->second;
            for(int i = 0; i < 16; i += 2){
                int x = cell->getX() + neighbursCoords[i];
                int y = cell->getY() + neighbursCoords[i+1];
                if(isInMap(x,y)){
                    Cell* chechingCell = this->getCellDict()->find(std::make_pair(x,y))->second;
                    if(chechingCell->getMapElement() != NULL && chechingCell->getMapElement()->isBomb()){
                        numberOfBombs++;
                    }
                }
            }
            if(numberOfBombs != 0 && (cell->getMapElement() == NULL || !cell->getMapElement()->isBomb())){
                cell->setMapElement(new Number(numberOfBombs));
            }
        }
    }

    bool isInMap(int x, int y){
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    int getWidth(){
        return this -> width;
    };
    int getHeight(){
        return this -> height;
    };

    int getCellSize(){
        return this->cellSize;
    }

    int getBombs(){
        return this->bombs;
    }

    std::map<std::pair<int,int>,Cell*>* getCellDict(){
        return &cellDict;
    };

};

class EventProvider{
private:
    sf::RenderWindow* window;
    Map* map_v;
    bool loose = false;
    bool won = false;
    int restBombs;
public:
    EventProvider(sf::RenderWindow* window, Map* map_v){
        this->window = window;
        this->map_v = map_v;
        this->restBombs = map_v->getBombs();
    }

    void onLeftClick(){
        sf::Vector2i mousePosition = sf::Mouse::getPosition(*window);
        if(mousePosition.x > 0 && mousePosition.x < map_v->getWidth()*map_v->getCellSize() && mousePosition.y > 0 && mousePosition.y < map_v->getHeight()*map_v->getCellSize()){
            int cellXCoord = mousePosition.x/map_v->getCellSize();
            int cellYCoord = mousePosition.y/map_v->getCellSize();
            auto it = map_v->getCellDict()->find(std::make_pair(cellXCoord,cellYCoord));
            if(!it->second->isFlaged()){
                if(it->second->getMapElement() != NULL && it->second->getMapElement()->isBomb()){
                    this->setLoose(true);
                }
                else{
                    propagateClick(it->second);
                }
            }

        }

    }

    void onRightClick(){
        sf::Vector2i mousePosition = sf::Mouse::getPosition(*window);
        if(mousePosition.x > 0 && mousePosition.x < map_v->getWidth()*map_v->getCellSize() && mousePosition.y > 0 && mousePosition.y < map_v->getHeight()*map_v->getCellSize()){
            int cellXCoord = mousePosition.x/map_v->getCellSize();
            int cellYCoord = mousePosition.y/map_v->getCellSize();
            auto it = map_v->getCellDict()->find(std::make_pair(cellXCoord,cellYCoord));
            Cell *cell = it->second;
            if(!cell->isClicked()) {
                if(cell->getMapElement() != NULL && cell->getMapElement()->isBomb()){
                    if(!cell->isFlaged()){
                        this->restBombs--;
                    }
                    else{
                        this->restBombs++;
                    }
                }
                cell->setFlaged(!cell->isFlaged());
                if(this->restBombs == 0) {
                    this->setWin(true);
                }
            }
        }
    }

    void propagateClick(Cell* cell){
        cell->setClicked(true);
        cell->setColor(202, 235, 185);
        if(cell->getMapElement() == NULL){
            int neighbursCoords[16] = {-1,0,-1,-1,0,-1,1,-1,1,0,1,1,0,1,-1,1};
            for(int i = 0; i < 16; i+=2){
                int x = cell->getX() + neighbursCoords[i];
                int y = cell->getY() + neighbursCoords[i+1];
                if(map_v->isInMap(x,y)){
                    Cell* chechingCell = this->map_v->getCellDict()->find(std::make_pair(x,y))->second;
                    if(!chechingCell->isClicked()){
                        propagateClick(chechingCell);
                    }
                }
            }
        }

    }

    void setLoose(bool loose){
        this->loose = loose;
    }

    bool isLost(){
        return this->loose;
    }

    void setWin(bool win){
        this->won = win;
    }

    bool hasWon(){
        return this->won;
    }

};

class MapVisualizer{
private:
    sf::RenderWindow* window;
    EventProvider* eventProvider;
    Map* map_v;
public:
    MapVisualizer(sf::RenderWindow* window, Map* map_v){
        this -> window = window;
        this-> eventProvider = new EventProvider(window, map_v);
        this -> map_v = map_v;
    };

    void drawMap(){
        std::map<std::pair<int, int>, Cell*>::iterator it;
        for (it = map_v->getCellDict()->begin(); it != map_v->getCellDict()->end(); it++){
            it->second->draw(window);
        }
    };


    EventProvider *getEventProvider(){
        return this->eventProvider;
    }

    Map* getMap(){
        return this->map_v;
    }

};


class GameEngine{
private:
    MapVisualizer* mapVisualizer;
    sf::RenderWindow* app;
    bool loose = false, win = false;
public:
    GameEngine(MapVisualizer* mapVisualizer, sf::RenderWindow* app){
        this->mapVisualizer = mapVisualizer;
        this->app = app;
    }

    void frame(){
        this->mapVisualizer->drawMap();
        this->processEvents();
        if(this->loose) lost();
        else if(this->win) won();
    }

    void processEvents(){
        sf::Event event;
        while (this->app->pollEvent(event)){
            if(this->mapVisualizer->getEventProvider()->isLost()){
                this->loose = true;
            }

            else if(this->mapVisualizer->getEventProvider()->hasWon()){
                this->win = true;
            }
            else{
                if (event.type == sf::Event::MouseButtonPressed){
                    if(event.mouseButton.button == sf::Mouse::Left){
                        this->mapVisualizer->getEventProvider()->onLeftClick();
                    }
                    else if(event.mouseButton.button == sf::Mouse::Right){
                        this->mapVisualizer->getEventProvider()->onRightClick();
                    }

                }
            // Close window : exit
            }

            if (event.type == sf::Event::Closed)
                this->app->close();
        }
    }

    void lost(){
        std::map<std::pair<int, int>, Cell*>::iterator it;
        for (it = this->mapVisualizer->getMap()->getCellDict()->begin(); it != this->mapVisualizer->getMap()->getCellDict()->end(); it++){
            Cell *foundCell = it->second;
            if(foundCell->getMapElement() != NULL && foundCell->getMapElement()->isBomb()){
                foundCell->setFlaged(false);
                foundCell->setColor(255, 0, 0);
                foundCell->drawElements(this->app);
            }
        }
    }

    void won(){
        std::map<std::pair<int, int>, Cell*>::iterator it;
        for (it = this->mapVisualizer->getMap()->getCellDict()->begin(); it != this->mapVisualizer->getMap()->getCellDict()->end(); it++){
            Cell *foundCell = it->second;
            if(foundCell->getMapElement() != NULL && foundCell->getMapElement()->isBomb()){
                foundCell->setFlaged(false);
                foundCell->setColor(0, 255, 4);
                foundCell->drawElements(this->app);
            }
        }
    }

};

int main()
{
    int cellSize = 40;
    int mapWidth = 30;
    int mapHeight = 20;
    srand( (unsigned)time(NULL) );


    // Create the main window
    sf::RenderWindow app(sf::VideoMode(mapWidth*cellSize + 20,mapHeight*cellSize + 20), "Saper",sf::Style::Titlebar | sf::Style::Close);

    Map mainMap(mapWidth,mapHeight,cellSize,20);
    MapVisualizer drawer(&app, &mainMap); //adres g³ownego obiektu rysujacego, szer pixela komorki jednej
    GameEngine game(&drawer, &app);
	// Start the game loop
    while (app.isOpen())
    {


        // Clear screen
        app.clear();

        game.frame();


        // Update the window
        app.display();
    }

    return EXIT_SUCCESS;
}
