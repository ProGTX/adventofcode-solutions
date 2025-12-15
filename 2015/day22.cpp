// https://adventofcode.com/2015/day/22

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <print>
#include <ranges>
#include <variant>

struct Entity {
  u16 hp;
  u32 mana; // Used as damage in immediate spell actions
  u16 armor;
};
using Player = Entity;

struct Boss {
  u16 hp;
  u16 damage;
};

enum class TurnWinner {
  Tie,
  Player,
  Boss,
};

enum Effect {
  MagicMissile, // Not an effect, but easier to treat as one
  Drain,        // Not an effect, but easier to treat as one
  Shield,
  Poison,
  Recharge,
};

struct Spell {
  u16 cost;
  Effect effect;
  u8 duration;
  Entity action;
};

constexpr let NO_ACTION = Entity{
    .hp = 0,
    .mana = 0,
    .armor = 0,
};

constexpr let SPELLS = std::array{
    Spell{53, Effect::MagicMissile, 1, Entity{0, 4, 0}},
    Spell{73, Effect::Drain, 1, Entity{2, 2, 0}},
    Spell{113, Effect::Shield, 6, Entity{0, 0, 7}},
    Spell{173, Effect::Poison, 6, NO_ACTION},
    Spell{229, Effect::Recharge, 5, NO_ACTION},
};

struct GameState;

namespace PlayerTurnResult {
struct GameEnded {
  TurnWinner winner;
};
struct PossiblePlays {
  Vec<GameState> plays;
};
using Any = std::variant<GameEnded, PossiblePlays>;
} // namespace PlayerTurnResult

struct GameState {
  Player player;
  Boss boss;
  u32 mana_spent;
  u8 instant_damage; // Treated as an effect of duration 1
  u8 shield_left;
  u8 poison_left;
  u8 recharge_left;

  /// Returns true if player died
  // Only used on hard difficulty
  fn bleed_player(this GameState& self) -> bool {
    self.player.hp -= 1;
    return self.player.hp == 0;
  }

  /// Returns true if boss was defeated
  fn apply_effect(this GameState& self) -> bool {
    if (self.instant_damage > 0) {
      let instant_damage = static_cast<u16>(self.instant_damage);
      self.instant_damage = 0;
      if (self.boss.hp <= instant_damage) {
        return true;
      } else {
        self.boss.hp -= instant_damage;
      }
    }
    if (self.poison_left > 0) {
      self.poison_left -= 1;
      if (self.boss.hp <= 3) {
        return true;
      } else {
        self.boss.hp -= 3;
      }
    }
    if (self.shield_left > 0) {
      self.shield_left -= 1;
      if (self.shield_left == 0) {
        self.player.armor -= 7;
      }
    }
    if (self.recharge_left > 0) {
      self.recharge_left -= 1;
      self.player.mana += 101;
    }
    return false;
  }

  fn player_turn(this GameState& self, const u32 lowest_mana)
      -> PlayerTurnResult::Any {
    if (self.apply_effect()) {
      return PlayerTurnResult::GameEnded(TurnWinner::Player);
    }
    let possible_plays =
        SPELLS |
        std::views::filter([&](Spell const& spell) {
          return (static_cast<u32>(spell.cost) < self.player.mana)
                 // Pruning based on lowest_mana is key to solving this
                 // in a reasonable amount of time
                 &&
                 ((self.mana_spent + static_cast<u32>(spell.cost)) <
                  lowest_mana) &&
                 [&] {
                   switch (spell.effect) {
                     // Cannot cast a spell that's still in effect
                     case Effect::Shield:
                       return self.shield_left == 0;
                     case Effect::Poison:
                       return self.poison_left == 0;
                     case Effect::Recharge:
                       return self.recharge_left == 0;
                     default:
                       return true;
                   }
                 }();
        }) |
        std::views::transform([&](Spell const& spell) {
          auto new_state = self;
          new_state.mana_spent += static_cast<u32>(spell.cost);
          new_state.player.mana -= static_cast<u32>(spell.cost);
          new_state.player.hp += spell.action.hp;
          new_state.player.armor += spell.action.armor;
          switch (spell.effect) {
            case Effect::Shield:
              new_state.shield_left = spell.duration;
              break;
            case Effect::Poison:
              new_state.poison_left = spell.duration;
              break;
            case Effect::Recharge:
              new_state.recharge_left = spell.duration;
              break;
            default:
              new_state.instant_damage = static_cast<u8>(spell.action.mana);
              break;
          }
          return new_state;
        }) |
        aoc::ranges::to<Vec<GameState>>();
    if (possible_plays.empty()) {
      return PlayerTurnResult::GameEnded(TurnWinner::Boss);
    } else {
      return PlayerTurnResult::PossiblePlays(possible_plays);
    };
  }

  fn boss_turn(this GameState& self) -> TurnWinner {
    if (self.apply_effect()) {
      return TurnWinner::Player;
    }
    let actual_damage = (self.boss.damage <= self.player.armor)
                            ? 1
                            : self.boss.damage - self.player.armor;
    AOC_ASSERT(actual_damage > 0, "There is always some damage");
    if (actual_damage >= self.player.hp) {
      self.player.hp = 0;
      return TurnWinner::Boss;
    } else {
      self.player.hp -= actual_damage;
      return TurnWinner::Tie;
    };
  }
};

auto parse(String const& filename) -> Boss {
  auto lines_view = aoc::views::read_lines(filename);
  auto line_it = std::ranges::begin(lines_view);
  let parse_line = [&] {
    let[_, value_str] = aoc::split_once(*line_it, ": ");
    let value = aoc::to_number<u16>(value_str);
    ++line_it;
    return value;
  };
  return Boss{
      .hp = parse_line(),
      .damage = parse_line(),
  };
}

template <bool FULL_INPUT, bool HARD_MODE>
fn solve_case(Boss const& boss_orig) -> u32 {
  auto initial_state = GameState{
      .player =
          Player{
              .hp = FULL_INPUT ? 50 : 10,
              .mana = FULL_INPUT ? 500 : 250,
              .armor = 0,
          },
      .boss = boss_orig,
      .mana_spent = 0,
      .instant_damage = 0,
      .shield_left = 0,
      .poison_left = 0,
      .recharge_left = 0,
  };
  if constexpr (HARD_MODE) {
    if constexpr (!FULL_INPUT) {
      // Examples cannot be solved on hard mode, make it slightly easier again
      initial_state.player.hp += 5;
    }
    (void)initial_state.bleed_player();
  }
  auto lowest_mana = std::numeric_limits<u32>::max();
  auto game_stack = aoc::match(
      initial_state.player_turn(lowest_mana),
      [](PlayerTurnResult::PossiblePlays possible_plays) {
        return possible_plays.plays;
      },
      [](PlayerTurnResult::GameEnded) -> Vec<GameState> {
        AOC_UNREACHABLE("Game shouldn't have ended on first turn");
      });
  while (!game_stack.empty()) {
    auto current_game = game_stack.back();
    game_stack.pop_back();
    switch (current_game.boss_turn()) {
      case TurnWinner::Player:
        lowest_mana = std::min(lowest_mana, current_game.mana_spent);
        continue;
      case TurnWinner::Boss:
        continue;
      case TurnWinner::Tie:
        AOC_ASSERT(current_game.player.hp > 0,
                   "A tie must leave the player with some health");
        break;
    }
    if constexpr (HARD_MODE) {
      if (current_game.bleed_player()) {
        continue;
      }
    }
    aoc::match(
        current_game.player_turn(lowest_mana),
        [&](PlayerTurnResult::GameEnded end_result) {
          switch (end_result.winner) {
            case TurnWinner::Player:
              lowest_mana = std::min(lowest_mana, current_game.mana_spent);
              break;
            case TurnWinner::Boss:
              break;
            case TurnWinner::Tie:
              AOC_UNREACHABLE("Player turn cannot end in a tie, "
                              "must get possible new plays");
          }
        },
        [&](PlayerTurnResult::PossiblePlays possible_plays) {
          aoc::ranges::extend(game_stack, possible_plays.plays);
        });
  }
  return lowest_mana;
}

int main() {
  std::println("Part 1");
  let example = parse("day22.example");
  AOC_EXPECT_RESULT(226, (solve_case<false, false>(example)));
  let example2 = parse("day22.example2");
  AOC_EXPECT_RESULT(641, (solve_case<false, false>(example2)));
  let input = parse("day22.input");
  AOC_EXPECT_RESULT(953, (solve_case<true, false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(226, (solve_case<false, true>(example)));
  AOC_EXPECT_RESULT(588, (solve_case<false, true>(example2)));
  AOC_EXPECT_RESULT(1289, (solve_case<true, true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
