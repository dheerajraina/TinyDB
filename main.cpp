#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

string trim(const std::string &s)
{
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c)
                                    { return std::isspace(c); });
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c)
                                   { return std::isspace(c); })
                      .base();
    return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

string to_upper(string s)
{
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

struct Column
{
    string name;
    string type;
};

struct Table
{
    string name;
    vector<Column> columns;

    Table(const string &name, const vector<Column> &cols)
        : name(name), columns(cols) {}
};

struct Database
{
    string name;
    unordered_map<string, Table> tables;

    Database(const string &name) : name(name) {}

    void create_table(const string &table_name, const vector<Column> &cols)
    {
        if (tables.count(table_name))
        {
            cout << "Error: Table already exists\n";
            return;
        }

        tables.emplace(table_name, Table(table_name, cols));
        cout << "Table '" << table_name << "' created\n";
    }
};

class DatabaseManager
{
private:
    unordered_map<string, Database> databases;
    Database *current_db = nullptr;

public:
    void create_database(const string &name)
    {
        if (databases.count(name))
        {
            cout << "Error: Database already exists\n";
            return;
        }
        databases.emplace(name, Database(name));
        cout << "Database '" << name << "' created\n";
    }

    void use_database(const string& name){
         if (databases.count(name)) {
            cout << "Error: Database already exists\n";
            return;
        }
        databases.emplace(name, Database(name));
        cout << "Database '" << name << "' created\n";
    }
};

int main()
{

    string input, line;
    cout << "Welcome to TinyDB (type 'exit;')\n";

    while (true)
    {

        cout << ">>";
        getline(cin, line);
        input += line;
        cout << "line -> " << trim(input);

        if (input.find(';') == string::npos)
            continue;

        if (to_upper(trim(input)) == "EXIT;")
            break;

        input.clear();
    }

    return 0;
}