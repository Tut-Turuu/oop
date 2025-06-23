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
    int size;
public:

    CharTable(const std::string filename, int size): size(size) {
        alphabet = new char**[27];

        for (int i = 0; i < 27; ++i) {
            alphabet[i] = new char*[size];
            for (int j = 0; j < size; ++j) {
                alphabet[i][j] = new char[size];
            }
        }
        
        std::ifstream fin;
        fin.open(filename);


        char trash;
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < 27; ++j) {
                
                fin.read(alphabet[j][i], size);
                
            }
            fin.read(&trash, 1);
        }
        fin.close();
        
    }
    int get_row_count() {
        
        return size;
    }

    std::string get_row(char c, int n) {
        std::string str;
        str.resize(size);
        
        memcpy(str.data() , alphabet[c][n], size);
        
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
    int size;
public:
    static CharTable char_table5;
    static CharTable char_table7;




    Printer(Color color, std::pair<int, int> position, char symbol, int char_size): color(color), position(position), symbol(symbol), size(char_size) {}

    void print(std::string text) {
        CharTable* char_table;

        if (size == 5) {
            char_table = &Printer::char_table5;
        } else if (size == 7) {
            char_table = &Printer::char_table7;
        } else {
            std::cerr << "cant find font with size: " << size << std::endl;
            return ;
        }

        for (int i = 0; i < position.second; ++i) {
            std::cout << '\n';
        }

        std::string tmp;
        std::cout << "\u001b[" << (int)color << "m";
        
        for (int i = 0; i < char_table->get_row_count(); ++i) {
            
            for (int j = 0; j < position.first; ++j) {
                std::cout << ' ';
            }
            
            for (char c: text) {
                tmp = char_table->get_row(char_to_index(c), i);
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

    static void print(std::string text, Color color, std::pair<int, int> position, char symbol, int char_size) {
        Printer printer(color, position, symbol, char_size);
        printer.print(text);
    
    }



};

CharTable Printer::char_table5("ascii_5*5.txt", 5);

CharTable Printer::char_table7("ascii_7*7.txt", 7);

int main() {


    Printer printer(Color::Blue, std::pair<int, int>(1, 1), '?', 5);

    printer.print("hello oop");

    Printer::print("abc", Color::Red, std::pair<int, int>(10,2), '1', 7);
    Printer::print("red", Color::Green, std::pair<int, int>(0,0), '@', 5);


}