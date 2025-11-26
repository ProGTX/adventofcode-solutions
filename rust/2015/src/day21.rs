use itertools::Itertools;

#[derive(Clone)]
struct Entity {
    value: u32,
    damage: u32,
    armor: u32,
}

impl Entity {
    // This function exists purely to reduce the lines of code
    // taken up by construction in arrays
    const fn new(value: u32, damage: u32, armor: u32) -> Self {
        return Entity {
            value,
            damage,
            armor,
        };
    }

    // Returns true if victim was defeated
    fn attack(&self, victim: &mut Entity) -> bool {
        let actual_damage = if (self.damage <= victim.armor) {
            1
        } else {
            self.damage - victim.armor
        };
        debug_assert!(actual_damage > 0, "There is always some damage");
        return if (actual_damage >= victim.value) {
            victim.value = 0;
            true
        } else {
            victim.value -= actual_damage;
            false
        };
    }
}

// Returns true if player won
fn play_game(player: &mut Entity, boss: &mut Entity) -> bool {
    loop {
        if (player.attack(boss)) {
            return true;
        }
        if (boss.attack(player)) {
            return false;
        }
    }
}

fn parse(filename: &str) -> Entity {
    let lines = aoc::file::read_lines(filename);
    let mut lines_it = lines.iter();
    let mut parse_line = || {
        let line = lines_it.next().unwrap();
        let (_, value_str) = line.split_once(": ").unwrap();
        let value = value_str.parse::<u32>().unwrap();
        return value;
    };
    return Entity {
        value: parse_line(),
        damage: parse_line(),
        armor: parse_line(),
    };
}

const SHOP_WEAPONS: [Entity; 5] = [
    Entity::new(8, 4, 0),
    Entity::new(10, 5, 0),
    Entity::new(25, 6, 0),
    Entity::new(40, 7, 0),
    Entity::new(74, 8, 0),
];
const SHOP_ARMOR: [Entity; 6] = [
    Entity::new(0, 0, 0), // No armor
    Entity::new(13, 0, 1),
    Entity::new(31, 0, 2),
    Entity::new(53, 0, 3),
    Entity::new(75, 0, 4),
    Entity::new(102, 0, 5),
];
// Note that the rings are sorted by price
// (just like weapons and armor are anyway)
const SHOP_RINGS: [Entity; 8] = [
    Entity::new(0, 0, 0), // No ring
    Entity::new(0, 0, 0), // No ring
    Entity::new(20, 0, 1),
    Entity::new(25, 1, 0),
    Entity::new(40, 0, 2),
    Entity::new(50, 2, 0),
    Entity::new(80, 0, 3),
    Entity::new(100, 3, 0),
];

fn solve_case<const START_HP: u32, const PART2: bool>(input: &Entity) -> u32 {
    let mut best_cost = if (PART2) {
        0
    } else {
        SHOP_WEAPONS[4].value + SHOP_ARMOR[4].value + SHOP_RINGS[4].value + SHOP_RINGS[5].value
    };
    let expected = !PART2;
    let update_cost = |current_cost: u32, new_cost: u32| {
        return if (!PART2) {
            current_cost.min(new_cost)
        } else {
            current_cost.max(new_cost)
        };
    };
    for weapon in &SHOP_WEAPONS {
        for armor in &SHOP_ARMOR {
            for ring_pair in SHOP_RINGS.iter().combinations(2) {
                let mut player = Entity {
                    value: START_HP,
                    damage: weapon.damage + ring_pair[0].damage + ring_pair[1].damage,
                    armor: armor.armor + ring_pair[0].armor + ring_pair[1].armor,
                };
                let mut boss = input.clone();
                if (play_game(&mut player, &mut boss) == expected) {
                    best_cost = update_cost(
                        best_cost,
                        weapon.value + armor.value + ring_pair[0].value + ring_pair[1].value,
                    );
                }
            }
        }
    }
    return best_cost;
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    assert_eq!(65, solve_case::<8, false>(&example));
    let input = parse("day21.input");
    assert_eq!(111, solve_case::<100, false>(&input));

    println!("Part 2");
    assert_eq!(188, solve_case::<8, true>(&example));
    assert_eq!(188, solve_case::<100, true>(&input));
}
