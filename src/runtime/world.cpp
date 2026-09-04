#include "runtime/world.h"

#include <cmath>
#include <stdexcept>

namespace causis::runtime {

World::World(int width, int height) : width_(width), height_(height) {
    if (width_ <= 0 || height_ <= 0) {
        throw std::invalid_argument("world dimensions must be positive");
    }

    grid_.resize(static_cast<std::size_t>(width_ * height_));
}

int World::width() const {
    return width_;
}

int World::height() const {
    return height_;
}

// Converts 2D coordinates to a flat grid index. Caller must ensure in_bounds(x, y).
std::size_t World::grid_index(int x, int y) const {
    return static_cast<std::size_t>(y * width_ + x);
}

bool World::in_bounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

Cell& World::cell_at_mut(int x, int y) {
    if (!in_bounds(x, y)) {
        throw std::out_of_range("cell coordinates out of bounds");
    }

    return grid_[grid_index(x, y)];
}

Cell World::cell_at(int x, int y) const {
    if (!in_bounds(x, y)) {
        throw std::out_of_range("cell coordinates out of bounds");
    }

    return grid_[grid_index(x, y)];
}

bool World::is_passable(int x, int y) const {
    if (!in_bounds(x, y)) {
        return false;
    }

    return cell_at(x, y).kind == CellKind::Empty;
}

bool World::place_robot(const std::string& name, int x, int y) {
    if (robot_index_.contains(name)) {
        return false;
    }

    if (!in_bounds(x, y) || !is_passable(x, y)) {
        return false;
    }

    Robot robot;
    robot.name = name;
    robot.x = x;
    robot.y = y;
    robot.direction = Direction::Right;
    robot.collision_flag = false;

    robot_index_[name] = robots_.size();
    robots_.push_back(robot);

    Cell& cell = cell_at_mut(x, y);
    cell.kind = CellKind::Robot;
    cell.label = name;
    return true;
}

bool World::place_target(const std::string& name, int x, int y) {
    if (target_index_.contains(name)) {
        return false;
    }

    if (!in_bounds(x, y) || !is_passable(x, y)) {
        return false;
    }

    Target target;
    target.name = name;
    target.x = x;
    target.y = y;

    target_index_[name] = targets_.size();
    targets_.push_back(target);

    Cell& cell = cell_at_mut(x, y);
    cell.kind = CellKind::Target;
    cell.label = name;
    return true;
}

bool World::place_obstacle(int x, int y) {
    if (!in_bounds(x, y) || !is_passable(x, y)) {
        return false;
    }

    Cell& cell = cell_at_mut(x, y);
    cell.kind = CellKind::Obstacle;
    cell.label.clear();
    return true;
}

Robot* World::find_robot(const std::string& name) {
    const auto found = robot_index_.find(name);
    if (found == robot_index_.end()) {
        return nullptr;
    }

    return &robots_[found->second];
}

const Robot* World::find_robot(const std::string& name) const {
    const auto found = robot_index_.find(name);
    if (found == robot_index_.end()) {
        return nullptr;
    }

    return &robots_[found->second];
}

const Target* World::find_target(const std::string& name) const {
    const auto found = target_index_.find(name);
    if (found == target_index_.end()) {
        return nullptr;
    }

    return &targets_[found->second];
}

bool World::try_move_robot(Robot& robot, int dx, int dy) {
    const int new_x = robot.x + dx;
    const int new_y = robot.y + dy;

    if (!in_bounds(new_x, new_y) || !is_passable(new_x, new_y)) {
        robot.collision_flag = true;
        return false;
    }

    // modify/move cells
    Cell& old_cell = cell_at_mut(robot.x, robot.y);
    old_cell.kind = CellKind::Empty;
    old_cell.label.clear();

    robot.x = new_x;
    robot.y = new_y;

    Cell& new_cell = cell_at_mut(new_x, new_y);
    new_cell.kind = CellKind::Robot;
    new_cell.label = robot.name;
    return true;
}

bool World::move_up(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return false;
    }

    return try_move_robot(*robot, 0, -1);
}

bool World::move_down(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return false;
    }

    return try_move_robot(*robot, 0, 1);
}

bool World::move_left(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return false;
    }

    return try_move_robot(*robot, -1, 0);
}

bool World::move_right(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return false;
    }

    return try_move_robot(*robot, 1, 0);
}

bool World::move_forward(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return false;
    }

    const Position delta = direction_delta(robot->direction);
    return try_move_robot(*robot, delta.x, delta.y);
}

bool World::move_toward(const std::string& robot_name, const std::string& target_name) {
    Robot* robot = find_robot(robot_name);
    const Target* target = find_target(target_name);
    if (robot == nullptr || target == nullptr) {
        return false;
    }

    const int dx = target->x - robot->x;
    const int dy = target->y - robot->y;
    const int abs_dx = std::abs(dx);
    const int abs_dy = std::abs(dy);

    if (abs_dx == 0 && abs_dy == 0) {
        robot->collision_flag = true;
        return false;
    }

    if (abs_dx >= abs_dy) {
        const int step_x = dx > 0 ? 1 : -1;
        return try_move_robot(*robot, step_x, 0);
    }

    const int step_y = dy > 0 ? 1 : -1;
    return try_move_robot(*robot, 0, step_y);
}

void World::turn_left(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return;
    }

    robot->direction = ::causis::runtime::turn_left(robot->direction);
}

void World::turn_right(const std::string& robot_name) {
    Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return;
    }

    robot->direction = ::causis::runtime::turn_right(robot->direction);
}

int World::distance_to(const std::string& robot_name, const std::string& target_name) const {
    const Robot* robot = find_robot(robot_name);
    const Target* target = find_target(target_name);
    if (robot == nullptr || target == nullptr) {
        return 0;
    }

    return std::abs(robot->x - target->x) + std::abs(robot->y - target->y);
}

bool World::obstacle_ahead(const std::string& robot_name) const {
    const Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return true;
    }

    const Position delta = direction_delta(robot->direction);
    const int ahead_x = robot->x + delta.x;
    const int ahead_y = robot->y + delta.y;

    if (!in_bounds(ahead_x, ahead_y)) {
        return true;
    }

    return cell_at(ahead_x, ahead_y).kind == CellKind::Obstacle;
}

bool World::collision(const std::string& robot_name) const {
    const Robot* robot = find_robot(robot_name);
    if (robot == nullptr) {
        return false;
    }

    return robot->collision_flag;
}

const Robot& World::robot(const std::string& name) const {
    const Robot* found = find_robot(name);
    if (found == nullptr) {
        throw std::out_of_range("unknown robot");
    }

    return *found;
}

const Target& World::target(const std::string& name) const {
    const Target* found = find_target(name);
    if (found == nullptr) {
        throw std::out_of_range("unknown target");
    }

    return *found;
}

const std::vector<Robot>& World::robots() const {
    return robots_;
}

const std::vector<Target>& World::targets() const {
    return targets_;
}

void World::clear_collision_flags() {
    for (Robot& robot : robots_) {
        robot.collision_flag = false;
    }
}

} // namespace causis::runtime
