#pragma once

#include <filesystem>
#include <fstream>

#include "Engine.hpp"

namespace persisitance {
template <typename T>
void safeStreamRead(std::istream& is, T& value);
}

namespace physenv {
namespace details {
template <typename T>
std::istream& operator>>(std::istream& is, ::physenv::details::Vector2<T>& v) {
    persisitance::safeStreamRead(is, v.x);
    persisitance::safeStreamRead(is, v.y);
    return is;
}
} // namespace details

inline std::istream& operator>>(std::istream& is, Point& p) {
    persisitance::safeStreamRead(is, p.fixed);
    persisitance::safeStreamRead(is, p.pos);
    persisitance::safeStreamRead(is, p.vel);
    persisitance::safeStreamRead(is, p.mass);
    if (is.good()) {
        throw std::runtime_error("To many columns for a point - file invalid");
    }
    return is;
}

inline std::istream& operator>>(std::istream& is, physenv::Polygon& p) {
    std::vector<physenv::Vec2> verts{{}, {}, {}};
    persisitance::safeStreamRead(is, verts[0]);
    persisitance::safeStreamRead(is, verts[1]);
    persisitance::safeStreamRead(is, verts[2]);
    while (is.good()) {
        verts.push_back(physenv::Vec2{});
        persisitance::safeStreamRead(is, verts.back());
    }
    p = physenv::Polygon(verts);
    if (p.isConvex() == false)
        throw std::runtime_error("Polygon vertices do not form a convex polygon");
    return is;
}
} // namespace physenv

namespace persisitance {

static const std::string PointHeaders{"point-id fixed posx posy velx vely mass color(rgba)"};
static const std::string SpringHeaders =
    "spring-id spring-const natural-length damping-factor point1 point2";
static const std::string PolyHeaders = "polygon-verts: x y ...";

struct ObjectEnabled {
    bool points;
    bool springs;
    bool polygons;
};

template <typename T>
inline void safeStreamRead(std::istream& is, T& value) {
    if (!is.good()) throw std::runtime_error("Not enough columns - read failed");
    is >> value;
}

// TODO will have to be moved outside as graphs will need to access temp references
inline void loadEng(physenv::Engine& eng, std::filesystem::path path, bool replace,
                    ObjectEnabled enabled) {
    path.make_preferred();
    if (replace) {
        eng.points.clear();
        eng.springs.clear();
        eng.polys.clear();
    }
    //  pointOffset = static_cast<PointId>(points.size());

    std::ifstream file{path, std::ios_base::in};
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file \"" + path.string() + '"');
    }

    std::string line;
    // points
    std::getline(file, line);
    if (line != PointHeaders)
        throw std::runtime_error("Point headers invalid: \n is - " + line + "\n should be - " +
                                 PointHeaders);

    std::unordered_map<std::size_t, physenv::PointRef> tempPointIds{};
    std::stringstream                                  ss;
    std::size_t                                        index = 0;
    while (true) {
        std::getline(file, line);
        if (SpringHeaders == line) break;
        if (enabled.points) {
            ss = std::stringstream(line);
            std::size_t temp;
            safeStreamRead(ss, temp);
            if (temp != index) throw std::runtime_error("Non continous point indicie - " + line);
            // read point
            tempPointIds.insert(std::make_pair(temp, eng.addPoint(physenv::Point{})));
            safeStreamRead(ss, eng.points[tempPointIds.at(temp)]);
            ++index;
        }
    }

    index = 0;
    while (true) {
        std::getline(file, line);
        if (PolyHeaders == line) break;
        if (enabled.springs) {
            ss = std::stringstream(line);
            std::size_t temp;
            safeStreamRead(ss, temp);
            if (temp != index) throw std::runtime_error("Non continous spring indicie - " + line);
            // read spring
            double      springConst;
            double      dampFact;
            double      naturalLength;
            std::size_t tempIdP1;
            std::size_t tempIdP2;
            safeStreamRead(ss, springConst);
            safeStreamRead(ss, dampFact);
            safeStreamRead(ss, naturalLength);
            safeStreamRead(ss, tempIdP1);
            safeStreamRead(ss, tempIdP2);
            eng.addSpring(physenv::Spring{springConst, dampFact, naturalLength,
                                          tempPointIds.at(tempIdP1), tempPointIds.at(tempIdP2)});
            ++index;
        }
    }

    while (!file.eof()) {
        std::getline(file, line);
        if (enabled.polygons) {
            ss = std::stringstream(line);
            if (ss.good()) { // deal with emtpy new lines at end
                physenv::Polygon poly{};
                safeStreamRead(ss, poly);
                eng.polys.insert(poly);
            }
        }
    }
}

inline void saveEng(const physenv::Engine& eng, std::filesystem::path path, ObjectEnabled enabled) {
    path.make_preferred();
    std::ofstream file{path};
    if (!file.is_open()) {
        throw std::runtime_error("Falied to open fstream \n");
    }
    std::cout << "Saving to " << path << std::endl;

    file << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
    file << PointHeaders << "\n";
    std::unordered_map<physenv::PointRef, std::size_t> tempPointIds;
    if (enabled.points) {
        std::size_t i = 0;
        for (const auto& p: eng.points) {
            file << i << ' ' << p.obj << "\n";
            tempPointIds[p.ind] = i;
            tempPointIds.insert(std::make_pair(p.ind, i));
            ++i;
        }
    }
    file << SpringHeaders << "\n";
    if (enabled.springs) {
        std::size_t i = 0;
        for (const auto& s: eng.springs) {
            file << i << ' ' << s.obj << ' ' << tempPointIds[s.obj.p1] << ' '
                 << tempPointIds[s.obj.p2] << "\n";
            ++i;
        }
    }
    file << PolyHeaders;
    if (enabled.polygons) {
        for (const auto& p: eng.polys) {
            if (!p.obj.edges.empty()) file << "\n" << p.obj;
        }
    }
}

} // namespace persisitance