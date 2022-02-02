#include <iostream>
#include <vector>
#include <iomanip>
#include <exception>
#include <list>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <random>
#include <algorithm>



// pro kompilaci stačí:
//             g++ -std=c++17 -O2 main.cpp

// Pro odehrání tahu stačí napsat souřadnice místa, kam chcete hrát:
// první číslo řady (číslo v levo) potom sloupece (číslo v pravo) odedělené mezerou
// viz ---   2 3    
//   1 2 3
//  1
//  2    X
//  3



#define BWIDTH 15
#define BHEIGHT 15
#define CIRCLE 1
#define CROSS 2
#define coordsGood(y, x) (y >= 0 && x >= 0 && y < BHEIGHT && x < BWIDTH)
#define opponent(type) (3 - type)

// první AI :  double DNA0[6] = {0, -1, 0, 2, 6, 8};
// offensive týpek: double DNAX[6] = {0, 0, 0, 2, 2, 3};
// first breed lol: {-0.9, -1.2, -1, 0, 4.9, 9.5};

using namespace std;

class PlayBoard
{
    public: struct Data;
    public:
                //~PlayBoard  () { this->printBig(); }
                PlayBoard   ();
                PlayBoard   (vector<double> cross, vector<double> circle);
        void    print       (void)                      const;
        void    printBig    (void)                      const;
        void    set         (int y, int x, int type);
        void    remove      (int y, int x, int type);
        void    updateRow   (int y, int x, int movY, int movX);
        void    updateNode  (int y, int x, int movY, int movX, int type);
        void    setNodeData (int y, int x, int movY, int movX, int u, int type, bool create = true);
        void    deleteMove  (int n);
        void    addMove     (int n);
        double  minimax     (int depth, int maxMoves, int type, bool first, int & toPlay, double alpha, double beta, int player);
        double  boardHValue (int type);
        void    play        (int mode = 0);
        double  boardHVal   (int player);
        bool    isWinMove   (int move, int type);

    public:
        struct Data
        {
            int circleCreate[4] = {0}; 
            int crossCreate [4] = {0};
            int circleBlock [4] = {0}; 
            int crossBlock  [4] = {0};
            int active = 0;
            static bool cmpMoves(const Data & a, const Data & b, int type);
        };
        int board[BHEIGHT][BWIDTH] = {{0}};
        Data data[BHEIGHT][BWIDTH];
        list<int> possibleMoves;
        int numberOfMoves = 0;
        vector<double> DNAO;
        vector<double> DNAX;
        int lastMove = -1;
        bool end = false;
        bool tie = false;
};
bool PlayBoard::isWinMove(int move, int type)
{
    bool win = false;
    int y = move / BWIDTH;
    int x = move % BWIDTH;
    if (type == CIRCLE)
    {
        for (int i = 0; i < 4; ++i)
            if (data[y][x].circleCreate[i] == 5)
            {
                win = true;
                break;
            }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            if (data[y][x].crossCreate[i] == 5)
            {
                win = true;
                break;
            }
    }
    return win;
}
double PlayBoard::boardHVal(int player)
{
    double circle = 0, cross = 0;
    for (int move : possibleMoves)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (player == CIRCLE)
            {
                circle += DNAO[data[move / BWIDTH][move % BWIDTH].circleCreate[i]];
                cross  += DNAO[data[move / BWIDTH][move % BWIDTH].crossCreate[i]];
            }
            else
            {
                circle += DNAX[data[move / BWIDTH][move % BWIDTH].circleCreate[i]];
                cross  += DNAX[data[move / BWIDTH][move % BWIDTH].crossCreate[i]];
            }


        }
    }
    return circle - cross;
}
bool PlayBoard::Data::cmpMoves(const Data & a, const Data & b, int type)
{
    int aList[6] = {0};
    int bList[6] = {0};
    if (type == CIRCLE)
    {
        for (int i = 0; i < 4; ++i)
        {
            aList[a.circleCreate[i]] ++;
            aList[a.circleBlock [i]] ++;
            bList[b.circleCreate[i]] ++;
            bList[b.circleBlock [i]] ++;
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            aList[a.crossCreate[i]] ++;
            aList[a.crossBlock [i]] ++;
            bList[b.crossCreate[i]] ++;
            bList[b.crossBlock [i]] ++;
        }
    }
    for (int i = 5; i >= 0; --i)
    {
        if (aList[i] < bList[i])
            return true;
        if (aList[i] > bList[i])
            return false;
    }
    return false;
}
void PlayBoard::deleteMove(int n)
{
    auto it = find(possibleMoves.begin(), possibleMoves.end(), n);
    if (it != this->possibleMoves.end())
        this->possibleMoves.erase(it);
}
void PlayBoard::addMove(int n)
{
    this->possibleMoves.push_front(n);
}

void PlayBoard::updateRow(int y, int x, int movY, int movX)
{
    for (int i = 0; i < 5; ++i)
    {
        y += movY;
        x += movX;
        if (!coordsGood(y, x))
            break;
        if (this->board[y][x] == 0)
        {
            updateNode(y, x, movY, movX, CIRCLE);
            updateNode(y, x, movY, movX, CROSS);
        }
    }
}
void PlayBoard::updateNode(int y, int x, int movY, int movX, int type)
{
    int tightRight = 0, farRight = 0, spaceRight = 0, realRight = 0;
    int tightLeft  = 0, farLeft  = 0, spaceLeft  = 0, realLeft  = 0;
    int newY = y + movY, newX = x + movX;
    for (int i = 0; i < 4; ++i, newY += movY, newX += movX)
    {
        if (!coordsGood(newY, newX) || this->board[newY][newX] == opponent(type))
            break;
        if (this->board[newY][newX] == 0)
            spaceRight++;
        else if (spaceRight == 0)
            tightRight++;
        else if (spaceRight == 1)
            farRight++;
        realRight++;
    }
    newY = y - movY; newX = x - movX;
    for (int i = 0; i < 4; ++i, newY -= movY, newX -= movX)
    {
        if (!coordsGood(newY, newX) || this->board[newY][newX] == 3 - type)
            break;
        if (this->board[newY][newX] == 0)
            spaceLeft++;
        else if (spaceLeft == 0)
            tightLeft++;
        else if (spaceLeft == 1)
            farLeft++;
        realLeft++;
    }

    if (realLeft + realRight < 4)
    {
        setNodeData(y, x, movY, movX, 0, type);
        setNodeData(y, x, movY, movX, 0, opponent(type), false);
        return;
    }

    bool goLeft = true;
    int count;
    int countLeft  = farLeft   + tightLeft  + tightRight;
    int countRight = tightLeft + tightRight + farRight;
    if (countLeft > countRight)
    {
        goLeft = true;
        count = countLeft;
    }
    else
    {
        goLeft = false;
        count = countRight;
    }

    bool blockedTightLeft  = (realLeft  == tightLeft);
    bool blockedTightRight = (realRight == tightRight);
    bool blockedFarLeft  = blockedTightLeft  || (farLeft  != 0 && realLeft  == countLeft  + 1);
    bool blockedFarRight = blockedTightRight || (farRight != 0 && realRight == countRight + 1);
    bool blocked;

    if (goLeft)
        blocked = blockedFarLeft || blockedTightRight;
    else
        blocked = blockedTightLeft || blockedFarRight;
    
    if (count >= 4)
    {
        if (tightLeft + tightRight >= 4)
        {
            setNodeData(y, x, movY, movX, 5, type);
            setNodeData(y, x, movY, movX, 5, opponent(type), false);
            return;
        }
        else
            count = 3;
    }
    if (count == 3)
    {
        setNodeData(y, x, movY, movX, 4, type);
        if (!blocked)
            setNodeData(y, x, movY, movX, 4, opponent(type), false);
        else
            setNodeData(y, x, movY, movX, 2, opponent(type), false);
    }
    else if (count == 2)
    {
        if (!blocked)
        {
            setNodeData(y, x, movY, movX, 3, type); 
            setNodeData(y, x, movY, movX, 3, opponent(type), false);
        }
        else
        {
            setNodeData(y, x, movY, movX, 2, type); 
            setNodeData(y, x, movY, movX, 2, opponent(type), false);
        }
    }
    else if (count == 1)
    {
        setNodeData(y, x, movY, movX, 1, type); 
        setNodeData(y, x, movY, movX, 1, opponent(type), false);
        if (!blocked)
        {
            setNodeData(y, x, movY, movX, 2, type);
            setNodeData(y, x, movY, movX, 2, opponent(type), false);
        }
        else if (realLeft == 4 && this->board[y - movY * 3][x - movX * 3] == type && this->board[y - movY * 4][x - movX * 4])
        {
            setNodeData(y, x, movY, movX, 2, type);
            setNodeData(y, x, movY, movX, 2, opponent(type), false);
        }
        else if (realRight == 4 && this->board[y + movY * 3][x + movX * 3] == type && this->board[y + movY * 4][x + movX * 4])
        {
            setNodeData(y, x, movY, movX, 2, type);
            setNodeData(y, x, movY, movX, 2, opponent(type), false);
        }
    }
    else if (realLeft == 4 && this->board[y - movY * 3][x - movX * 3] == type)
    {
        setNodeData(y, x, movY, movX, 2, type);
        setNodeData(y, x, movY, movX, 2, opponent(type), false);
    }
    else if (realRight == 4 && this->board[y + movY * 3][x + movX * 3] == type)
    {
        setNodeData(y, x, movY, movX, 2, type);
        setNodeData(y, x, movY, movX, 2, opponent(type), false);
    }
    else
    {
        setNodeData(y, x, movY, movX, 1, type);
        setNodeData(y, x, movY, movX, 1, opponent(type), false);
    }
}
#define updatePossibleMoves(value, u)           \
    {if (value != 0 && u == 0)                  \
    {                                           \
        this->data[y][x].active--;              \
        if (this->data[y][x].active == 0)       \
            this->deleteMove(y * BWIDTH + x);   \
    }                                           \
    else if (value == 0 && u != 0)              \
    {                                           \
        this->data[y][x].active++;              \
        if (this->data[y][x].active == 1)       \
            this->addMove(y * BWIDTH + x);      \
    }                                           \
    value = u;}
void PlayBoard::setNodeData(int y, int x, int movY, int movX, int u, int type, bool create)
{
    int direction;
    Data & data = this->data[y][x];
    if (abs(movY + movX) == 2)
        direction = 3;
    else if (abs(movY + movX) == 0)
        direction = 1;
    else if (abs(movY) == 1)
        direction = 0;
    else
        direction = 2;
    if (type == CIRCLE)
    {
        if (create)
            updatePossibleMoves(data.circleCreate[direction], u)
        else
            updatePossibleMoves(data.circleBlock[direction], u)
    }
    else if (create)
        updatePossibleMoves(data.crossCreate[direction], u)
    else
        updatePossibleMoves(data.crossBlock[direction], u)
}
PlayBoard::PlayBoard()
{
    this->DNAO = {-5.3, -6.1, -5.4, -3.2, 8.7, 5.5};
    this->DNAX = {-5.3, -6.1, -5.4, -3.2, 8.7, 5.5};
    for (int i = 0; i < BHEIGHT; ++i)
    for (int j = 0; j < BWIDTH; ++j)
        this->data[i][j] = Data();
}
PlayBoard::PlayBoard(vector<double> cross, vector<double> circle)
: DNAO(circle), DNAX(cross)
{
    for (int i = 0; i < BHEIGHT; ++i)
    for (int j = 0; j < BWIDTH; ++j)
        this->data[i][j] = Data();
}
int movesY[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
int movesX[8] = {0, 1, 1, 1, 0, -1, -1, -1};

void PlayBoard::set(int y, int x, int type)
{
    deleteMove(y * BWIDTH + x);
    if (this->board[y][x] != 0)
        throw invalid_argument("was trying to set on not 0");
    
    this->numberOfMoves++;
    this->board[y][x] = type;
    
    for (int i = 0; i < 8; ++i)
        updateRow(y, x, movesY[i], movesX[i]);
}

void PlayBoard::remove(int y, int x, int type)
{
    if (this->board[y][x] == 0)
        throw invalid_argument("");
    
    this->numberOfMoves--;
    this->board[y][x] = 0;
    addMove(y * BWIDTH + x);
    for (int i = 0; i < 8; ++i)
        updateRow(y, x, movesY[i], movesX[i]);
}
void PlayBoard::printBig() const
{
    cout << "    ";
    for (int j = 0; j < BWIDTH; ++j)
        cout << " " << setw(2) << j + 1 << " ";
    cout << endl;

    for (int i = 0; i < BHEIGHT; ++i)
    {
        cout << "    ";
        for (int j = 0; j < BWIDTH; ++j)
            cout << "+---";
        cout << '+' << endl;

        cout << " " << setw(2) << i + 1 << " ";
        for (int j = 0; j < BWIDTH; ++j)
        {
            cout << "| ";
            if (this->board[i][j] == 0)
                cout << "  ";
            else if (this->board[i][j] == CIRCLE)
            {
                if (this->lastMove == i * BWIDTH + j)
                    cout << "O ";
                else
                    cout << "\033[1;34mO \033[0m";
            }
            else
            {
                if (this->lastMove == i * BWIDTH + j)
                    cout << "X ";
                else
                    cout << "\033[1;31mX \033[0m";
            }
        }
        cout << '|' << endl;
    }
    cout << "    ";
    for (int j = 0; j < BWIDTH; ++j)
        cout << "+---";
    cout << '+' << endl;
}
double PlayBoard::minimax(int depth, int maxMoves, int type, bool first, int & toPlay, double alpha, double beta, int player)
{
    //cout << depth << endl;
    if (depth == 0)
        return this->boardHVal(player);
    
    if (type == CIRCLE)
    {
        double maxEval = -INFINITY;
        int tmpMaxMoves = maxMoves;
        possibleMoves.sort([this](int a, int b) -> bool
        {
            return !PlayBoard::Data::cmpMoves(data[a / BWIDTH][a % BWIDTH], data[b / BWIDTH][b % BWIDTH], CIRCLE);
        });
        auto it = possibleMoves.begin();
        while(++it != possibleMoves.end() && --tmpMaxMoves >= 0){};
        list<int> bestMoves(possibleMoves.begin(), --it);
        if (first)
            toPlay = possibleMoves.front();
        for (auto move : bestMoves)
        {
            for (int i = 0; i < 4; ++i)
                if (data[move / BWIDTH][move % BWIDTH].circleCreate[i] == 5)
                {
                    if (first)
                        toPlay = move;
                    alpha = INFINITY;
                    return INFINITY;
                }

            set(move / BWIDTH, move % BWIDTH, CIRCLE);
            double eval = minimax(depth - 1, maxMoves, CROSS, false, toPlay, alpha, beta, player);
            if (eval > maxEval)
            {
                if (first)
                    toPlay = move;
                maxEval = eval;
            }
            remove(move / BWIDTH, move % BWIDTH, CIRCLE);
            alpha = max(alpha, eval);
            if (beta <= alpha)
                break;
            
        }
        //if (first) cout << maxEval << endl;
        return maxEval;
    }
    else
    {
        double minEval = INFINITY;
        int tmpMaxMoves = maxMoves;
        possibleMoves.sort([this](int a, int b) -> bool
        {
            return !PlayBoard::Data::cmpMoves(data[a / BWIDTH][a % BWIDTH], data[b / BWIDTH][b % BWIDTH], CROSS);
        });
        auto it = possibleMoves.begin();
        while(++it != possibleMoves.end() && --tmpMaxMoves >= 0){};
        list<int> bestMoves(possibleMoves.begin(), --it);
        if (first)
            toPlay = possibleMoves.front();
        for (auto move : bestMoves)
        {
            for (int i = 0; i < 4; ++i)
                if (data[move / BWIDTH][move % BWIDTH].crossCreate[i] == 5)
                {
                    if (first)
                        toPlay = move;
                    beta = -INFINITY;
                    return -INFINITY;
                }

            set(move / BWIDTH, move % BWIDTH, CROSS);
            double eval = minimax(depth - 1, maxMoves, CIRCLE, false, toPlay, alpha, beta, player);
            if (eval < minEval)
            {
                if (first)
                    toPlay = move;
                minEval = eval;
            }
            remove(move / BWIDTH, move % BWIDTH, CROSS);
            beta = min(beta, eval);
            if (beta <= alpha)
                break;
        }
        //if (first) cout << minEval << endl;
        return minEval;
    }
}

void PlayBoard::play(int mode)
{
    int type;
    int sth = -999;
    int deep = 10, maxmax = 11;
    if (mode == 0)
    {
        deep = 4;
        maxmax = 5;
    }
    else if (mode == 1)
    {
        deep = 6;
        maxmax = 8;
    }
    else if (numberOfMoves < 5)
        maxmax /= 2;

    if (numberOfMoves % 2 == 0)
        type = CROSS;
    else
        type = CIRCLE;

    if (numberOfMoves == 0)
        sth = 7 * 15 + 7;
    else if (possibleMoves.empty())
    {
        this->end = true;
        this->tie = true;
    }
    else
        minimax(deep, maxmax, type, true, sth, -INFINITY, INFINITY, type);

    
    int toPlay = sth;
    int x = toPlay % BWIDTH;
    int y = toPlay / BWIDTH;
    set(y, x, type);
    this->lastMove = sth;
    if (isWinMove(toPlay, type))
        this->end = true;
    //printBig();
    //cout << "char: ";
    //getchar();
}



int battle(vector<double> cross, vector<double> circle)
{
    PlayBoard desk(cross, circle);
    while (true)
    {
        desk.play();
        if (desk.tie)
            return 0;
        if (desk.end)
            return -1;
        desk.play();
        if (desk.tie)
            return 0;
        if (desk.end)
            return 1;
    }
}
vector<double> breed(vector<double> parent, int count)
{
    vector<vector<double> > childs(count, vector<double>(6, 0));
    //vector<vector<double> > childs = {
    //    {-0.9, -1.2, -1, 0, 4.9, 9.5},
    //    {-5.3, -6.1, -5.4, -3.2, 8.7, 5.5},
    //    {-7.7, -8, -7.8, -5.6, 7.5, 7.6}};
    vector<int> scoreBoard(count, 0);
    childs[0] = parent;
    for (int i = 1; i < count; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            double value = parent[j] + (rand() % 60) / 10.0 - 3;
            childs[i][j] = value;
        }
    }
    int fight = 0;
    for (int i = 0; i < count; ++i)
    {
        for (int j = i + 1; j < count && j > 1; ++j)
        {
            cout << ++fight << endl;
            try
            {
                int result = battle(childs[i], childs[j]);
                if (result == -1)
                    scoreBoard[i]++;
                if (result == 1)
                    scoreBoard[j]++;
            }
            catch(const std::exception& e)
            { }
            try
            {
                int result = battle(childs[j], childs[i]);
                if (result == -1)
                    scoreBoard[j]++;
                if (result == 1)
                    scoreBoard[i]++;
            }
            catch(const std::exception& e)
            { }
        }
    }
    for (int i = 0; i < count; ++i)
    {
        cout << "[";
        for (double s : childs[i])
            cout << s << " ][";
        cout << "\t\tscore: " << scoreBoard[i] << endl;
    }
    cout << endl;
    vector<double> theBest = childs[0];
    int max = scoreBoard[0];
    for (int i = 1; i < count; ++i)
    {
        if (scoreBoard[i] > max)
        {
            max = scoreBoard[i];
            theBest = childs[i];
        }
    }
    cout << "{";
    for (double d : theBest)
        cout <<  d << ", ";
    cout << "}" << endl;
    cout << "maxScore = " << max << endl;
    return theBest;
}
void playCross(int mode, vector<double> ai)
{
    PlayBoard desk(ai, ai);
    desk.printBig();
    while (true)
    {
        int x, y;
        cout << "Move: ";
        while (!(cin >> y >> x) || !coordsGood(y-1, x-1) || desk.board[y-1][x-1] != 0)
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input." << endl;
            cout << "Move: ";
        }
        desk.set(y - 1, x - 1, CROSS);
        if (desk.isWinMove((y - 1) * BWIDTH + x - 1, CROSS))
        {
            desk.printBig();
            cout << "You win!" << endl;
            break;
        }
        cout << "Thinking..." << endl;
        desk.play(mode);
        desk.printBig();
        if (desk.end)
        {
            cout << "You lost. :(" << endl;
            break;
        }
    }
}
void playCircle(int mode, vector<double> ai)
{
    PlayBoard desk(ai, ai);
    desk.printBig();
    while (true)
    {
        cout << "Thinking..." << endl;
        desk.play(mode);
        desk.printBig();
        if (desk.end)
        {
            cout << "You lost. :(" << endl;
            break;
        }
        int x, y;
        cout << "Move: ";
        while (!(cin >> y >> x) || !coordsGood(y-1, x-1) || desk.board[y-1][x-1] != 0)
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input." << endl;
            cout << "Move: ";
        }
        desk.set(y - 1, x - 1, CIRCLE);
        if (desk.isWinMove((y - 1) * BWIDTH + x - 1, CIRCLE))
        {
            cout << "You win!" << endl;
            break;
        }
    }
}

int main(int argc, char ** argv)
{
    cout << "Select mode: " << endl;
    cout << "1 : light mode  - easy" << endl;
    cout << "2 : normal mode - medium" << endl;
    cout << "3 : heavy mode  - hard" << endl;
    int input, mode;
    cout << "-";
    while (!(cin >> input) || input < 1 || input > 3)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input." << endl;
        cout << "-";
    }
    switch (input)
    {
        case 1: mode = 0; break;
        case 2: mode = 1; break;
        case 3: mode = 2; break;
    }

    cout << "Select opponent: " << endl;
    cout << "1 : random guy" << endl;
    cout << "2 : 2nd place" << endl;
    cout << "3 : 1st place" << endl;
    vector<double> opp;
    cout << "-";
    while (!(cin >> input) || input < 1 || input > 3)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input." << endl;
        cout << "-";
    }
    switch (input)
    {
        case 1: opp = {-7.7, -8, -7.8, -5.6, 7.5, 7.6}; break;
        case 2: opp = {-5.3, -6.1, -5.4, -3.2, 8.7, 5.5}; break;
        case 3: opp = {-0.9, -1.2, -1, 0, 4.9, 9.5}; break;
    }

    cout << "Select: " << endl;
    cout << "1 : cross" << endl;
    cout << "2 : circle" << endl;
    int shape;
    cout << "-";
    while (!(cin >> input) || input < 1 || input > 2)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input." << endl;
        cout << "-";
    }
    switch (input)
    {
        case 1: shape = CROSS; break;
        case 2: shape = CIRCLE; break;
    }

    if (shape == CROSS)
        playCross(mode, opp);
    else
        playCircle(mode, opp);

    cout << "End of Game." << endl;
    return 0;
}


// --- TOP 10
// {0, -1, 0, 2, 6, 8}
// {-0.9, -1.2, -1, 0, 4.9, 9.5};
// {-0.7, -2.9, -4.6, 0.1, 5.5, 8.2, }
//{-0.7, {-1.3, {-3.2, {1, {7.2, {10.7, }
//{-1.5, -2, -3.5, -0.3, 5, 5.7, }
//{-3.5, -4.3, -5.3, -2.4, 6.4, 3.7}
//{-5.3, -6.1, -5.4, -3.2, 8.7, 5.5}
//{-3.7, -5.5, -3.8, -1, 11.6, 4.3}
//{-7.7, -8, -7.8, -5.6, 7.5, 7.6}
//{-10.1, -9.3, -11, -4.6, 6.7, 9}


// 4, 8

//[0 ][-1 ][0 ][2 ][6 ][8 ][                      score: 5
//[-0.9 ][-1.2 ][-1 ][0 ][4.9 ][9.5 ][            score: 10
//[-0.7 ][-2.9 ][-4.6 ][0.1 ][5.5 ][8.2 ][        score: 11
//[-0.7 ][-1.3 ][-3.2 ][1 ][7.2 ][10.7 ][         score: 10
//[-1.5 ][-2 ][-3.5 ][-0.3 ][5 ][5.7 ][           score: 4    -- looser
//[-3.5 ][-4.3 ][-5.3 ][-2.4 ][6.4 ][3.7 ][       score: 10
//   [-5.3 ][-6.1 ][-5.4 ][-3.2 ][8.7 ][5.5 ][    score: 14   < ***** WINNER
//[-3.7 ][-5.5 ][-3.8 ][-1 ][11.6 ][4.3 ][        score: 4    -- looser
//[-7.7 ][-8 ][-7.8 ][-5.6 ][7.5 ][7.6 ][         score: 13
//[-10.1 ][-9.3 ][-11 ][-4.6 ][6.7 ][9 ][         score: 9

// 7, 8

//[0 ][-1 ][0 ][2 ][6 ][8 ][                      score: 11
//[-0.9 ][-1.2 ][-1 ][0 ][4.9 ][9.5 ][            score: 9
//[-0.7 ][-2.9 ][-4.6 ][0.1 ][5.5 ][8.2 ][        score: 7
//[-0.7 ][-1.3 ][-3.2 ][1 ][7.2 ][10.7 ][         score: 3   -- looser
//[-1.5 ][-2 ][-3.5 ][-0.3 ][5 ][5.7 ][           score: 7
//[-3.5 ][-4.3 ][-5.3 ][-2.4 ][6.4 ][3.7 ][       score: 8
//[-5.3 ][-6.1 ][-5.4 ][-3.2 ][8.7 ][5.5 ][       score: 12  < ***** WINNER
//[-3.7 ][-5.5 ][-3.8 ][-1 ][11.6 ][4.3 ][        score: 10
//[-7.7 ][-8 ][-7.8 ][-5.6 ][7.5 ][7.6 ][         score: 12  < ***** WINNER
//[-10.1 ][-9.3 ][-11 ][-4.6 ][6.7 ][9 ][         score: 11


// 8, 11

//[0 ][-1 ][0 ][2 ][6 ][8 ][                      score: 9
//[-0.9 ][-1.2 ][-1 ][0 ][4.9 ][9.5 ][            score: 13   < ***** WINNER
//[-0.7 ][-2.9 ][-4.6 ][0.1 ][5.5 ][8.2 ][        score: 6
//[-0.7 ][-1.3 ][-3.2 ][1 ][7.2 ][10.7 ][         score: 6
//[-1.5 ][-2 ][-3.5 ][-0.3 ][5 ][5.7 ][           score: 8
//[-3.5 ][-4.3 ][-5.3 ][-2.4 ][6.4 ][3.7 ][       score: 5    -- looser
//[-5.3 ][-6.1 ][-5.4 ][-3.2 ][8.7 ][5.5 ][       score: 10
//[-3.7 ][-5.5 ][-3.8 ][-1 ][11.6 ][4.3 ][        score: 12
//[-7.7 ][-8 ][-7.8 ][-5.6 ][7.5 ][7.6 ][         score: 12
//[-10.1 ][-9.3 ][-11 ][-4.6 ][6.7 ][9 ][         score: 8


//-----TOTAL
//[0 ][-1 ][0 ][2 ][6 ][8 ][                      score: 25
//[-0.9 ][-1.2 ][-1 ][0 ][4.9 ][9.5 ][            score: 32 ---- 3. PLACE
//[-0.7 ][-2.9 ][-4.6 ][0.1 ][5.5 ][8.2 ][        score: 24
//[-0.7 ][-1.3 ][-3.2 ][1 ][7.2 ][10.7 ][         score: 19
//[-1.5 ][-2 ][-3.5 ][-0.3 ][5 ][5.7 ][           score: 19
//[-3.5 ][-4.3 ][-5.3 ][-2.4 ][6.4 ][3.7 ][       score: 23
//[-5.3 ][-6.1 ][-5.4 ][-3.2 ][8.7 ][5.5 ][       score: 36 ---- 2. PLACE
//[-3.7 ][-5.5 ][-3.8 ][-1 ][11.6 ][4.3 ][        score: 26
//[-7.7 ][-8 ][-7.8 ][-5.6 ][7.5 ][7.6 ][         score: 37 ---- 1. PLACE
//[-10.1 ][-9.3 ][-11 ][-4.6 ][6.7 ][9 ][         score: 28



// MasterMinds
//  {-7.7, -8, -7.8, -5.6, 7.5, 7.6}}
//  {-5.3, -6.1, -5.4, -3.2, 8.7, 5.5}
//  {-0.9, -1.2, -1, 0, 4.9, 9.5}
