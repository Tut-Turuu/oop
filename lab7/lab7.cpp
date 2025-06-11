#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <any>


enum class LifeStyle {
    per_request,
    scoped,
    singleton
};

// Базовый класс для всех интерфейсов
class IInterface {
public:
    virtual ~IInterface() = default;
};


class Injector {

private:
    struct Registration {
        std::function<std::any()> creator;
        LifeStyle lifeStyle;
        IInterface* instance;
    };

    std::unordered_map<std::string, Registration> registrations;
    bool in_scope = false;

    void begin_scope() {
        in_scope = true;
    }

    void end_scope() {
        in_scope = false;

        for (auto& pair : registrations) {
            if (pair.second.lifeStyle == LifeStyle::scoped) {
                delete pair.second.instance;
                pair.second.instance = nullptr;
            }
        }
    }

    template<typename T, typename... Args>
    IInterface* create_instance(Args&&... args) {
        return dynamic_cast<IInterface*>(new T(std::forward<Args>(args)...));
    }

public:

    ~Injector() {
        for (auto& [k, v]: registrations) {
            delete v.instance;
        }
    }
    // Регистрация зависимости
    template<typename Interface, typename Implementation, typename... Args>
    void register_type(LifeStyle lifeStyle = LifeStyle::per_request, Args&&... args) {
        auto creator = [this, args...]() {
            return create_instance<Implementation>(args...);
        };
        
        registrations[typeid(Interface).name()] = Registration{
            creator,
            lifeStyle
        };
    }

    // // Регистрация фабричного метода
    // template<typename Interface, typename Factory>
    // void register_factory(Factory&& factory) {
    //     registrations[typeid(Interface).name()] = Registration{
    //         factory, //[factory]() { return factory(); },
    //         LifeStyle::per_request // Фабричные методы всегда создают новый экземпляр
    //     };
    // }


    template<typename Interface>
    Interface* get_instance() {
        auto it = registrations.find(typeid(Interface).name());
        if (it == registrations.end()) {
            throw std::runtime_error("No registration found for type");
        }

        auto& registration = it->second;

        switch (registration.lifeStyle) {
            case LifeStyle::singleton:
                if (!(bool)registration.instance) {
                    registration.instance = std::any_cast<IInterface*>(registration.creator());
                }
                return dynamic_cast<Interface*>(registration.instance);
            
            case LifeStyle::scoped:
                if (!in_scope) {
                    throw std::runtime_error("Cannot get scoped instance outside of scope");
                }
                if (!(bool)registration.instance) {
                    registration.instance = std::any_cast<IInterface*>(registration.creator());
                }
                return  dynamic_cast<Interface*>(registration.instance);
            
            case LifeStyle::per_request:
                delete registration.instance;
                registration.instance = std::any_cast<IInterface*>(registration.creator());
                return dynamic_cast<Interface*>(registration.instance);
            default:
                return dynamic_cast<Interface*>(registration.instance);
        }
    }

    // Область видимости
    class Scope {
    public:
        explicit Scope(Injector& injector) : injector(injector) {
            injector.begin_scope();
        }
        ~Scope() {
            injector.end_scope();
        }
    private:
        Injector& injector;
    };
};


class Interface1: public IInterface {
public:
    virtual void hello1() = 0;
};


class Interface2: public IInterface {
public:
    virtual void hello2() = 0;
};


class Interface3: public IInterface {
public:
    virtual void hello3() = 0;
};


class Class1_debug: public Interface1 {
    std::string str;
public:
    Class1_debug(const std::string& str): str(str) {}

    void hello1() override {
        std::cout << "Hello from class 1 debug, i have string " << str << "\n";
    }
};


class Class1_release: public Interface1 {
public:
    void hello1() override {
        std::cout << "Hello from class 1 release\n";
    }
};



class Class2_debug: public Interface2 {
    int x;
    int y;
public:
    Class2_debug(int x, int y): x(x), y(y) {}

    void hello2() override {
        std::cout << "Hello from class 2 debug and i at " << x << ' ' << y << " coordinates\n";
    }
};


class Class2_release: public Interface2 {
public:
    void hello2() override {
        std::cout << "Hello from class 2 release\n";
    }
};



class Class3_debug: public Interface3 {
public:
    void hello3() override {
        std::cout << "Hello from class 3 debug and i cant do anything\n";
    }
};


class Class3_release: public Interface3 {
public:
    void hello3() override {
        std::cout << "Hello from class 3 release\n";
    }
};


int main() {
    Injector injector;

    // configuration number one

    injector.register_type<Interface1, Class1_release>(LifeStyle::per_request);
    injector.register_type<Interface2, Class2_release>(LifeStyle::scoped);
    injector.register_type<Interface3, Class3_release>(LifeStyle::singleton);



    auto inst = injector.get_instance<Interface1>();
    inst->hello1();

    Interface2* inst_scoped1;
    Interface2* inst_scoped2;


    {
        Injector::Scope scope(injector);
        inst_scoped1 = injector.get_instance<Interface2>();
        inst_scoped1->hello2();
    }
    {
        Injector::Scope scope(injector);
        auto inst_scoped2 = injector.get_instance<Interface2>();
    }

    std::cout << "is instances from different scope are the same?: " << (inst_scoped1 == inst_scoped2 ? "Yes" : "No") << "\n";

    Interface3* inst_singleton1;
    Interface3* inst_singleton2;

    inst_singleton1 = injector.get_instance<Interface3>();
    inst_singleton2 = injector.get_instance<Interface3>();

    inst_singleton1->hello3();

    std::cout << "is singleton instances from different calls are the same?: " << (inst_singleton1 == inst_singleton2 ? "Yes" : "No") << "\n";


    // configuration number two

    injector.register_type<Interface1, Class1_debug>(LifeStyle::per_request, "hello world");
    injector.register_type<Interface2, Class2_debug>(LifeStyle::scoped, 101, 37);
    injector.register_type<Interface3, Class3_debug>(LifeStyle::singleton);

    auto inst1 = injector.get_instance<Interface1>();

    inst1->hello1();

    {
        Injector::Scope scope(injector);
        auto inst2 = injector.get_instance<Interface2>();

        inst2->hello2();
    }

    auto inst3 = injector.get_instance<Interface3>();

    inst3->hello3();

}