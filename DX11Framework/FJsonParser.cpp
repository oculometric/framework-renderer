#include "FJsonParser.h"

#include <fstream>

bool FJsonBlob::validate(const string& s)
{
    // assume it's good
    return s.length() > 0;
}

size_t FJsonBlob::next(const string& s, const size_t start, const char delim)
{
    int stack_end = -1;
    vector<char> brace_stack;

    size_t pos;
    for (pos = start; pos < s.length(); pos++)
    {
        if (stack_end >= 0 && brace_stack[stack_end] == '\"' && s[pos] != '\"') continue;
        if (s[pos] == '{')
        {
            brace_stack.push_back('{');
            stack_end++;
        }
        else if (s[pos] == '[')
        {
            brace_stack.push_back('[');
            stack_end++;
        }
        else if (s[pos] == '\"')
        {
            if (stack_end >= 0 && brace_stack[stack_end] == '\"')
            {
                brace_stack.pop_back();
                stack_end--;
            }
            else
            {
                brace_stack.push_back('\"');
                stack_end++;
            }
        }
        else if (s[pos] == '}')
        {
            if (stack_end >= 0 && brace_stack[stack_end] == '{')
            {
                brace_stack.pop_back();
                stack_end--;
            }
            else return string::npos;
        }
        else if (s[pos] == ']')
        {
            if (stack_end >= 0 && brace_stack[stack_end] == '[')
            {
                brace_stack.pop_back();
                stack_end--;
            }
            else return string::npos;
        }

        if (stack_end == -1 && s[pos] == delim) break;
    }

    if (pos < s.length()) return pos;

    return string::npos;
}

string FJsonBlob::extract(const string& s, const size_t start, size_t& end)
{
    string r;
    char delim = s[start];
    char target = '\0';
    switch (delim)
    {
    case '{': target = '}'; break;
    case '[': target = ']'; break;
    case '\"': target = '\"'; break;
    default: return "";
    }

    end = next(s, start, target);

    return s.substr(start + 1, (end - start) - 1);
}

FJsonElement FJsonBlob::decode(const string& s)
{
    if (s.length() == 0) throw runtime_error("invalid json element");
    if (s[0] == '\"')
    {
        if (s[s.length() - 1] != '\"' || s.length() < 2) throw runtime_error("invalid json element");

        return FJsonElement(s.substr(1, s.length() - 2));
    }
    else if (s[0] == '{')
    {
        if (s[s.length() - 1] != '}' || s.length() < 2) throw runtime_error("invalid json element");

        size_t tmp;
        string extracted = extract(s, 0, tmp);
        return FJsonElement(parse(extracted));
    }
    else if (s[0] == '[')
    {
        if (s[s.length() - 1] != ']' || s.length() < 2) throw runtime_error("invalid json element");

        size_t tmp;
        size_t section_start = 0;
        size_t section_end = 0;

        vector<FJsonElement> elements;
        string inner = extract(s, 0, tmp);

        do
        {
            section_end = next(inner, section_start, ',');
            string section = inner.substr(section_start, section_end - section_start);
            section_start = section_end + 1;

            elements.push_back(decode(section));
        }
        while (section_start - 1 != string::npos);

        return FJsonElement(elements);
    }
    else
    {
        return FJsonElement(stof(s));
    }
}

FJsonObject* FJsonBlob::parse(const string& s)
{
    FJsonObject* obj = new FJsonObject();
    all_objects.push_back(obj);

    size_t section_start = 0;
    size_t section_end = 0;
    
    do
    {
        section_end = next(s, section_start, ',');
        string section = s.substr(section_start, section_end - section_start);
        section_start = section_end + 1;

        size_t colon = next(section, 0, ':');
        string key = section.substr(0, colon);
        string value = section.substr(colon + 1);
        key = extract(key, 0, colon);
        obj->elements.insert_or_assign(key, decode(value));
    }
    while (section_start - 1 != string::npos);

    return obj;
}

string FJsonBlob::reduce(const string& s)
{
    bool inside_line_comment = false;
    bool inside_block_comment = false;
    bool inside_string = false;

    size_t l = s.length();
    string r;
    r.reserve(s.size());

    for (size_t pos = 0; pos < l; pos++)
    {
        if (inside_string)
        {
            r.push_back(s[pos]);
            if (s[pos] == '\"')
                inside_string = false;
        }
        else
        {
            if (!inside_block_comment && !inside_line_comment)
            {
                if (s[pos] == '/')
                {
                    if (pos < l - 1 && s[pos + 1] == '/') inside_line_comment = true;
                    if (pos < l - 1 && s[pos + 1] == '*') inside_block_comment = true;
                }
                else
                {
                    if (s[pos] == '\n' || s[pos] == '\t' || s[pos] == ' ') continue;
                    r.push_back(s[pos]);
                    if (s[pos] == '\"')
                        inside_string = true;
                }
            }
            else if (inside_line_comment)
            {
                if (s[pos] == '\n') inside_line_comment = false;
                if (pos < l - 1 && s[pos] == '/' && s[pos + 1] == '*') inside_block_comment = true;
                if (pos < l - 1 && s[pos] == '*' && s[pos + 1] == '/') inside_block_comment = false;
            }
            else if (inside_block_comment)
            {
                if (pos < l - 1 && s[pos] == '*' && s[pos + 1] == '/') inside_block_comment = false;
            }
        }
    }

    return r;
}

FJsonBlob::FJsonBlob(string path)
{
    ifstream file;
    file.open(path);
    if (!file.is_open()) return;
    file.seekg(0, ios::end);
    streamsize size = file.tellg();
    string everything((size_t)size, '\0');
    file.seekg(ios::beg);
    file.read((char*)everything.c_str(), size);
    file.close();

    everything = reduce(everything);

    if (!validate(everything)) return;

    size_t end;
    root = FJsonElement(parse(extract(everything, 0, end)));
}

FJsonBlob::~FJsonBlob()
{
    for (FJsonObject* o : all_objects) delete o;
    root = nullptr;
}