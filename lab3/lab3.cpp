#include <regex>
#include <iostream>
#include <fstream>


class ILogFilter {
public: 
    virtual bool match(const std::string& text) const = 0;
    virtual ~ILogFilter() = default;
};


class SimpleLogFilter: public ILogFilter {
    std::string pattern;
public:
    SimpleLogFilter(const std::string& pattern): pattern(pattern) {}

    bool match(const std::string& text) const override {
        return text.find(pattern) != std::string::npos;
    }
};


class ReLogFilter: public ILogFilter {
    std::regex pattern;
public:
    ReLogFilter(const std::regex& pattern): pattern(pattern) {}

    bool match(const std::string& text) const override {
        return std::regex_search(text, pattern);
    }
};


class ILogHandler {
public: 
    virtual void handle(const std::string& text) const = 0;
    virtual ~ILogHandler() = default;
};


class ConsoleHandler : public ILogHandler {
public:
    void handle(const std::string& text) const override {
        std::cout << "ConsoleHandler: " << text << std::endl;
    }
};


class Socket {
public:
    void send(const std::string& str) {
        std::cout << str << std::endl;
    }
};


class SocketHandler : public ILogHandler {
public:
    void handle(const std::string& text) const override {
        Socket socket;
        socket.send("SocketHandler: " + text);
    }
};


class FileHandler : public ILogHandler {
    std::string path;
public:
    FileHandler(std::string path): path(path) {}

    void handle(const std::string& text) const override {
        std::ofstream fout;
        fout.open(path, std::ios_base::app);
        
        fout << "FileHandler: " << text << std::endl;
        fout.close();
    }
};

class SyslogHandler : public ILogHandler {
public:
    void handle(const std::string& text) const override {
        std::ofstream syslog;
        syslog.open("/var/log/syslog");

        syslog << "SyslogHandler: " << text << std::endl;
    }
};



class Logger {
    std::vector<ILogFilter*> filters;
    std::vector<ILogHandler*> handlers;

public:
    Logger(std::vector<ILogFilter*> filters, std::vector<ILogHandler*> handlers): filters(filters), handlers(handlers) { }

    void log(const std::string& text) {

        for (ILogFilter* filter : filters) {
            if (filter->match(text)) {
                for (ILogHandler* handler : handlers) {
                    handler->handle(text);
                }
                return;
            }
        }


    }
};

int main() {
    SimpleLogFilter error_filter("error");
    ReLogFilter email_filter(std::regex("[0-9]+"));



    ConsoleHandler console_handler;
    FileHandler file_handler("log.txt");
    SocketHandler socket_handler;
    SyslogHandler syslog_handler;

    Logger logger(
        {&error_filter, &email_filter}, {&console_handler, &file_handler, &socket_handler, &syslog_handler}
    );

    logger.log("A critical error");

    logger.log("error 404");
    logger.log("User test@test.com sent message");
    logger.log("User 1 created an error");
    logger.log("Warning 234 message");
}