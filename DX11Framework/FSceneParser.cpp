#include "FSceneParser.h"

#include <fstream>

bool FJsonBlob::validate(string s)
{
    // assume it's good
    return true;
}

string FJsonBlob::reduce(string s)
{
    bool inside_line_comment = false;
    bool inside_block_comment = false;
    bool inside_string = false;

    string r;
    r.reserve(s.size());

    for (size_t pos = 0; pos < s.length(); pos++)
    {
        if (inside_string)
        {
            r.push_back(s[pos]);
            if (s[pos] == '\"')
            {
                inside_string = false;
            }
        }
        else
        {
            if (!inside_block_comment && !inside_line_comment)
            {
                r.push_back(s[pos]);
                // TODO: here...
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
    file.seekg(ios::end);
    streamsize size = file.tellg();
    string everything((size_t)size, '\0');
    file.seekg(ios::beg);
    file.read((char*)everything.c_str(), size);
    file.close();

    everything = reduce(everything);

    if (!validate(everything)) return;

    root = parseBlock(extractBlock(everything, 0, '{'));
}
