#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <time.h>

using namespace std;
using namespace sf;

enum Direction {

    TOP,
    RIGHT,
    BOTTOM,
    LEFT

};

map<string, int> Config;
map<string, Color> Colors;
Font MainFont;

// COLORS
Color BackgroundColor = Color{0xdee8ecFF};
Color EmptyCellColor = Color{0x93b0d0FF};
Color CellV2Color = Color{0xf77f00FF};
Color CellV4Color = Color{0xa2d729FF};
Color CellV8Color = Color{0xf77f00FF};

int GetRandomInt(int End);
int RandomIntInRange(int min, int max);
map<string, int> GetConfigValues();
map<string, Color> GetColorsValues();
tuple<string, int> DivideLineOnValues(string Line);
void LoadSource();
int GetCellRandomValue();

class Cell {
private:
    RectangleShape CellBackground;
    Text ValueLabel;
    bool IsEmpty = true;
    int Size;

public:

    Cell() {}

    Cell(int Size, int Value, Vector2f Position, bool IsEmpty, map<string, int> Config) {

        this->Size = Size;

        if (IsEmpty == true) {

            CellBackground = RectangleShape(Vector2f(Size, Size));
            CellBackground.setFillColor(EmptyCellColor);
            CellBackground.setPosition(Position.x, Position.y);

        } else {

            ValueLabel = Text(to_string(Value), MainFont);
            ValueLabel.setCharacterSize(34);
            ValueLabel.setStyle(Text::Bold); 

            CellBackground = RectangleShape(Vector2f(Size, Size));
            CellBackground.setPosition(Position.x, Position.y);

            float CellCenterPositionX = Position.x + Size / 2;
            float CellCenterPositionY = Position.y + Size / 2;

            ValueLabel.setPosition(CellCenterPositionX - ValueLabel.getLocalBounds().width / 2, 
                                   CellCenterPositionY - ValueLabel.getLocalBounds().height * 3 / 4);

            this->IsEmpty = false;

            UpdateColor();

        }

        CellBackground.setOutlineThickness(2);
        CellBackground.setOutlineColor(Color::Black);
        ValueLabel.setOutlineThickness(1);
        ValueLabel.setOutlineColor(Color::Black);

    }

    void Draw(RenderWindow *window) {

        window->draw(CellBackground);
        if (!IsEmpty)
            window->draw(ValueLabel);

    }

    void UpdateColor() {
        
        string ValueLabelText = ValueLabel.getString();
        int Value = stoi(ValueLabelText);

        switch (Value) {

            case 2:
                CellBackground.setFillColor(CellV2Color);
                break;

            case 4:
                CellBackground.setFillColor(CellV4Color);
                break;

                

        }

    }

    bool IsEmptyCell() {
        return IsEmpty;
    }

    void Move(Direction CellDirection) {

        switch (CellDirection) {

            case RIGHT:
                CellBackground.move(Size + Config["BORDERS"], 0);
                ValueLabel.move(Size + Config["BORDERS"], 0);
                break;

            case LEFT:
                CellBackground.move(-(Size + Config["BORDERS"]), 0);
                ValueLabel.move(-(Size + Config["BORDERS"]), 0);
                break;

        }

    }

};

void CreateCell(Vector2f GridPosition, int Value, vector<vector<Cell>> *Field, map<string, int> *Config);
void CreateCellInRandomPlace(vector<vector<Cell>> *Field, map<string, int> &Config);
void MergeRow(vector<vector<Cell>> *Field, map<string, int> Config, Direction MergeDirection);
void MergeRight(vector<vector<Cell>> *Field, map<string, int> &Config);

int main() {

    srand(time(0));
    LoadSource();

    Config = GetConfigValues();
    Colors = GetColorsValues();

    int WINDOW_WIDTH = Config["BORDERS"] * (Config["COLS"] + 1) + Config["COLS"] * Config["CELL_SIZE"];
    int WINDOW_HEIGHT = Config["BORDERS"] * (Config["ROWS"] + 1) + Config["ROWS"] * Config["CELL_SIZE"];

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2048");

    vector<vector<Cell>> Field(Config["ROWS"], vector<Cell>(Config["COLS"]));

    float MarginsY = (WINDOW_HEIGHT - Config["ROWS"] * Config["CELL_SIZE"]) / (Config["ROWS"] + 1);
    float MarginsX = (WINDOW_WIDTH - Config["COLS"] * Config["CELL_SIZE"]) / (Config["COLS"] + 1);

    float CellShiftX = MarginsX;
    float CellShiftY = MarginsY;

    for (int i = 0; i < Config["ROWS"]; i++) {

        for (int j = 0; j < Config["COLS"]; j++) {
            Field[i][j] = Cell(Config["CELL_SIZE"], 0, Vector2f(Config["CELL_SIZE"] * j + CellShiftX, Config["CELL_SIZE"] * i + CellShiftY), true, Config);
            CellShiftX += MarginsX;
        }

        CellShiftX = MarginsX;
        CellShiftY += MarginsY;

    }

    
    while (window.isOpen()) {

        Event event;

        while (window.pollEvent(event)) {

            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space) {
                CreateCellInRandomPlace(&Field, Config);
            }

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Up) {
                for (int i = 0; i < 5; i++) {
                        for (int j = 0; j < 5; j++)
                            cout << Field[i][j].IsEmptyCell();
                        cout << endl;
                    }
            }

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Right) {
                MergeRow(&Field, Config, Direction::RIGHT);
            }

        }

        window.clear(BackgroundColor);

        for (int i = 0; i < Config["ROWS"]; i++)
            for (int j = 0; j < Config["COLS"]; j++)
                Field[i][j].Draw(&window);

        window.display();

    }

    return 0;
}

map<string, int> GetConfigValues() {

    string Line;
    map<string, int> ConfigValues;
    ifstream ConfigFileStream("./config.txt");

    if (ConfigFileStream.is_open()) {

        while (getline(ConfigFileStream, Line)) {

            tuple<string, int> KeyAndValue = DivideLineOnValues(Line);
            ConfigValues[get<0>(KeyAndValue)] = get<1>(KeyAndValue);

        }

    }

    ConfigFileStream.close();

    return ConfigValues;

}

map<string, Color> GetColorsValues() {

    string Line;
    map<string, Color> ColorsValues;
    ifstream ConfigFileStream("./colors.txt");

    if (ConfigFileStream.is_open()) {

        while (getline(ConfigFileStream, Line)) {

            tuple<string, int> KeyAndValue = DivideLineOnValues(Line);
            ColorsValues[get<0>(KeyAndValue)] = Color{(uint)get<1>(KeyAndValue)};
            cout << get<1>(KeyAndValue) << endl;

        }

    }

    ConfigFileStream.close();

    return ColorsValues;

}

tuple<string, int> DivideLineOnValues(string Line) {

    int IndexOfEquals = -1;

    for (int i = 0; i < Line.length(); i++) {

        if (Line[i] == '=') {
            IndexOfEquals = i;
            break;
        }

    }

    return make_tuple(Line.substr(0, IndexOfEquals - 1), stoi(Line.substr(IndexOfEquals + 2, Line.length())));

}

void LoadSource() {

    if (!MainFont.loadFromFile("./Roboto-Regular.ttf")) {
        cout << "GG";
    }

}

void CreateCell(Vector2f GridPosition, int Value, vector<vector<Cell>> *Field, map<string, int> Config) {

    int PositionX = GridPosition.x * Config["CELL_SIZE"] + (GridPosition.x + 1) * Config["BORDERS"];
    int PositionY = GridPosition.y * Config["CELL_SIZE"] + (GridPosition.y + 1) * Config["BORDERS"];

    Field->at(GridPosition.y)[GridPosition.x] = Cell(Config["CELL_SIZE"], Value, Vector2f(PositionX, PositionY), false, Config);
    
    Config["CELL_COUNTS"] += 1;

}

int GetRandomInt(int End) {

    return rand() % End;

}

void CreateCellInRandomPlace(vector<vector<Cell>> *Field, map<string, int> &Config) {

    if (Config["CELLS_COUNT"] < Config["COLS"] * Config["ROWS"]) {

        int Col;
        int Row;

        do {

            Col = GetRandomInt(Config["COLS"]);
            Row = GetRandomInt(Config["ROWS"]);

        } while(Field->at(Row)[Col].IsEmptyCell() == false);

        int Value = GetCellRandomValue();

        CreateCell(Vector2f(Col, Row), Value, Field, Config);

        Config["CELLS_COUNT"] += 1;

    }

}


int GetCellRandomValue() {

   int Value = rand() % 2;
   Value = (Value == 0) ? 2 : 4;

   return Value;

}

void MergeRow(vector<vector<Cell>> *Field, map<string, int> Config, Direction MergeDirection) {

    switch (MergeDirection) {

        case RIGHT:
            MergeRight(Field, Config);
            break;

    }

}

void MergeRight(vector<vector<Cell>> *Field, map<string, int> &Config) {

    bool WasMerged = false;

    for (int i = 0; i < Config["ROWS"]; i++) {

        bool IsEmptyRow = true;

        for (int f = 0; f < Config["COLS"]; f++) {

            if (!Field->at(i)[f].IsEmptyCell()) {
                IsEmptyRow = false;
                break;
            }

        }

        if (!IsEmptyRow) {

            for (int j = Config["COLS"] - 1; j > 0; j--) {

                if (Field->at(i)[j].IsEmptyCell() && !Field->at(i)[j - 1].IsEmptyCell()) {

                    Field->at(i)[j].Move(Direction::LEFT);
                    Field->at(i)[j - 1].Move(Direction::RIGHT);

                    Cell TempCell = Field->at(i)[j];
                    Field->at(i)[j] = Field->at(i)[j - 1];
                    Field->at(i)[j - 1] = TempCell;

                    WasMerged = true;

                    cout << endl;

                }

            }


        }

    }

    if (WasMerged)
            CreateCellInRandomPlace(Field, Config);

}
