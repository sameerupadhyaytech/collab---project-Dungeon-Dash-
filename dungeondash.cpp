#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

// ── CONSTANTS ───────────────────────────────────────────────
const int W=40, H=20, TILE=25, UI=60;
const int P_HP=15, E_HP=5, N_ENE=3, N_POT=5;
const int ATK=2, COOL=3, HEAL=4, FPS=130;

// ── STRUCTS ─────────────────────────────────────────────────
struct Entity { int x,y; };          // base data shared by all

struct Enemy : Entity {              // Enemy inherits Entity (inheritance)
    int hp=E_HP, cd=0;
    bool alive=true;
};

struct Potion : Entity {             // Potion inherits Entity (inheritance)
    bool active=true;
};

// ── GLOBALS ─────────────────────────────────────────────────
vector<string>  gMap;
vector<Enemy>   enemies(N_ENE);
vector<Potion>  potions(N_POT);
int pX,pY, pHP=P_HP, score=0;

// ── UTILITY ─────────────────────────────────────────────────
bool isWall(int x,int y){
    if(x<0||x>=W||y<0||y>=H) return true;
    return gMap[y][x]=='#';
}

void rndEmpty(int&rx,int&ry){
    do{ rx=rand()%(W-2)+1;
        ry=rand()%(H-2)+1;
    }while(isWall(rx,ry));
}

// ── MAP GENERATION ──────────────────────────────────────────
void genMap(){
    gMap.clear();
    for(int y=0;y<H;y++){
        string row(W,'.');
        for(int x=0;x<W;x++)
            if(y==0||y==H-1||x==0||x==W-1) row[x]='#';
        gMap.push_back(row);
    }
    int walls=18;
    while(walls>0){
        int wx=rand()%(W-4)+2, wy=rand()%(H-4)+2;
        if(gMap[wy][wx]=='.'){
            gMap[wy][wx]='#';
            if(rand()%2&&wx+1<W-1) gMap[wy][wx+1]='#';
            else if(wy+1<H-1)      gMap[wy+1][wx]='#';
            walls--;
        }
    }
}

// ── SPAWN + RESET ────────────────────────────────────────────
void reset(){
    pHP=P_HP; score=0;
    genMap();
    rndEmpty(pX,pY);
    for(auto&e:enemies){ rndEmpty(e.x,e.y); e.hp=E_HP; e.cd=0; e.alive=true; }
    for(auto&p:potions){ rndEmpty(p.x,p.y); p.active=true; }
}

// ── GAME LOGIC ───────────────────────────────────────────────
void moveEnemy(Enemy&e){
    if(!e.alive) return;
    int dx=(e.x<pX)?1:(e.x>pX)?-1:0;
    int dy=(e.y<pY)?1:(e.y>pY)?-1:0;
    if(dx&&!isWall(e.x+dx,e.y)) e.x+=dx;
    else if(dy&&!isWall(e.x,e.y+dy)) e.y+=dy;
}

void enemyTurn(){
    for(auto&e:enemies){
        if(!e.alive) continue;
        if(e.cd>0){e.cd--;continue;}
        if(abs(e.x-pX)+abs(e.y-pY)<=1){ pHP--; e.cd=COOL; }
    }
}

void checkPotions(){
    for(auto&p:potions)
        if(p.active&&p.x==pX&&p.y==pY){
            pHP=min(pHP+HEAL,P_HP);
            p.active=false;
        }
}

int aliveCount(){
    int n=0;
    for(auto&e:enemies) if(e.alive) n++;
    return n;
}

// ── DRAW HELPERS ─────────────────────────────────────────────
void drawHP(sf::RenderWindow&win,float x,float y,float w,float h,
            int cur,int mx,sf::Color col){
    sf::RectangleShape bg({w,h}),bar({w*(float)cur/mx,h});
    bg.setPosition({x,y});  bg.setFillColor({60,60,60});  win.draw(bg);
    bar.setPosition({x,y}); bar.setFillColor(col);         win.draw(bar);
}

void drawText(sf::RenderWindow&win,sf::Font&font,const string&s,
              int size,sf::Color col,float x,float y){
    sf::Text t(font);
    t.setString(s); t.setCharacterSize(size);
    t.setFillColor(col); t.setPosition({x,y});
    win.draw(t);
}

// ── MAIN ─────────────────────────────────────────────────────
int main(){
    srand((unsigned)time(0));
    reset();

    sf::RenderWindow win(
        sf::VideoMode({(unsigned)(W*TILE),(unsigned)(H*TILE+UI)}),
        "Dungeon Dash");

    sf::Font font;
    if(!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) return -1;

    sf::RectangleShape tSh({TILE-1.f,TILE-1.f});
    sf::RectangleShape pSh({TILE-2.f,TILE-2.f}); pSh.setFillColor({80,220,80});
    sf::RectangleShape eSh({TILE-2.f,TILE-2.f});
    sf::RectangleShape poSh({TILE/2.f,TILE/2.f}); poSh.setFillColor({80,180,255});
    sf::RectangleShape btnSh({220.f,48.f});        btnSh.setFillColor({40,160,40});

    enum State{MENU,PLAY,WIN,LOSE};
    State state=MENU;
    bool prevF=false,prevE=false,prevR=false;

    while(win.isOpen()){
        while(auto ev=win.pollEvent())
            if(ev->is<sf::Event::Closed>()) win.close();
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) win.close();

        bool curE=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
        bool curR=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R);
        bool curF=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F);

        // ── MENU ──────────────────────────────────────────────
        if(state==MENU){
            win.clear({15,15,30});
            for(int y=0;y<H;y++) for(int x=0;x<W;x++)
                if((x+y)%6==0){
                    tSh.setPosition({(float)x*TILE,(float)y*TILE});
                    tSh.setFillColor({30,30,55}); win.draw(tSh);
                }
            float cx=W*TILE/2.f;
            sf::Text title(font); title.setString("Dungeon Dash");
            title.setCharacterSize(52); title.setFillColor({255,200,50});
            title.setPosition({cx-title.getGlobalBounds().size.x/2,H*TILE/2.f-120});
            win.draw(title);
            drawText(win,font,"Survive. Kill. Collect.",18,{180,180,180},cx-130,H*TILE/2.f-50);
            btnSh.setPosition({cx-110,H*TILE/2.f+20}); win.draw(btnSh);
            drawText(win,font,"Press ENTER to Start",20,{255,255,255},cx-105,H*TILE/2.f+30);
            drawText(win,font,"WASD:Move  F:Attack  R:Restart  Q:Quit",
                     14,{130,130,130},cx-170,H*TILE/2.f+100);
            win.display();
            if(curE&&!prevE){ reset(); state=PLAY; }
            prevE=curE;
            sf::sleep(sf::milliseconds(50));
            continue;
        }

        // ── WIN / LOSE ────────────────────────────────────────
        if(state==WIN||state==LOSE){
            win.clear({20,20,20});
            string msg=(state==WIN)?"YOU WIN!  Score: ":"GAME OVER! Score: ";
            msg+=to_string(score);
            float cx=W*TILE/2.f;
            drawText(win,font,msg,34,
                     state==WIN?sf::Color::Yellow:sf::Color::Red,cx-160,H*TILE/2.f-40);
            drawText(win,font,"R: Play Again    Q: Quit",20,{180,180,180},cx-130,H*TILE/2.f+30);
            win.display();
            if(curR&&!prevR) state=MENU;
            prevR=curR; prevE=curE;
            sf::sleep(sf::milliseconds(50));
            continue;
        }

        // ── INPUT ─────────────────────────────────────────────
        bool act=false,atk=false;
        if     (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)&&!isWall(pX,pY-1)){pY--;act=true;}
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)&&!isWall(pX,pY+1)){pY++;act=true;}
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)&&!isWall(pX-1,pY)){pX--;act=true;}
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)&&!isWall(pX+1,pY)){pX++;act=true;}

        if(curF&&!prevF){
            for(auto&e:enemies){
                if(!e.alive) continue;
                if(abs(pX-e.x)<=ATK&&abs(pY-e.y)<=ATK){
                    if(--e.hp<=0){e.alive=false;score++;}
                    atk=act=true; break;
                }
            }
        }
        prevF=curF; prevE=curE; prevR=curR;

        if(act) checkPotions();
        if(act&&!atk){ for(auto&e:enemies) moveEnemy(e); enemyTurn(); }

        if(pHP<=0)        state=LOSE;
        if(!aliveCount()) state=WIN;

        // ── DRAW ──────────────────────────────────────────────
        win.clear({15,15,15});
        for(int y=0;y<H;y++) for(int x=0;x<W;x++){
            tSh.setPosition({(float)x*TILE,(float)y*TILE});
            tSh.setFillColor(gMap[y][x]=='#'?sf::Color(100,100,120):sf::Color(30,30,35));
            win.draw(tSh);
        }
        for(auto&p:potions){
            if(!p.active) continue;
            poSh.setPosition({p.x*TILE+TILE/4.f,p.y*TILE+TILE/4.f});
            win.draw(poSh);
        }
        for(auto&e:enemies){
            if(!e.alive) continue;
            bool near=(abs(e.x-pX)<=ATK&&abs(e.y-pY)<=ATK);
            eSh.setFillColor(near?sf::Color(255,60,60):sf::Color(200,50,50));
            eSh.setPosition({e.x*TILE+1.f,e.y*TILE+1.f}); win.draw(eSh);
        }
        pSh.setPosition({pX*TILE+1.f,pY*TILE+1.f}); win.draw(pSh);

        sf::RectangleShape uiBg({(float)(W*TILE),(float)UI});
        uiBg.setPosition({0,(float)(H*TILE)}); uiBg.setFillColor({20,20,25});
        win.draw(uiBg);
        drawHP(win,10,H*TILE+10,150,14,pHP,P_HP,{80,220,80});
        drawText(win,font,
            "HP:"+to_string(pHP)+"/"+to_string(P_HP)+
            "  Enemies:"+to_string(aliveCount())+
            "  Score:"+to_string(score)+
            "  WASD:Move F:Atk R:Restart Q:Quit",
            15,sf::Color::White,10,H*TILE+32);
        drawText(win,font,"DUNGEON DASH",13,{255,200,50},W*TILE-125.f,H*TILE+5);

        win.display();
        sf::sleep(sf::milliseconds(FPS));
    }
    return 0;
}
