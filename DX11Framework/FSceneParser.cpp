#include "FSceneParser.h"

#include <fstream>

bool FJsonBlob::validate(string s)
{
    // assume it's good
    return true;
}

string FJsonBlob::extractBlock(string s, size_t start, char delim)
{
    string r;
    char end = '\0';
    switch (delim)
    {
    case '{': end = '}'; break;
    case '[': end = ']'; break;
    case '\"': end = '\"'; break;
    default: return r;
    }

    int brace_c = 0;
    int brack_c = 0;
    int quote_c = 0;

    for (size_t pos = start; pos < s.length(); pos++)
    {
        if (s[pos] == ..... HERE)
    }
    return r;
}

string FJsonBlob::reduce(string s)
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

    root = parseBlock(extractBlock(everything, 0, '{'));
}
