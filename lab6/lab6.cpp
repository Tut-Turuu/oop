#include <fstream>
#include <iostream>
#include <stack>
#include <map>
#include "json.hpp"


using json = nlohmann::json;


class Command {
public:
    virtual ~Command() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string get_type() const = 0;
    virtual json get_params() const = 0;
};


class Editor {
private:
    std::string text;
    int volume;
    bool media_playing;
    std::ofstream log_file;
public:
    Editor() : volume(50), media_playing(false) {
        log_file.open("output.txt");
    }

    ~Editor() {
        log_file.close();
    }

    void print_char(char c) {
        text += c;
        std::cout << c << std::endl;
        log_file << text << std::endl;
    }

    void delete_last_char() {
        if (!text.empty()) {
            text.pop_back();
            std::cout << "Undo: " << text << std::endl;
            log_file << text << std::endl;
        }
    }

    void increase_volume(int step) {
        volume = std::min(volume + step, 100);
        std::cout << "Volume increased to " << volume << "%" << std::endl;
        log_file << "Volume increased to " << volume << "%" << std::endl;
    }

    void decrease_volume(int step) {
        volume = std::max(volume - step, 0);
        std::cout << "Volume decreased to " << volume << "%" << std::endl;
        log_file << "Volume decreased to " << volume << "%" << std::endl;
    }

    void toggle_media_player() {
        media_playing = !media_playing;
        std::string status = media_playing ? "launched" : "closed";
        std::cout << "Media player " << status << std::endl;
        log_file << "Media player " << status << std::endl;
    }
};

class Keyboard {
private:
    std::map<std::string, Command*> key_commands;
    std::stack<Command*> undo_stack;
    std::stack<Command*> redo_stack;
    Editor& editor;

public:
    Keyboard(Editor& ed);

    ~Keyboard();

    void bind_key(const std::string& keyCombo, Command* cmd);

    void press_key(const std::string& keyCombo);

    void undo();
    
    void redo();

    json get_state() const;

    void set_state(const json& state);
};



class KeyCommand: public Command {
private:
    Editor& editor;
    char character;
public:
    KeyCommand(Editor& editor, char c) : editor(editor), character(c) {}

    void execute() {
        editor.print_char(character);
    }

    void undo() {
        editor.delete_last_char();
    }

    std::string get_type() const {
        return "PrintChar";
    }

    json get_params() const {
        return { {"char", std::string(1, character)} };
    }
};


class KeyboardStateSaver {
public:
    void save(const Keyboard& keyboard, const std::string& filename) {
        json state = keyboard.get_state();
        std::ofstream file(filename);
        if (file.is_open()) {
            file << state.dump(4);
            file.close();
        }
    }

    void load(Keyboard& keyboard, const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            json state;
            file >> state;
            keyboard.set_state(state);
            file.close();
        }
    }
};

class MediaPlayerCommand: public Command {
private:
    Editor& editor;
public:
    MediaPlayerCommand(Editor& ed) : editor(ed) {}

    void execute() {
        editor.toggle_media_player();
    }

    void undo() {
        editor.toggle_media_player();
    }

    std::string get_type() const {
        return "MediaPlayer";
    }

    json get_params() const {
        return {};
    }
};

class VolumeUpCommand: public Command {
private:
    Editor& editor;
    int step;
public:
    VolumeUpCommand(Editor& ed, int s) : editor(ed), step(s) {}

    void execute() {
        editor.increase_volume(step);
    }

    void undo() {
        editor.decrease_volume(step);
    }

    std::string get_type() const {
        return "VolumeUp";
    }

    json get_params() const {
        return { {"step", step} };
    }
};

class VolumeDownCommand: public Command {
private:
    Editor& editor;
    int step;
public:
    
    VolumeDownCommand(Editor& ed, int s) : editor(ed), step(s) {}

    void execute() {
        editor.decrease_volume(step);
    }

    void undo() {
        editor.increase_volume(step);
    }

    std::string get_type() const {
        return "VolumeDown";
    }

    json get_params() const {
        return { {"step", step} };
    }

};

class CommandFactory {
public:
    static Command* create_command(const std::string& type, const json& params, Editor& editor) {
        if (type == "PrintChar") {
            char c = params["char"].get<std::string>()[0];
            return new KeyCommand(editor, c);
        }
        else if (type == "VolumeUp") {
            int step = params.value("step", 10);
            return new VolumeUpCommand(editor, step);
        }
        else if (type == "VolumeDown") {
            int step = params.value("step", 10);
            return new VolumeDownCommand(editor, step);
        }
        else if (type == "MediaPlayer") {
            return new MediaPlayerCommand(editor);
        }
        return nullptr;
    }
};


Keyboard::Keyboard(Editor& ed) : editor(ed) {}

Keyboard::~Keyboard() {
    for (auto& pair : key_commands) {
        delete pair.second;
    }
}

void Keyboard::bind_key(const std::string& keyCombo, Command* cmd) {
    if (key_commands.find(keyCombo) != key_commands.end()) {
        delete key_commands[keyCombo];
    }
    key_commands[keyCombo] = cmd;
}

void Keyboard::press_key(const std::string& keyCombo) {
    auto it = key_commands.find(keyCombo);
    if (it != key_commands.end()) {
        Command* cmd = it->second;
        cmd->execute();
        undo_stack.push(cmd);
        while (!redo_stack.empty()) {
            redo_stack.pop();
        }
    }
}

void Keyboard::undo() {
    if (!undo_stack.empty()) {
        Command* cmd = undo_stack.top();
        cmd->undo();
        undo_stack.pop();
        redo_stack.push(cmd);
    }
}

void Keyboard::redo() {
    if (!redo_stack.empty()) {
        Command* cmd = redo_stack.top();
        cmd->execute();
        redo_stack.pop();
        undo_stack.push(cmd);
    }
}

json Keyboard::get_state() const {
    json state;
    for (const auto& pair : key_commands) {
        const Command* cmd = pair.second;
        json cmdJson;
        cmdJson["type"] = cmd->get_type();
        cmdJson["params"] = cmd->get_params();
        state[pair.first] = cmdJson;
    }
    return state;
}

void Keyboard::set_state(const json& state) {
    for (auto& pair : key_commands) {
        delete pair.second;
    }
    key_commands.clear();

    for (auto& el : state.items()) {
        std::string keyCombo = el.key();
        json cmdJson = el.value();
        std::string type = cmdJson["type"];
        json params = cmdJson["params"];
        Command* cmd = CommandFactory::create_command(type, params, editor);
        if (cmd) {
            key_commands[keyCombo] = cmd;
        }
    }
}



int main() {
    Editor editor;
    Keyboard keyboard(editor);
    KeyboardStateSaver saver;

    saver.load(keyboard, "keyboard_state.json");

    if (keyboard.get_state().empty()) {
        keyboard.bind_key("a", new KeyCommand(editor, 'a'));
        keyboard.bind_key("b", new KeyCommand(editor, 'b'));
        keyboard.bind_key("c", new KeyCommand(editor, 'c'));
        keyboard.bind_key("d", new KeyCommand(editor, 'd'));
        keyboard.bind_key("e", new KeyCommand(editor, 'e'));
        keyboard.bind_key("ctrl++", new VolumeUpCommand(editor, 20));
        keyboard.bind_key("ctrl+-", new VolumeDownCommand(editor, 20));
        keyboard.bind_key("ctrl+p", new MediaPlayerCommand(editor));
        keyboard.bind_key("ctrl+alt+p", new MediaPlayerCommand(editor));
    }

    

    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            saver.save(keyboard, "keyboard_state.json");
            break;
        }
        else if (input == "undo") {
            keyboard.undo();
        }
        else if (input == "redo") {
            keyboard.redo();
        }
        else {
            keyboard.press_key(input);
        }
    }

    return 0;
}