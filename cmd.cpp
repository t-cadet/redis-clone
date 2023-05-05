#pragma once

#include "store.cpp"
using store::Store;
#include "utils.cpp"

#include <cassert>
#include <iostream>
using std::ostream;
using std::istream;
using std::stringstream;
#include <memory>
using std::unique_ptr;
#include <optional>
using std::optional;
#include <sstream>

#include <string>
using std::string;

// Header
namespace cmd {
    static const string OK = "ok";
    static const string KO = "ko";

    // Result
    class Result {
        public:
            virtual ostream& write(ostream& os) const = 0;
            virtual ~Result() = default;
    };
    ostream& operator<<(ostream& os, Result* r) {
        return r->write(os);
    }

    // ErrorResult
    class ErrorResult: public Result {
    public:
        string error;
        ErrorResult(string error_): error(error_) {}
        ostream& write(ostream& os) const override {
            os << error;
            return os;
        }
        ~ErrorResult() override = default;
    };    
    ostream& operator<<(ostream& os, ErrorResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, ErrorResult& r) {
        char c = is.get();
        while (!is.eof()) {
            r.error.push_back(c);
            c = is.get();
        }
        return is;
    }

    // PingResult
    class PingResult: public Result {
        public:
            string pong;
            PingResult(string pong_): pong(pong_) {}
            ostream& write(ostream& os) const override {
                os << pong;
                return os;
            }
            ~PingResult() override = default;
    };
    ostream& operator<<(ostream& os, PingResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, PingResult& r) {
        for (int i = 0; i < 4; i++) {
            r.pong.push_back(is.get());
        }
        return is;
    }

    // GetResult
    class GetResult: public Result {
        public:
            optional<string> value;
            GetResult(optional<string> value_): value(value_) {}
            ostream& write(ostream& os) const override {
                os << value.value_or("0"); // FIXME: better return format?
                return os;
            }
            ~GetResult() override = default;
    };
    ostream& operator<<(ostream& os, GetResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, GetResult& r) {
        // FIXME
        string buf = "";
        while(!is.eof()) {
            buf.push_back(is.get());
        }
        r.value = buf == KO ? optional<string>() : optional(buf);
        return is;
    }

    // SetResult
    class SetResult: public Result {
        public:
            SetResult() {}
            ostream& write(ostream& os) const override {
                os << OK;
                return os;
            }
            ~SetResult() override = default;
    };
    ostream& operator<<(ostream& os, SetResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, SetResult& r) {
        // FIXME
        is.get();
        is.get();
        return is;
    }

    // DelResult
    class DelResult: public Result {
        public:
            bool key_was_in_store;
            DelResult(bool key_was_in_store_): key_was_in_store(key_was_in_store_) {}
            ostream& write(ostream& os) const override {
                os << key_was_in_store;
                return os;
            }
            ~DelResult() override = default;
    };
    ostream& operator<<(ostream& os, DelResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, DelResult& r) {
        is >> r.key_was_in_store;
        return is;
    }

    // SaveResult
    class SaveResult: public Result {
        public:
            size_t count;
            SaveResult(size_t count_): count(count_) {}
            ostream& write(ostream& os) const override {
                os << count;
                return os;
            }
            ~SaveResult() override = default;
    };
    ostream& operator<<(ostream& os, SaveResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, SaveResult& r) {
        is >> r.count;
        return is;
    }

    // LoadResult
    class LoadResult: public Result {
        public:
            size_t count;
            LoadResult(size_t count_): count(count_) {}
            ostream& write(ostream& os) const override {
                os << count;
                return os;
            }
            ~LoadResult() override = default;
    };
    ostream& operator<<(ostream& os, LoadResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, LoadResult& r) {
        is >> r.count;
        return is;
    }

    // SizeResult
    class SizeResult: public Result {
        public:
            size_t sz;
            SizeResult(size_t sz_): sz(sz_) {}
            ostream& write(ostream& os) const override {
                os << sz;
                return os;
            }
            ~SizeResult() override = default;
    };
    ostream& operator<<(ostream& os, SizeResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, SizeResult& r) {
        is >> r.sz;
        return is;
    }

    // ClearResult
    class ClearResult: public Result {
        public:
            size_t count;
            ClearResult(size_t count_): count(count_) {}
            ostream& write(ostream& os) const override {
                os << count;
                return os;
            }
            ~ClearResult() override = default;
    };
    ostream& operator<<(ostream& os, ClearResult& r) {
        return r.write(os);
    }
    istream& operator>>(istream& is, ClearResult& r) {
        is >> r.count;
        return is;
    }

    // Cmd
    class Cmd {
        public:
            virtual Result* run(Store& s) = 0;
            virtual ~Cmd() = default;
    };

    // Ping
    class Ping: public Cmd {
        public:
            Result* run(Store& s) override {
                return new PingResult{s.ping()};
            }
            ~Ping() override = default;
    };

    class Get: public Cmd {
        public:
            string key;
            Get(string key_): key(key_) {}
            Result* run(Store& s) override {
                return new GetResult(s.get(key));
            }
            ~Get() override = default;
    };

    class Set: public Cmd {
        public:
            string key;
            string value;
            Set(string key_, string value_): key(key_), value(value_) {}
            Result* run(Store& s) override {
                s.set(key, value);
                return new SetResult();
            }
            ~Set() override = default;
    };

    class Del: public Cmd {
        public:
            string key;
            Del(string key_): key(key_) {}
            Result* run(Store& s) override {
                return new DelResult(s.del(key));
            }
            ~Del() override = default;
    };

    class Save: public Cmd {
        public:
            string path;
            Save(string path_): path(path_) {}
            Result* run(Store& s) override {
                return new SaveResult(s.save(path));
            }
            ~Save() override = default;
    };

    class Load: public Cmd {
        public:
            string path;
            Load(string path_): path(path_) {}
            Result* run(Store& s) override {
                return new LoadResult(s.load(path));
            }
            ~Load() override = default;
    };

    class Size: public Cmd {
        public:
            Size() {}
            Result* run(Store& s) override {
                return new SizeResult(s.size());
            }
            ~Size() override = default;
    };

    class Clear: public Cmd {
        public:
            Clear() {}
            Result* run(Store& s) override {
                return new ClearResult(s.clear());
            }
            ~Clear() override = default;
    };

    // Error
    class Error: public Cmd {
        public:
            string error;

            Error(string error_): error(error_) {}
            Result* run(Store&) override {
                return new ErrorResult{error};
            }
            ~Error() override = default;
    };



    // Cmd
    istream& operator>>(istream& is, Cmd*& cmd) {


        // parse command name
        string cmd_name {};
        utils::read_until(is, cmd_name, ' ');

        // FIXME: handle parsing errors
        if (cmd_name == "ping") {
            cmd = new Ping;
        } else if (cmd_name == "get") {
            string key = "";
            utils::read_until(is, key, ' ');
            cmd = new Get(key);
        } else if (cmd_name == "set") {
            string key = "";
            utils::read_until(is, key, ' ');

            string value = "";
            utils::read_until(is, value, ' ');

            cmd = new Set(key, value);
        } else if (cmd_name == "del") {
            string key = "";
            utils::read_until(is, key, ' ');
            cmd = new Del(key);
        } else if (cmd_name == "save") {
            string path = "";
            utils::read_until(is, path, ' ');
            cmd = new Save(path);
        } else if (cmd_name == "load") {
            string path = "";
            utils::read_until(is, path, ' ');
            cmd = new Load(path);
        } else if (cmd_name == "size") {
            cmd = new Size();
        } else if (cmd_name == "clear") {
            cmd = new Clear();
        } else {
            cmd = new Error{"unknown command: " + cmd_name};
        }
        return is;
    }

    void tests() {
        std::cerr << "running cmd tests . . .";

        {
            Store s {};
            stringstream cmd1("ping");
            stringstream out1;

            // parse cmd
            Cmd* cmd;
            cmd1 >> cmd;

            // run cmd & write result
            out1 << cmd->run(s);
            utils::assert_eq(out1.str(), string("pong"));

            // parse result
            PingResult res{""};
            out1 >> res;
            utils::assert_eq(res.pong, string("pong"));
        }
        {
            Store s {};
            stringstream cmd1("pinga");
            stringstream out1;

            // parse cmd
            Cmd* cmd;
            cmd1 >> cmd;

            // run cmd & write result
            out1 << cmd->run(s);
            utils::assert_eq(out1.str(), string("unknown command: pinga"));

            // parse result
            ErrorResult res{""};
            out1 >> res;
            utils::assert_eq(res.error, string("unknown command: pinga"));
        }

        std::cerr << " ok\n";
    }
}

// int main() {
//     cmd::tests();
// }
