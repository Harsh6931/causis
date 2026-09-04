#pragma once

#include "runtime/types.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace causis::runtime {

struct Robot {
    std::string name;
    int x{0};
    int y{0};
    Direction direction{Direction::Right};
    bool collision_flag{false};
};

struct Target {
    std::string name;
    int x{0};
    int y{0};
};

struct Cell {
    CellKind kind{CellKind::Empty};
    std::string label;
};

class World {
public:
    World(int width, int height);

    int width() const;
    int height() const;

    bool place_robot(const std::string& name, int x, int y);
    bool place_target(const std::string& name, int x, int y);
    bool place_obstacle(int x, int y);

    bool move_up(const std::string& robot_name);
    bool move_down(const std::string& robot_name);
    bool move_left(const std::string& robot_name);
    bool move_right(const std::string& robot_name);
    bool move_forward(const std::string& robot_name);
    bool move_toward(const std::string& robot_name, const std::string& target_name);

    void turn_left(const std::string& robot_name);
    void turn_right(const std::string& robot_name);

    int distance_to(const std::string& robot_name, const std::string& target_name) const;
    bool obstacle_ahead(const std::string& robot_name) const;
    bool collision(const std::string& robot_name) const;

    const Robot& robot(const std::string& name) const;
    const Target& target(const std::string& name) const;
    const std::vector<Robot>& robots() const;
    const std::vector<Target>& targets() const;

    void clear_collision_flags();

    Cell cell_at(int x, int y) const;

private:
    int width_{0};
    int height_{0};
    std::vector<Cell> grid_;
    std::vector<Robot> robots_;
    std::vector<Target> targets_;
    std::unordered_map<std::string, std::size_t> robot_index_;
    std::unordered_map<std::string, std::size_t> target_index_;

    bool in_bounds(int x, int y) const;
    Cell& cell_at_mut(int x, int y);
    bool is_passable(int x, int y) const;
    bool try_move_robot(Robot& robot, int dx, int dy);
    Robot* find_robot(const std::string& name);
    const Robot* find_robot(const std::string& name) const;
    const Target* find_target(const std::string& name) const;
    std::size_t grid_index(int x, int y) const;
};

} // namespace causis::runtime
