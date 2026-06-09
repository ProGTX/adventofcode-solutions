// https://adventofcode.com/2023/day/7

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <compare>
#include <print>
#include <ranges>

using Input = Vec<std::pair<String, i32>>;

auto parse(String const& filename) -> Input {
  using Pair = Input::value_type;
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           auto [hand_str, bid_str] = aoc::split_once(line, ' ');
           return Pair{String{hand_str}, aoc::to_number<i32>(bid_str)};
         }) |
         aoc::collect_vec<Pair>();
}

constexpr let joker_card_number = 1;
constexpr let ace_card_number = 14;

template <bool joker>
fn card_to_number(char c) -> i32 {
  if (aoc::is_number(c)) {
    let number = static_cast<i32>(c - '0');
    AOC_ASSERT((number > joker_card_number) && (number < 10),
               "Invalid card number");
    return number;
  }
  switch (c) {
    case 'T':
      return 10;
    case 'J':
      if constexpr (joker) {
        return joker_card_number;
      } else {
        return 11;
      }
    case 'Q':
      return 12;
    case 'K':
      return 13;
    case 'A':
      return ace_card_number;
    default:
      throw std::runtime_error("Invalid card number");
  }
}

static_assert(7 == card_to_number<false>('7'));
static_assert(13 == card_to_number<false>('K'));
static_assert(11 == card_to_number<false>('J'));
static_assert(1 == card_to_number<true>('J'));

enum class type_t {
  high_card,
  one_pair,
  two_pair,
  three_kind,
  full_house,
  four_kind,
  five_kind,
};

template <bool joker>
struct hand_type {
  using cards_t = std::array<i32, 5>;

  cards_t cards;

  constexpr hand_type(str cards_str) {
    stdr::transform(cards_str, cards.begin(), &card_to_number<joker>);
  }

  constexpr hand_type(const cards_t& cards_) : cards{cards_} {}

  fn type() const -> type_t {
    std::array<i32, ace_card_number> card_map{};
    i32 num_jokers = 0;
    for (auto c : cards) {
      if (c == joker_card_number) {
        if constexpr (joker) {
          ++num_jokers;
        } else {
          // This shouldn't actually be triggered,
          // we're just preventing code from being generated unnecessarily
        }
      } else {
        // Minus 2 to exclude 0 and 1
        ++card_map[c - 2];
      }
    }
    stdr::sort(card_map, std::greater<>{});
    if constexpr (joker) {
      card_map[0] += num_jokers;
    }
    switch (card_map[0]) {
      case 5:
        return type_t::five_kind;
      case 4:
        return type_t::four_kind;
      case 3:
        if (card_map[1] == 2) {
          return type_t::full_house;
        } else {
          return type_t::three_kind;
        }
      case 2:
        if (card_map[1] == 2) {
          return type_t::two_pair;
        } else {
          return type_t::one_pair;
        }
      default:
        return type_t::high_card;
    }
  }

  fn operator<=>(const hand_type& other) const->std::strong_ordering {
    auto type_comp = (type() <=> other.type());
    if (type_comp != std::strong_ordering::equal) {
      return type_comp;
    }
    return std::strong_order(cards, other.cards);
  }

  constexpr bool operator==(const hand_type& other) const = default;
};

static_assert(hand_type<false>{"AAAAA"}.type() == type_t::five_kind);
static_assert(hand_type<false>{"AA8AA"}.type() == type_t::four_kind);
static_assert(hand_type<false>{"23332"}.type() == type_t::full_house);
static_assert(hand_type<false>{"TTT98"}.type() == type_t::three_kind);
static_assert(hand_type<false>{"23432"}.type() == type_t::two_pair);
static_assert(hand_type<false>{"A23A4"}.type() == type_t::one_pair);
static_assert(hand_type<false>{"23456"}.type() == type_t::high_card);

static_assert(hand_type<false>{"T55J5"}.type() == type_t::three_kind);
static_assert(hand_type<true>{"T55J5"}.type() == type_t::four_kind);

static_assert(hand_type<false>{"23332"} > hand_type<false>{"TTT98"});
static_assert(hand_type<false>{"33332"} > hand_type<false>{"2AAAA"});
static_assert(hand_type<false>{"77888"} > hand_type<false>{"77788"});

static_assert(hand_type<false>{"KTJJT"} < hand_type<false>{"KK677"});
static_assert(hand_type<true>{"KTJJT"} > hand_type<true>{"KK677"});

template <bool joker>
using bid_type = std::pair<hand_type<joker>, i32>;

template <bool joker>
fn total_winnings(Vec<bid_type<joker>> const& bids) -> i32 {
  let sorted_bids = aoc::ranges::sorted(bids);
  return aoc::ranges::accumulate(
      sorted_bids | stdv::enumerate | stdv::transform([](let& elem) {
        let & [ i, bid ] = elem;
        return static_cast<i32>(i + 1) * bid.second;
      }),
      0);
}

static_assert((765 * 1 + 12 * 2) ==
              total_winnings<false>({bid_type<false>{"KTJJT", 765},
                                     bid_type<false>{"QQQJA", 12}}));
static_assert((765 * 2 + 12 * 1) ==
              total_winnings<true>({bid_type<true>{"KTJJT", 765},
                                    bid_type<true>{"QQQJA", 12}}));

template <bool joker>
fn solve_case(Input const& raw) -> i32 {
  return total_winnings<joker>( //
      raw |
      stdv::transform([](let& elem) {
        let & [ hand_str, bid ] = elem;
        return bid_type<joker>(hand_str, bid);
      }) |
      aoc::collect_vec<bid_type<joker>>());
}

int main() {
  std::println("Part 1");
  let example = parse("day07.example");
  AOC_EXPECT_RESULT(6440, (solve_case<false>(example)));
  let input = parse("day07.input");
  AOC_EXPECT_RESULT(249638405, (solve_case<false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(5905, (solve_case<true>(example)));
  AOC_EXPECT_RESULT(249776650, (solve_case<true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
