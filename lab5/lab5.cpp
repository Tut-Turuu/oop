#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "json.hpp"


using json = nlohmann::json;

class User {
public:
    int id;
    std::string name;
    std::string login;
    int password;
    std::string email;
    std::string address;

    std::string str() const {
		std::string str;

		str += "{id: ";					str += std::to_string(id);			str += ", ";
		str += "name: \"";				str += name;						str += "\", ";
		str += "login: \"";				str += login;						str += "\", ";
		str += "password (hashed): \"";	str += std::to_string(password);	str += "\", ";
		str += "email: \"";				str += email;						str += "\", ";
		str += "address: \"";			str += address;						str += "\"}";

		return str;
	}

    User(int id, const std::string& name, const std::string& login, const std::string& password) :
		id(id), name(name), login(login), password(hash(password)), email(), address() {

	}
    
    bool operator == (const User& user) {
		return id == user.id;
	}

    json to_json() const {
		return {
			{"id", id},
			{"name", name},
			{"login", login},
			{"password (hashed)", password},
			{"email", email},
			{"address", address}
		};
	}

	static User from_json(const json& j) {
		return User(
			j["id"].get<int>(),
			j["name"].get<std::string>(),
			j["login"].get<std::string>(),
			j["password (hashed)"].get<int>(),
			j["email"].get<std::string>(),
			j["address"].get<std::string>()
		);
	}

private:
	User(int id, const std::string& name, const std::string& login, int password_hashed, const std::string& email, const std::string& address) :
		id(id), name(name), login(login), password(password_hashed), email(email), address(address) {

	}

    int hash(const std::string& str) {
        int res = 0;
        for (char c: str) {
            srand(c);
            res += rand();
        }
        return res;
    }

};


template <typename T>
class IDataRepository {
public:
    virtual std::vector<T> get_all() = 0;
    virtual T* get_by_id(int id) = 0;
    virtual void add(const T& item) = 0;
    virtual void update(const T& item) = 0;
    virtual void del(const T& item) = 0;
	virtual ~IDataRepository() = default;
};


class IUserRepository: virtual public IDataRepository<User> {
public:
    virtual User* get_by_login(std::string login) = 0;
};


template <typename T>
class DataRepository: virtual public IDataRepository<T> {
protected:
        std::string file_name;
        std::vector<T> data;

        void load_data() {
            std::ifstream file;
            file.exceptions(std::ifstream::badbit);

            try {
                file.open(this->file_name);

                if (file.eof()) {
                    this->data.clear();
                    return;
                }

                json j = json::parse(file);
                this->data.clear();
                for (const auto& item : j) {
                    this->data.push_back(T::from_json(item));
                }
            }

            catch (const std::ifstream::failure& e) {
                std::cerr << "File error: " << e.what() << std::endl;
                throw;
            }

            catch (const json::exception& e) {
                std::cerr << "JSON error: " << e.what() << std::endl;
                this->data.clear();
                throw;
            }

            catch (const std::exception& e) {
                std::cerr << "Common error: " << e.what() << std::endl;
                throw;
            }

            file.close();
        }

        void save_data() {
            json j = json::array();

            for (const auto& item : this->data) {
                j.push_back(item.to_json());
            }

            std::ofstream file;
            file.exceptions(std::ofstream::badbit);

            try {
                file.open(this->file_name);
                file << j.dump(0);
            }
            catch (const std::ofstream::failure& fail) {
                std::cout << fail.what() << '\n';
            }

            file.close();
        }
public:

        DataRepository(const std::string& file_name): file_name(file_name) {
            load_data();
        }

        std::vector<T> get_all() override {
            return this->data;
        }

        T* get_by_id(int id) override {
            auto iter = std::find_if(this->data.begin(), this->data.end(), [id](const T& object) { return object.id == id; });

            if (iter == data.end()) {
                return nullptr;
            }

            return &(*iter);
        }

        void add(const T& object) override {
            auto iter = std::find_if(this->data.begin(), this->data.end(), [&object](const T& obj) { return obj.id == object.id; });

            if (iter != this->data.end()) {
                std::cout << "Object id " << object.id << " already in repository\n";
                return;
            }

            this->data.push_back(object);
            save_data();
        }

        void update(const T& object) override {
            auto iter = std::find_if(this->data.begin(), this->data.end(), [&object](const T& obj) { return obj.id == object.id; });

            if (iter == this->data.end()) {
                return;
            }

            *iter = object;
            save_data();
        }

        void del(const T& object) override {
            auto iter = std::find(this->data.begin(), this->data.end(), object);

            if (iter == this->data.end()) {
                return;
            }

            this->data.erase(iter);
            save_data();
        }

};

class UserRepository: public IUserRepository, public DataRepository<User> {
public:

    UserRepository(const std::string& file_name): DataRepository<User>(file_name) {}

    User* get_by_login(std::string login) {
        auto iter = std::find_if(this->data.begin(), this->data.end(), [login](const User& object) { return object.login == login; });

        if (iter == data.end()) {
            return nullptr;
        }

        return &(*iter);
    }
};


class IAuthService {
public:
    virtual void sign_in(const User& user) = 0;
    virtual void sign_out() = 0;
    virtual bool is_authorized() = 0;
    virtual User* current_user() = 0;
    virtual ~IAuthService() = default;

};


class AuthService: public IAuthService {
protected:
    UserRepository& repository;
    std::string auth_file_name;
    int user_id;
    bool is_auth = false;
    

    void load_auth() {
        std::ifstream file;
        file.exceptions(std::ifstream::badbit);

        try {
            file.open(this->auth_file_name);

            if (file.eof()) {
                return;
            }

            json j;
            file >> j;
            if (j.contains("id") && j["id"] != "null") {
                user_id = j["id"];

                if (!repository.get_by_id(user_id)) {
                    std::cout << "Stored user ID " << user_id << " not found in repository\n";
                }
            }
        }

        catch (const std::ifstream::failure& e) {
            std::cerr << "File error: " << e.what() << std::endl;
            throw;
        }

        catch (const json::exception& e) {
            std::cerr << "JSON error: " << e.what() << std::endl;
            throw;
        }

        catch (const std::exception& e) {
            std::cerr << "Common error: " << e.what() << std::endl;
            throw;
        }
    }

    void save_auth() const {
        json j;

        if (is_auth) {
            j["id"] = user_id;
        }
        else {
            j["id"] = "null";
        }

        std::ofstream file;
        file.exceptions(std::ofstream::badbit);

        try {
            file.open(this->auth_file_name);
            file << j.dump(0);
        }
        catch (const std::ofstream::failure& fail) {
            std::cout << fail.what() << '\n';
        }

        file.close();
    }

public:
    AuthService(UserRepository& repository, const std::string& auth_file_name) : repository(repository), auth_file_name(auth_file_name) {
        load_auth();
    }

    void sign_in(const User& user) override {
        user_id = user.id;
        is_auth = true;
        save_auth();
    }

    void sign_out() override {
        is_auth = false;
        save_auth();
    }

    bool is_authorized() override {
        return is_auth;
    }

    User* current_user() override {
        if (!is_auth) {
            throw std::runtime_error("Not authorized");
        }
        return repository.get_by_id(user_id);
    }
};




int main() {
    UserRepository user_repo("users.json");
    AuthService auth(user_repo, "auth.json");


    User user1(1, "cool_user", "cool_user123", "cool_password");
    user_repo.add(user1);
    
    if (user_repo.get_by_login(user1.login)) {
        auth.sign_in(user1);
        std::cout << "Logged in: " << auth.current_user()->str() << "\n";
    }

    std::cout << "Authorized: " << (auth.is_authorized() ? "Yes" : "No") << "\n\n\n";


    User user2(2, "Sasha", "Tut-Turuu", "qwerty");
    user_repo.add(user2);

    if (user_repo.get_by_login(user2.login)) {
        auth.sign_in(user2);
        std::cout << "New user: " << auth.current_user()->str() << "\n";
    }

    auth.sign_out();
    std::cout << "After logout: " << (auth.is_authorized() ? "Yes" : "No") << "\n\n\n";


    
    if (user_repo.get_by_login(user1.login)) {
        auth.sign_in(user1);
    }
    
    AuthService newAuth(user_repo, "auth.json");
    
    std::cout << "Current user after restart: " << auth.current_user()->str() << "\n\n\n";
    
}