#include "FMesh.h"

#include <fstream>

struct FFaceCorner { uint16_t co; uint16_t uv; uint16_t vn; };

// TODO: doxygen-ify all the comments

// splits a formatted OBJ face corner into its component indices
inline FFaceCorner splitOBJFaceCorner(string str)
{
    FFaceCorner fci = { 0,0,0 };
    size_t first_break_ind = str.find('/');
    if (first_break_ind == string::npos) return fci;
    fci.co = static_cast<uint16_t>(stoi(str.substr(0, first_break_ind)) - 1);
    size_t second_break_ind = str.find('/', first_break_ind + 1);
    if (second_break_ind != first_break_ind + 1)
        fci.uv = static_cast<uint16_t>(stoi(str.substr(first_break_ind + 1, second_break_ind - first_break_ind)) - 1);
    fci.vn = static_cast<uint16_t>(stoi(str.substr(second_break_ind + 1, str.find('/', second_break_ind + 1) - second_break_ind)) - 1);

    return fci;
}

FMeshData* FMesh::loadMesh(string path)
{
    ifstream file;
    file.open(path);
    if (!file.is_open())
        return nullptr;

    // vectors to load data into
    vector<XMFLOAT3> tmp_co;
    vector<FFaceCorner> tmp_fc;
    vector<XMFLOAT2> tmp_uv;
    vector<XMFLOAT3> tmp_vn;

    // temporary locations for reading data to
    string tmps;
    XMFLOAT3 tmp3;
    XMFLOAT2 tmp2;

    // repeat for every line in the file
    while (!file.eof())
    {
        file >> tmps;
        if (tmps == "v")
        {
            // read a vertex coordinate
            file >> tmp3.x;
            file >> tmp3.y;
            file >> tmp3.z;
            tmp_co.push_back(tmp3);
        }
        else if (tmps == "vn")
        {
            // read a face corner normal
            file >> tmp3.x;
            file >> tmp3.y;
            file >> tmp3.z;
            tmp_vn.push_back(tmp3);
        }
        else if (tmps == "vt")
        {
            // read a face corner uv (texture coordinate)
            file >> tmp2.x;
            file >> tmp2.y;
            tmp_uv.push_back(tmp2);
        }
        else if (tmps == "f")
        {
            // read a face (only supports triangles)
            file >> tmps;
            tmp_fc.push_back(splitOBJFaceCorner(tmps));
            file >> tmps;
            tmp_fc.push_back(splitOBJFaceCorner(tmps));
            file >> tmps;
            tmp_fc.push_back(splitOBJFaceCorner(tmps));
        }
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // TODO: support UV coordinates later!

    // for each coordinate, stores a list of all the times it has been used by a face corner, and what the normal index was for that face corner
    // this allows us to tell when we should split a vertex (i.e. if it has already been used by another face corner but which had a different normal)
    // the second half of the pair stores the index of the vertex in the real vertex array for the FVertex which has the model space position given by the index and normal given by the first half of the pair
    vector<vector<pair<uint16_t, uint16_t>>> fc_normal_uses(tmp_co.size(), vector<pair<uint16_t, uint16_t>>());

    FMeshData* mesh_data = new FMeshData();

    for (FFaceCorner fc : tmp_fc)
    {
        bool found_matching_vertex = false;
        uint16_t match = 0;
        for (pair<uint16_t, uint16_t> existing : fc_normal_uses[fc.co])
        {
            if (existing.first == fc.vn)
            {
                found_matching_vertex = true;
                match = existing.second;
                break;
            }
        }

        if (found_matching_vertex)
        {
            mesh_data->indices.push_back(match);
        }
        else
        {
            FVertex new_vert;
            new_vert.position = tmp_co[fc.co];
            new_vert.normal = tmp_vn[fc.vn];

            uint16_t new_index = static_cast<uint16_t>(mesh_data->vertices.size());
            fc_normal_uses[fc.co].push_back(pair<uint16_t, uint16_t>(fc.vn, new_index));

            mesh_data->indices.push_back(new_index);
            mesh_data->vertices.push_back(new_vert);
        }
    }

	return mesh_data;
}
