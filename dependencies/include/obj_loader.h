#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Loads a Wavefront .obj file into an interleaved vertex array:
//   x, y, z,  u, v,  nx, ny, nz   (8 floats per vertex)
// which matches the layout used by the VAO in main.cpp.
// Faces with more than 3 corners (quads, n-gons) are split into triangles
// using a fan: (0,1,2), (0,2,3), (0,3,4), ...
// Returns an empty vector if the file can't be opened.
static std::vector<float> loadOBJ(const std::string &path)
{
    std::vector<float> positions; // 3 per entry
    std::vector<float> uvs;       // 2 per entry
    std::vector<float> normals;   // 3 per entry
    std::vector<float> out;       // interleaved result

    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Failed to open OBJ: " << path << std::endl;
        return out;
    }

    // Appends one face corner like "12/5/3", "12//3", "12/5" or "12"
    auto pushCorner = [&](const std::string &corner)
    {
        int vi = 0, ti = 0, ni = 0;
        // Split on '/' (empty fields stay 0)
        size_t s1 = corner.find('/');
        vi = std::stoi(corner.substr(0, s1));
        if (s1 != std::string::npos)
        {
            size_t s2 = corner.find('/', s1 + 1);
            std::string t = corner.substr(s1 + 1, s2 == std::string::npos ? std::string::npos : s2 - s1 - 1);
            if (!t.empty()) ti = std::stoi(t);
            if (s2 != std::string::npos)
            {
                std::string n = corner.substr(s2 + 1);
                if (!n.empty()) ni = std::stoi(n);
            }
        }

        // OBJ indices are 1-based; negative indices count back from the end
        if (vi < 0) vi = (int)positions.size() / 3 + vi + 1;
        if (ti < 0) ti = (int)uvs.size() / 2 + ti + 1;
        if (ni < 0) ni = (int)normals.size() / 3 + ni + 1;

        // position
        out.push_back(positions[(vi - 1) * 3 + 0]);
        out.push_back(positions[(vi - 1) * 3 + 1]);
        out.push_back(positions[(vi - 1) * 3 + 2]);
        // uv (0,0 if missing)
        out.push_back(ti > 0 ? uvs[(ti - 1) * 2 + 0] : 0.0f);
        out.push_back(ti > 0 ? uvs[(ti - 1) * 2 + 1] : 0.0f);
        // normal (0,0,1 if missing)
        out.push_back(ni > 0 ? normals[(ni - 1) * 3 + 0] : 0.0f);
        out.push_back(ni > 0 ? normals[(ni - 1) * 3 + 1] : 0.0f);
        out.push_back(ni > 0 ? normals[(ni - 1) * 3 + 2] : 1.0f);
    };

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v")
        {
            float x, y, z;
            ss >> x >> y >> z;
            positions.insert(positions.end(), {x, y, z});
        }
        else if (type == "vt")
        {
            float u, v;
            ss >> u >> v;
            uvs.insert(uvs.end(), {u, v});
        }
        else if (type == "vn")
        {
            float x, y, z;
            ss >> x >> y >> z;
            normals.insert(normals.end(), {x, y, z});
        }
        else if (type == "f")
        {
            std::vector<std::string> corners;
            std::string c;
            while (ss >> c) corners.push_back(c);

            // Fan triangulation
            for (size_t i = 1; i + 1 < corners.size(); i++)
            {
                pushCorner(corners[0]);
                pushCorner(corners[i]);
                pushCorner(corners[i + 1]);
            }
        }
        // Everything else (o, s, mtllib, usemtl, comments) is ignored
    }

    std::cout << "Loaded " << path << ": " << out.size() / 8 << " vertices ("
              << out.size() / 24 << " triangles)" << std::endl;
    return out;
}

#endif
