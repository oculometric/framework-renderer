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

struct FFaceCornerReference
{
    uint16_t normal_index;
    uint16_t uv_index;
    uint16_t transferred_vert_index;
};

pair<XMFLOAT3, XMFLOAT3> computeTangent(XMFLOAT3 co_a, XMFLOAT3 co_b, XMFLOAT3 co_c, XMFLOAT2 uv_a, XMFLOAT2 uv_b, XMFLOAT2 uv_c, XMFLOAT3 vn_a)
{
    XMFLOAT3 ab = XMFLOAT3(co_b.x - co_a.x, co_b.y - co_a.y, co_b.z - co_a.z);
    XMFLOAT3 ac = XMFLOAT3(co_c.x - co_a.x, co_c.y - co_a.y, co_c.z - co_a.z);
    XMFLOAT2 uv_ab = XMFLOAT2(uv_b.x - uv_a.x, uv_b.y - uv_a.y);
    XMFLOAT2 uv_ac = XMFLOAT2(uv_c.x - uv_a.x, uv_c.y - uv_a.y);
    XMFLOAT3X3 uv_mat = XMFLOAT3X3
    (
        uv_ab.x, uv_ac.x, 0,
        uv_ab.y, uv_ac.y, 0,
        0,       0,        1
    );
    XMFLOAT3X3 vec_mat = XMFLOAT3X3
    (
        ab.x, ac.x, vn_a.x,
        ab.y, ac.y, vn_a.y,
        ab.z, ac.z, vn_a.z
    );
    
    // FIXME: this may or may not need to be transposed? also just fix this in general
    XMMATRIX result = XMLoadFloat3x3(&vec_mat) * XMMatrixInverse(nullptr, XMLoadFloat3x3(&uv_mat));

    //XMVECTOR tangent = XMLoadFloat3(&vn_a); // temporarily normal
    //ab = XMVector3Normalize(XMVector3Cross(tangent, ab)); // bitangent
    //tangent = XMVector3Normalize(XMVector3Cross(ab, tangent)); // now tangent
    XMStoreFloat3x3(&vec_mat, result);


    pair<XMFLOAT3, XMFLOAT3> ret;
    ret.first = XMFLOAT3(vec_mat._11, vec_mat._21, vec_mat._31);
    XMStoreFloat3(&ret.first, XMVector3Normalize(XMLoadFloat3(&ret.first)));
    //XMStoreFloat3(&result., tangent);
    //XMStoreFloat3(&result.second, ab);

    return ret;
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


    // for each coordinate, stores a list of all the times it has been used by a face corner, and what the normal/uv index was for that face corner
    // this allows us to tell when we should split a vertex (i.e. if it has already been used by another face corner but which had a different normal and/or a different uv)
    vector<vector<FFaceCornerReference>> fc_normal_uses(tmp_co.size(), vector<FFaceCornerReference>());

    FMeshData* mesh_data = new FMeshData();

    for (FFaceCorner fc : tmp_fc)
    {
        bool found_matching_vertex = false;
        uint16_t match = 0;
        for (FFaceCornerReference existing : fc_normal_uses[fc.co])
        {
            if (existing.normal_index == fc.vn && existing.uv_index == fc.uv)
            {
                found_matching_vertex = true;
                match = existing.transferred_vert_index;
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
            if (tmp_uv.size() > fc.uv)
                new_vert.uv = tmp_uv[fc.uv];

            uint16_t new_index = static_cast<uint16_t>(mesh_data->vertices.size());
            fc_normal_uses[fc.co].push_back(FFaceCornerReference{ fc.vn, fc.uv, new_index });

            mesh_data->indices.push_back(new_index);
            mesh_data->vertices.push_back(new_vert);
        }
    }

    vector<bool> touched = vector<bool>(mesh_data->vertices.size(), false);

    for (uint16_t tri = 0; tri < mesh_data->indices.size() / 3; tri++)
    {
        uint16_t v0 = mesh_data->indices[(tri * 3) + 0]; FVertex f0 = mesh_data->vertices[v0];
        uint16_t v1 = mesh_data->indices[(tri * 3) + 1]; FVertex f1 = mesh_data->vertices[v1];
        uint16_t v2 = mesh_data->indices[(tri * 3) + 2]; FVertex f2 = mesh_data->vertices[v2];
        
        if (!touched[v0]) mesh_data->vertices[v0].tangent = computeTangent(f0.position, f1.position, f2.position, f0.uv, f1.uv, f2.uv, f1.normal).first;
        if (!touched[v1]) mesh_data->vertices[v1].tangent = computeTangent(f1.position, f2.position, f0.position, f1.uv, f2.uv, f0.uv, f1.normal).first;
        if (!touched[v2]) mesh_data->vertices[v2].tangent = computeTangent(f2.position, f0.position, f1.position, f2.uv, f0.uv, f1.uv, f2.normal).first;

        touched[v0] = true; touched[v1] = true; touched[v2] = true;
    }

	return mesh_data;
}