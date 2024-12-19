#include "FMesh.h"

#include <fstream>

#include "FObject.h"

using namespace std;

struct FFaceCorner { uint16_t co; uint16_t uv; uint16_t vn; };

// splits a formatted OBJ face corner into its component indices
static inline FFaceCorner splitOBJFaceCorner(string str)
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

static pair<XMFLOAT3, XMFLOAT3> computeTangent(XMFLOAT3 co_a, XMFLOAT3 co_b, XMFLOAT3 co_c, XMFLOAT2 uv_a, XMFLOAT2 uv_b, XMFLOAT2 uv_c)
{
    XMFLOAT3 debug_tool = XMFLOAT3(1.0f, 1.0f, 1.0f);
    XMFLOAT2 other_tool = XMFLOAT2(1.0f, 1.0f);

    // vector from the target vertex to the second vertex
    XMFLOAT3 ab = XMFLOAT3(co_b.x - co_a.x, co_b.y - co_a.y, co_b.z - co_a.z); ab = XMFLOAT3(ab.x * debug_tool.x, ab.y * debug_tool.y, ab.z * debug_tool.z);
    // vector from the target vertex to the third vertex
    XMFLOAT3 ac = XMFLOAT3(co_c.x - co_a.x, co_c.y - co_a.y, co_c.z - co_a.z); ac = XMFLOAT3(ac.x * debug_tool.x, ac.y * debug_tool.y, ac.z * debug_tool.z);
    // delta uv between target and second
    XMFLOAT2 uv_ab = XMFLOAT2(uv_b.x - uv_a.x, uv_b.y - uv_a.y); uv_ab = XMFLOAT2(uv_ab.x * other_tool.x, uv_ab.y * other_tool.y);
    // delta uv between target and third
    XMFLOAT2 uv_ac = XMFLOAT2(uv_c.x - uv_a.x, uv_c.y - uv_a.y); uv_ac = XMFLOAT2(uv_ac.x * other_tool.x, uv_ac.y * other_tool.y);
    // matrix representing UVs
    XMFLOAT3X3 uv_mat = XMFLOAT3X3
    (
        uv_ab.x, uv_ac.x, 0,
        uv_ab.y, uv_ac.y, 0,
        0,       0,       1
    );
    // matrix representing vectors between vertices
    XMFLOAT3X3 vec_mat = XMFLOAT3X3
    (
        ab.x, ac.x, 0,
        ab.y, ac.y, 0,
        ab.z, ac.z, 0
    );
    
    // we should be able to express the vectors from A->B and A->C with reference to the difference in UV coordinate and the tangent and bitangent:
    //
    // AB = (duv_ab.x * T) + (duv_ab.y * B)
    // AC = (duv_ac.x * T) + (duv_ac.y * B)
    // 
    // this gives us 6 simultaneous equations for the XYZ coordinates of the tangent and bitangent
    // these can be expressed and solved with matrices:
    // 
    // [ AB.x  AC.x  0 ]     [ T.x  B.x  N.x ]   [ duv_ab.x  duv_ac.x  0 ]
    // [ AB.y  AC.y  0 ]  =  [ T.y  B.y  N.y ] * [ duv_ab.y  duv_ac.y  0 ]
    // [ AB.z  AC.z  0 ]     [ T.z  B.z  N.z ]   [ 0         0         1 ]
    //

    XMMATRIX result =  XMLoadFloat3x3(&vec_mat) * XMMatrixInverse(nullptr, XMLoadFloat3x3(&uv_mat));

    XMStoreFloat3x3(&vec_mat, result);

    pair<XMFLOAT3, XMFLOAT3> ret;
    ret.first = XMFLOAT3(vec_mat._11, vec_mat._21, vec_mat._31);                 // extract tangent
    XMStoreFloat3(&ret.first, XMVector3Normalize(XMLoadFloat3(&ret.first)));

    return ret;
}

FMesh* FMeshComponent::loadMesh(string path)
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
    XMFLOAT3 tmp3 = XMFLOAT3();
    XMFLOAT2 tmp2 = XMFLOAT2();

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

            swap(tmp_fc[tmp_fc.size() - 1], tmp_fc[tmp_fc.size() - 3]);
        }
        file.ignore(MAXLONGLONG, '\n');
    }

    // for each coordinate, stores a list of all the times it has been used by a face corner, and what the normal/uv index was for that face corner
    // this allows us to tell when we should split a vertex (i.e. if it has already been used by another face corner but which had a different normal and/or a different uv)
    vector<vector<FFaceCornerReference>> fc_normal_uses(tmp_co.size(), vector<FFaceCornerReference>());

    FMesh* mesh_data = new FMesh();

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

    // compute tangents
    vector<bool> touched = vector<bool>(mesh_data->vertices.size(), false);
    for (uint32_t tri = 0; tri < mesh_data->indices.size() / 3; tri++)
    {
        uint16_t v0 = mesh_data->indices[((uint32_t)tri * 3) + 0]; FVertex f0 = mesh_data->vertices[v0];
        uint16_t v1 = mesh_data->indices[((uint32_t)tri * 3) + 1]; FVertex f1 = mesh_data->vertices[v1];
        uint16_t v2 = mesh_data->indices[((uint32_t)tri * 3) + 2]; FVertex f2 = mesh_data->vertices[v2];
        
        if (!touched[v0]) mesh_data->vertices[v0].tangent = computeTangent(f0.position, f1.position, f2.position, f0.uv, f1.uv, f2.uv).first;
        if (!touched[v1]) mesh_data->vertices[v1].tangent = computeTangent(f1.position, f0.position, f2.position, f1.uv, f0.uv, f2.uv).first;
        if (!touched[v2]) mesh_data->vertices[v2].tangent = computeTangent(f2.position, f0.position, f1.position, f2.uv, f0.uv, f1.uv).first;

        touched[v0] = true; touched[v1] = true; touched[v2] = true;
    }

    // transform from Z back Y up space into Z up Y forward space
    for (FVertex& fv : mesh_data->vertices)
    {
        fv.position = XMFLOAT3(fv.position.x, -fv.position.z, fv.position.y);
        fv.normal = XMFLOAT3(fv.normal.x, -fv.normal.z, fv.normal.y);
        fv.tangent = XMFLOAT3(fv.tangent.x, -fv.tangent.z, fv.tangent.y);
    }

    XMFLOAT3 max_c = XMFLOAT3(0,0,0);
    XMFLOAT3 min_c = XMFLOAT3(0,0,0);

    // compute bounding box
    if (mesh_data->vertices.size() > 0)
    {
        max_c = mesh_data->vertices[0].position;
        min_c = mesh_data->vertices[0].position;
    }

    for (FVertex& v : mesh_data->vertices)
    {
        max_c.x = max(v.position.x, max_c.x);
        max_c.y = max(v.position.y, max_c.y);
        max_c.z = max(v.position.z, max_c.z);

        min_c.x = min(v.position.x, min_c.x);
        min_c.y = min(v.position.y, min_c.y);
        min_c.z = min(v.position.z, min_c.z);
    }

    mesh_data->bounds = FBoundingBox{ max_c, min_c };

	return mesh_data;
}

bool FMeshComponent::intersectBoundingBox(const FBoundingBox& bb, XMFLOAT3 ray_origin, XMFLOAT3 ray_direction, float& tmin, float& tmax)
{
    // based closely on https://psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html
    float mins[3] = { bb.min_corner.x, bb.min_corner.y, bb.min_corner.z };
    float maxs[3] = { bb.max_corner.x, bb.max_corner.y, bb.max_corner.z };

    float origin[3] = { ray_origin.x, ray_origin.y, ray_origin.z };
    float inv_direction[3] = { 1.0f / ray_direction.x, 1.0f / ray_direction.y, 1.0f / ray_direction.z };

    tmin = 0;
    tmax = INFINITY;
    bool failed = false;

    for (int i = 0; i < 3; i++)
    {
        float t0 = (mins[i] - origin[i]) * inv_direction[i];
        float t1 = (maxs[i] - origin[i]) * inv_direction[i];
        if (inv_direction[i] < 0)
            swap(t0, t1);
        tmin = max(t0, tmin);
        tmax = min(t1, tmax);

        if (tmax < tmin) { failed = true; break; }
    }

    return !failed;
}

FBoundingBox FMeshComponent::getWorldSpaceBounds()
{
    XMFLOAT3 mi = mesh_data->bounds.min_corner;
    XMFLOAT3 ma = mesh_data->bounds.max_corner;
    XMFLOAT4 nnn = XMFLOAT4(mi.x, mi.y, mi.z, 1);
    XMFLOAT4 ppp = XMFLOAT4(ma.x, ma.y, ma.z, 1);
    vector<XMFLOAT4> corners =
    {
        nnn,
        XMFLOAT4(ppp.x, nnn.y, nnn.z, 1),
        XMFLOAT4(nnn.x, ppp.y, nnn.z, 1),
        XMFLOAT4(ppp.x, ppp.y, nnn.z, 1),
        XMFLOAT4(nnn.x, nnn.y, ppp.z, 1),
        XMFLOAT4(ppp.x, nnn.y, ppp.z, 1),
        XMFLOAT4(nnn.x, ppp.y, ppp.z, 1),
        ppp
    };

    FBoundingBox world_bounds = FBoundingBox{ };
    XMFLOAT4X4 world = owner->transform.getTransform();
    XMMATRIX world_matrix = XMLoadFloat4x4(&world);
    bool is_first = true;
    for (XMFLOAT4 v : corners)
    {
        XMFLOAT3 ws; XMStoreFloat3(&ws, XMVector4Transform(XMLoadFloat4(&v), world_matrix));
        
        if (is_first)
        {
            world_bounds.max_corner = ws;
            world_bounds.min_corner = ws;
        }
        
        world_bounds.max_corner.x = max(ws.x, world_bounds.max_corner.x);
        world_bounds.max_corner.y = max(ws.y, world_bounds.max_corner.y);
        world_bounds.max_corner.z = max(ws.z, world_bounds.max_corner.z);

        world_bounds.min_corner.x = min(ws.x, world_bounds.min_corner.x);
        world_bounds.min_corner.y = min(ws.y, world_bounds.min_corner.y);
        world_bounds.min_corner.z = min(ws.z, world_bounds.min_corner.z);

        is_first = false;
    }

    return world_bounds;
}
