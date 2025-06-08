#include <string>
#include <iostream>
#include <fstream>
#include <cstring>



int char_to_index(const char c) {
    if (c == ' ') return 26;
    return c - 'a';
}

class CharTable {
    char*** alphabet;
public:

    CharTable(const std::string filename) {
        alphabet = new char**[27];

        for (int i = 0; i < 27; ++i) {
            alphabet[i] = new char*[7];
            for (int j = 0; j < 7; ++j) {
                alphabet[i][j] = new char[7];
            }
        }

        std::ifstream fin;
        fin.open(filename);


        char trash;
        
        for (int i = 0; i < 7; ++i) {
            for (int j = 0; j < 27; ++j) {
                
                fin.read(alphabet[j][i], 7);
                
            }
            fin.read(&trash, 1);
        }
        fin.close();
    }
    int get_row_count() {
        return 7;
    }
    std::string get_row(char c, int n) {
        std::string str;
        str.resize(7);
        memcpy(str.data() , alphabet[c][n], 7);
        return str;
    }
};



enum class Color {
    Red = 31, Green = 32, Blue = 34
};

class Printer {
    std::string text;
    Color color;
    std::pair<int, int> position;
    char symbol;

    static CharTable char_table;

public:

    Printer(Color color, std::pair<int, int> position, char symbol): color(color), position(position), symbol(symbol) {}

    void print(std::string text) {
        for (int i = 0; i < position.second; ++i) {
            std::cout << '\n';
        }

        std::string tmp;
        std::cout << "\u001b[" << (int)color << "m";
        for (int i = 0; i < char_table.get_row_count(); ++i) {
            for (int j = 0; j < position.first; ++j) {
                std::cout << ' ';
            }
            for (char c: text) {
                tmp = char_table.get_row(char_to_index(c), i);
                for (char ch: tmp) {
                    if (ch == '#') {
                        std::cout << symbol;
                    } else if(ch == ' ') {
                        std::cout << ' ';
                    }
                }
                std::cout << ' ';
            }

            std::cout << '\n';
        }
        std::cout << "\u001b[0m";
    }

    static void print(std::string text, Color color, std::pair<int, int> position, char symbol) {
        Printer printer(color, position, symbol);
        printer.print(text);
    
    }



};


CharTable Printer::char_table("ascii_7*7.txt");

int main() {

    Printer printer(Color::Blue, std::pair<int, int>(1, 1), '?');

    printer.print("hello oop");

    Printer::print("abc", Color::Red, std::pair<int, int>(10,2), '1');
    Printer::print("red", Color::Green, std::pair<int, int>(0,0), '@');


}