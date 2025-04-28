#include "parser.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "event.h"
#include "handles.h"
#include "uids.h"
#include "has.h"
#include "eq.h"

using namespace conditions;

vector<Condition*> Parser::parse(const json &expr, int start, int end)
{
    if(end == -1) end = expr.size();
    vector<Condition*> result;
    for(int i = start; i < end; i++)
        result.push_back(Parser::parse(expr[i]));
    return result;
}

Condition * Parser::parse(const json &expr)
{
    if(!expr.is_array() || expr.size() < 1 || !expr[0].is_string())
        throw std::runtime_error("invalid condition");

    string type = expr[0].as<string>();

    if(type == "and")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"and\" requires one or more arguments");
        return new And(Parser::parse(expr, 1));
    }
    else if(type == "or")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"or\" requires one or more arguments");
        return new Or(Parser::parse(expr, 1));
    }
    else if(type == "not")
    {
        if(expr.size() != 2)
            throw std::runtime_error("\"not\" requires exactly one argument");
        return new Not(Parser::parse(expr[1]));
    }
    else if(type == "event")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"event\" requires exactly one argument");
        if(!expr[1].is_string())
            throw std::runtime_error("\"event\" argument must be a string");
        string eventType = expr[1].as<string>();
        return new Event(eventType);
    }
    else if(type == "handles")
    {
        if(expr.size() != 2)
            throw std::runtime_error("\"handles\" requires exactly one argument");
        const auto &arg = expr[1];
        if(!arg.is_array())
            throw std::runtime_error("\"handles\" argument must be an array");
        vector<int> handles;
        for(int i = 0; i < arg.size(); i++)
        {
            if(!arg[i].is_int64())
                throw std::runtime_error("\"handles\" argument items must be int");
            int handle = arg[i].as<int64_t>();
            handles.push_back(handle);
        }
        return new Handles(handles);
    }
    else if(type == "uids")
    {
        if(expr.size() != 2)
            throw std::runtime_error("\"uids\" requires exactly one argument");
        const auto &arg = expr[1];
        if(!arg.is_array())
            throw std::runtime_error("\"uids\" argument must be an array");
        vector<long long> uids;
        for(int i = 0; i < arg.size(); i++)
        {
            if(!arg[i].is_int64())
                throw std::runtime_error("\"uids\" argument items must be int");
            long long uid = arg[i].as<int64_t>();
            uids.push_back(uid);
        }
        return new UIDs(uids);
    }
    else if(type == "has")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"has\" requires exactly one argument");
        if(!expr[1].is_string())
            throw std::runtime_error("\"has\" argument must be a string");
        string fieldName = expr[1].as<string>();
        return new Has(fieldName);
    }
    else if(type == "eq")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"eq\" requires exactly two arguments");
        if(!expr[1].is_string())
            throw std::runtime_error("\"eq\" argument 1 must be a string");
        string fieldName = expr[1].as<string>();
        json fieldValue = expr[2];
        return new Eq(fieldName, fieldValue);
    }
    else
        throw std::runtime_error("invalid condition type: \"" + type + "\"");
}
