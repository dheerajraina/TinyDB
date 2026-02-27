#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <variant>

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

vector<string> split(const string &s, char delim)
{
    vector<string> tokens;
    string token;
    stringstream ss(s);
    while (getline(ss, token, delim))
    {
        tokens.push_back(trim(token));
    }
    return tokens;
}

struct Column
{
    string name;
    string type;
};

using Value = variant<int, string>;
using Row = vector<Value>;

struct Table
{
    string name;
    vector<Column> columns;
    vector<Row> rows;

    Table(const string &name, const vector<Column> &cols)
        : name(name), columns(cols) {}

    bool insert_row(const vector<string> &values)
    {
        if (values.size() != columns.size())
        {
            cout << "Error: Column count mismatch\n";
            return false;
        }

        Row row;
        for (size_t i = 0; i < values.size(); i++)
        {
            const string &type = columns[i].type;
            const string &val = values[i];

            if (type == "INT")
            {
                try
                {
                    row.push_back(stoi(val));
                }
                catch (...)
                {
                    cout << "Type error:expected INT\n";
                    return false;
                }
            }
            else if (type == "TEXT")
            {
                cout<<"value text: "<<val<<"\n";
                if (val.front() != '"' || val.back() != '"')
                {
                    cout << "Type error: TEXT must be quoted\n";
                    return false;
                }
                row.push_back(val.substr(1, val.size() - 2));
                cout << "inserted text value : " << val.substr(1, val.size() - 2) << "\n";
            }
        }
        rows.push_back(row);
        cout << "1 row inserted\n";
        return true;
    }
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

    void use_database(const string &name)
    {
        if (!databases.count(name))
        {
            cout << "Error: Database not found\n";
            return;
        }
        current_db = &databases.at(name);
        cout << "Using database '" << name << "'\n";
    }

    Database *get_current_db()
    {
        return current_db;
    }
};

class SQLEngine
{
private:
    DatabaseManager dbm;

public:
    void execute(string query)
    {
        query = trim(query);
        if (query.back() == ';')
            query.pop_back();

        string upper = to_upper(query);

        if (upper.find("CREATE DATABASE") == 0)
        {
            cout << "create db command\n";
            handle_create_database(query);
        }
        else if (upper.find("USE") == 0)
        {
            cout << "use command\n";
            handle_use(query);
        }
        else if (upper.find("CREATE TABLE") == 0)
        {
            cout << "create tb command\n";
            handle_create_table(query);
        }
        else if (upper.find("INSERT INTO") == 0)
        {
            cout << "insert into command\n";
            insert_into(query);
        }
        else
        {
            cout << "Error: Unknown command\n";
        }
    }

private:
    void handle_create_database(const string &q)
    {
        auto parts = split(q, ' ');
        if (parts.size() != 3)
        {
            cout << "Syntax error in CREATE DATABASE\n";
            return;
        }
        dbm.create_database(parts[2]);
    }

    void handle_use(const string &q)
    {
        auto parts = split(q, ' ');
        if (parts.size() != 2)
        {
            cout << "Syntax error in USE\n";
            return;
        }
        dbm.use_database(parts[1]);
    }

    void handle_create_table(const string &q)
    {
        Database *db = dbm.get_current_db();
        if (!db)
        {
            cout << "Error: No database selected\n";
            return;
        }

        size_t start = q.find('(');
        size_t end = q.find(')');

        if (start == string::npos || end == string::npos || end <= start)
        {
            cout << "Syntax error in CREATE TABLE\n";
            return;
        }

        string header = trim(q.substr(0, start));
        auto header_parts = split(header, ' ');
        if (header_parts.size() != 3)
        {
            cout << "Syntax error in CREATE TABLE\n";
            return;
        }

        string table_name = header_parts[2];
        string cols_str = q.substr(start + 1, end - start - 1);

        vector<string> col_defs = split(cols_str, ',');
        vector<Column> columns;

        for (const string &def : col_defs)
        {
            auto parts = split(def, ' ');
            if (parts.size() != 2)
            {
                cout << "Invalid column definition: " << def << "\n";
                return;
            }

            string col_name = parts[0];
            string col_type = to_upper(parts[1]);

            if (col_type != "INT" && col_type != "TEXT")
            {
                cout << "Unsupported type: " << col_type << "\n";
                return;
            }

            columns.push_back({col_name, col_type});
        }

        db->create_table(table_name, columns);
    }

    void insert_into(const string &q)
    {
        Database *db = dbm.get_current_db();
        if (!db)
        {
            cout << "Error: No database selected\n";
            return;
        }

        size_t into = to_upper(q).find("INTO");
        size_t values = to_upper(q).find("VALUES");

        if (into == string::npos || values == string::npos)
        {
            cout << "Syntax error in INSERT\n";
            return;
        }

        string table_name = trim(q.substr(into + 4, values - (into + 4)));

        size_t l = q.find('(', values);
        size_t r = q.find(')', values);

    

        if (l == string::npos || r == string::npos || r <= l)
        {
            cout << "Syntax error in INSERT\n";
            return;
        }

        cout<<q.substr(l + 1, r - 1)<<"\n";

        vector<string> vals = split(q.substr(l + 1, r - l - 1), ',');

        if (!db->tables.count(table_name))
        {
            cout << "Table not found\n";
            return;
        }
        db->tables.at(table_name).insert_row(vals);
    }
};

int main()
{

    SQLEngine engine;
    string input, line;
    cout << "Welcome to TinyDB (type 'exit;')\n";

    while (true)
    {

        // cout << ">>";
        getline(cin, line);
        input += line;
        // cout << "line -> " << trim(input);

        if (input.find(';') == string::npos)
            continue;

        if (to_upper(trim(input)) == "EXIT;")
            break;

        engine.execute(input);
        input.clear();
    }

    return 0;
}