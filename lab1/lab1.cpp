#include <iostream>
#include <sstream>
#include <cmath>

#define WIDTH 10
#define HEIGHT 10

class Point2d {
private:

    int x;
    int y;

public:
    void set_x(int n) {
        if (0 <= n && n <= WIDTH) {
            x = n;
        } else {
            std::cerr << "Point2d: x must be lower or eq " << WIDTH << " and greater or eq 0\n";
        }
    }

    void set_y(int n) {
        if (0 <= n && n <= HEIGHT) {
            y = n;
        } else {
            std::cerr << "Point2d: y must be lower or eq " << HEIGHT << " and greater or eq 0\n";
        }
    }

    int get_x() { return x; }
    int get_y() { return y; }


    Point2d(int x, int y) {
        set_x(x);
        set_y(y);
    }

    bool eq(const Point2d& other) {
        return x == other.x && y == other.y;
    }

    std::string str() {
        std::stringstream s;
        s << "{Point2d: x = " << x << ", y = " << y << "}"; 
        return s.str();
    }
};

class Vector2d;

class Iterator {
private:
    int i = 0;
    Vector2d& vec;
public:
    Iterator(Vector2d& vec): vec(vec) {}
    Iterator(Vector2d& vec, int i): vec(vec), i(i) {}

    bool operator==(const Iterator& other) {
        return i == other.i;
    }

    bool operator!=(const Iterator& other) {
        return i != other.i;
    }

    Iterator& operator++() {
        ++i;
        return (*this);
    }

    int& operator*();
};


class Vector2d {
private:
    int x;
    int y;

public:

    Vector2d(int x, int y): x(x), y(y) {

    }

    Vector2d(Point2d start, Point2d end): x(end.get_x() - start.get_x()), y(end.get_y() - start.get_y()) {

    }

    int& getitem(int i) {
        if (i == 0){
            return x;
        }
        return y;
    }

    void setitem(int i, int a) {
        if (i = 0) {
            x = a;
        } else {
            y = a;
        }
    }

    Iterator begin() {
        return Iterator(*this);
    }

    Iterator end() {
        return Iterator(*this, len());
    }

    int len() {
        return 2;
    }

    bool eq(const Vector2d& other) {
        return x == other.x && y == other.y;
    }
        
    std::string str() {
        std::stringstream s;
        s << "{Vector2d: x = " << x << ", y = " << y << "}"; 
        return s.str();
    }

    double abs() {
        return std::sqrt(x*x + y*y);
    }

    Vector2d operator+(const Vector2d& other) {
        return Vector2d(x + other.x, y + other.y);
    }

    Vector2d operator-(const Vector2d& other) {
        return Vector2d(x - other.x, y - other.y);
    }

    Vector2d operator*(int n) {
        return Vector2d(x * n, y * n);
    }

    Vector2d operator/(int n) {
        return Vector2d(x / n, y / n);
    }

    int inner_product(const Vector2d& other) {
        return x * other.x + y * other.y;
    }

    // Возвращает 3-ю координату векторного произведения (остальные равны нулю)
    int cross_product(const Vector2d& other) { 
        return x * other.y - y * other.x;
    }

    static int inner_product(const Vector2d& first, const Vector2d& second) {
        return first.x * second.x + first.y * second.y;
    }

    
    static int cross_product(const Vector2d& first, const Vector2d& second) { 
        return first.x * second.y - first.y * second.x;
    }

    int mixed_product(const Vector2d v2, const Vector2d v3) {
        return 0; // векторы компланарны
    }
};


int& Iterator::operator*() {
    return vec.getitem(i);
}



int main() {
    Point2d p1(1,2);

    std::cout << p1.str() << '\n';

    Vector2d v1(30, 60);
    Vector2d v2(3, 6);

    for (auto i = v1.begin(); i != v1.end(); ++i) {
        std::cout << *i << '\n';
    }
    std::cout << v1.str() << '\n';

    std::cout << v1.inner_product(v2*4)  << '\n';
    std::cout << v1.cross_product(v2)  << '\n';
    std::cout << Vector2d::cross_product(v1, v2)  << '\n';


}
