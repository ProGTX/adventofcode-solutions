// https://adventofcode.com/2015/day/21

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>

struct Entity {
  u32 value;
  u32 damage;
  u32 armor;

  // Returns true if victim was defeated
  fn attack(Entity& victim) const -> bool {
    let actual_damage = [&] -> u32 {
      if (damage <= victim.armor) {
        return 1;
      } else {
        return damage - victim.armor;
      };
    }();
    if (actual_damage >= victim.value) {
      victim.value = 0;
      return true;
    } else {
      victim.value -= actual_damage;
      return false;
    };
  }
};

// Returns true if player won
fn play_game(Entity& player, Entity& boss) -> bool {
  loop {
    if (player.attack(boss)) {
      return true;
    }
    if (boss.attack(player)) {
      return false;
    }
  }
}

fn parse(String const& filename) -> Entity {
  auto lines_view = aoc::views::read_lines(filename);
  auto line_it = std::ranges::begin(lines_view);
  let parse_line = [&] {
    let[_, value_str] = aoc::split_once(*line_it, ": ");
    let value = aoc::to_number<u32>(value_str);
    ++line_it;
    return value;
  };
  return Entity{
      .value = parse_line(),
      .damage = parse_line(),
      .armor = parse_line(),
  };
}

constexpr const auto shop_weapons = std::array{
    Entity{8, 4, 0},  //
    Entity{10, 5, 0}, //
    Entity{25, 6, 0}, //
    Entity{40, 7, 0}, //
    Entity{74, 8, 0}, //
};
constexpr const auto shop_armor = std::array{
    Entity{0, 0, 0},   // No armor
    Entity{13, 0, 1},  //
    Entity{31, 0, 2},  //
    Entity{53, 0, 3},  //
    Entity{75, 0, 4},  //
    Entity{102, 0, 5}, //
};
constexpr const auto shop_rings = std::array{
    Entity{0, 0, 0},   // No ring
    Entity{0, 0, 0},   // No ring
    Entity{20, 0, 1},  //
    Entity{25, 1, 0},  //
    Entity{40, 0, 2},  //
    Entity{50, 2, 0},  //
    Entity{80, 0, 3},  //
    Entity{100, 3, 0}, //
};

template <class T>
struct inspect_t;

template <u32 START_HP, bool PART2>
fn solve_case(Entity const& input) -> u32 {
  auto best_cost = aoc::as_consteval(PART2 ? 0
                                           : shop_weapons[4].value +
                                                 shop_armor[5].value +
                                                 shop_rings[6].value +
                                                 shop_rings[7].value);
  constexpr const bool expected = !PART2;
  const auto update_cost = [&](u32 new_cost) {
    if constexpr (!PART2) {
      return std::min(best_cost, new_cost);
    } else {
      return std::max(best_cost, new_cost);
    }
  };
  for (let weapon : shop_weapons) {
    for (let armor : shop_armor) {
      for (let& rings_combo : shop_rings | aoc::views::combinations(2u)) {
        let ring_pair =
            aoc::binary_select_from_combination<std::array<Entity, 2>>(
                shop_rings, rings_combo);
        auto player = Entity{
            .value = START_HP,
            .damage = weapon.damage + ring_pair[0].damage + ring_pair[1].damage,
            .armor = armor.armor + ring_pair[0].armor + ring_pair[1].armor,
        };
        auto boss = input;
        if (play_game(player, boss) == expected) {
          best_cost = update_cost(weapon.value +
                                  armor.value +
                                  ring_pair[0].value +
                                  ring_pair[1].value);
        }
      }
    }
  }
  return best_cost;
}

int main() {
  std::println("Part 1");
  let example = parse("day21.example");
  AOC_EXPECT_RESULT(65, (solve_case<8, false>(example)));
  let input = parse("day21.input");
  AOC_EXPECT_RESULT(111, (solve_case<100, false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(188, (solve_case<8, true>(example)));
  AOC_EXPECT_RESULT(188, (solve_case<100, true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
