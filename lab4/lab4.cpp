#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>

class INotifyDataChanged;


class IPropertyChangedListener {
public:
    virtual void on_property_changed(INotifyDataChanged* object, const std::string& property_name) = 0;
    virtual ~IPropertyChangedListener() = default;
};


class INotifyDataChanged {
public:

    virtual void add_property_changed_listener(IPropertyChangedListener* listener) = 0;
    virtual void remove_property_changed_listener(IPropertyChangedListener* listener) = 0;
    virtual std::string str() = 0;
    virtual ~INotifyDataChanged() = default;
};


class INotifyDataChanging;


class IPropertyChangingListener {
public:
    virtual bool on_property_changing(INotifyDataChanging* object, const std::string& property_name, const std::string& old_value, const std::string& new_value) = 0;
    virtual ~IPropertyChangingListener() = default;
};


class INotifyDataChanging {
public:
    virtual void add_property_changing_listener(IPropertyChangingListener* listener) = 0;
    virtual void remove_property_changing_listener(IPropertyChangingListener* listener) = 0;
    virtual std::string str() = 0;
    virtual ~INotifyDataChanging() = default;
};



class User: public INotifyDataChanged {
    std::string username;
    std::string password;

    std::vector<IPropertyChangedListener*> listeners;

public:
    ~User() noexcept = default;

    void add_property_changed_listener(IPropertyChangedListener* listener) override {
        listeners.push_back(listener);
    }
    void remove_property_changed_listener(IPropertyChangedListener* listener) override {
        auto iter = std::find(listeners.begin(), listeners.end(), listener);

        if (iter != listeners.end()) {
            listeners.erase(iter);
        }
    }

    std::string str() override {
        std::stringstream s;
        s << "{User: {username: " << username << ", password: " << password << "}}";
        return s.str();
    }

    void set_username(const std::string& new_name) {
        if (username == new_name) return;
        username = new_name;

        for (auto listener: listeners) {
            listener->on_property_changed(this, "username");
        }
    }

    void set_password(const std::string& new_pass) {
        if (password == new_pass) return;
        password = new_pass;

        for (auto listener: listeners) {
            listener->on_property_changed(this, "password");
        }
    }
};



class SecureUser: public INotifyDataChanging {
    std::string username;
    std::string password;

    std::vector<IPropertyChangingListener*> listeners;

public:
    ~SecureUser() noexcept = default;

    void add_property_changing_listener(IPropertyChangingListener* listener) override {
        listeners.push_back(listener);
    }
    void remove_property_changing_listener(IPropertyChangingListener* listener) override {
        auto iter = std::find(listeners.begin(), listeners.end(), listener);

        if (iter != listeners.end()) {
            listeners.erase(iter);
        }
    }

    std::string str() override {
        std::stringstream s;
        s << "{SecureUser: {username: " << username << ", password: " << password << "}}";
        return s.str();
    }


    void set_username(const std::string& new_name) {
        if (this->username == new_name) return;

        for (auto listener : listeners) {
            if (!listener->on_property_changing(this, "username", username, new_name)) {
                return;
            }
        }

        username = new_name;
    }

    void set_password(const std::string& new_pass) {
        if (password == new_pass) return;

        for (auto listener : listeners) {
            if (!listener->on_property_changing(this, "password", password, new_pass)) {
                return;
            }
        }
        password = new_pass;

    }
};


class UsernameChangedListener: public IPropertyChangedListener {
public:
    void on_property_changed(INotifyDataChanged* object, const std::string& property_name) override {
        if (property_name != "username") return;
        std::cout << property_name << " of " << object->str() << " has been changed" << std::endl;
    }
};


class PasswordChangedListener: public IPropertyChangedListener {
public:
    void on_property_changed(INotifyDataChanged* object, const std::string& property_name) override {
        if (property_name != "password") return;
        std::cout << property_name << " of " << object->str() << " has been changed" << std::endl;
    }
};


class UsernameChangingListener: public IPropertyChangingListener {
public:
    bool on_property_changing(INotifyDataChanging* object, const std::string& property_name, const std::string& old_value, const std::string& new_value) override {
        if (property_name != "username") return true;

        if (new_value.find("bad word") != std::string::npos) {
            std::cout << "Cant change username of " << object->str() << " to " << new_value << " (contains bad word)" << std::endl;
            return false;
        }
        return true;
    }
};


class PasswordChangingListener: public IPropertyChangingListener {
public:
    bool on_property_changing(INotifyDataChanging* object, const std::string& property_name, const std::string& old_value, const std::string& new_value) override {
        if (property_name != "password") return true;

        if (new_value.find("qwerty") != std::string::npos) {
            std::cout << "Cant change password of " << object->str() << " to " << new_value << " (too weak)" << std::endl;
            return false;
        }
        return true;
    }
};



int main() {

    User user;
    SecureUser sec_user;

    UsernameChangedListener username_changed_listener;
    UsernameChangingListener username_changing_listener;
    PasswordChangedListener password_changed_listener;
    PasswordChangingListener password_changing_listener;


    
    user.add_property_changed_listener(&username_changed_listener);
    user.add_property_changed_listener(&password_changed_listener);

    sec_user.add_property_changing_listener(&username_changing_listener);
    sec_user.add_property_changing_listener(&password_changing_listener);

    user.set_username("cool guy");
    user.set_password("qwerty");

    sec_user.set_username("bad word");
    sec_user.set_password("qwerty");
}