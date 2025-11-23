#[derive(Clone)]
/// Used for the player and for immediate actions in spells
struct Entity {
    hp: u16,
    mana: u32, // Used as damage in immediate spell actions
    armor: u16,
}
type Player = Entity;
impl Entity {
    // Exists solely to reduce the lines of the SPELLS array
    const fn new(hp: u16, mana: u32, armor: u16) -> Self {
        Entity { hp, mana, armor }
    }
}

#[derive(Clone)]
struct Boss {
    hp: u16,
    damage: u16,
}

#[derive(PartialEq)]
enum TurnWinner {
    Tie,
    Player,
    Boss,
}

enum Effect {
    MagicMissile, // Not an effect, but easier to treat as one
    Drain,        // Not an effect, but easier to treat as one
    Shield,
    Poison,
    Recharge,
}

struct Spell {
    cost: u16,
    effect: Effect,
    duration: u8,
    action: Entity,
}
impl Spell {
    // Exists solely to reduce the lines of the SPELLS array
    const fn new(cost: u16, effect: Effect, duration: u8, action: Entity) -> Self {
        Spell {
            cost,
            effect,
            duration,
            action,
        }
    }
}

const NO_ACTION: Entity = Entity {
    hp: 0,
    mana: 0,
    armor: 0,
};

const SPELLS: [Spell; 5] = [
    Spell::new(53, Effect::MagicMissile, 1, Entity::new(0, 4, 0)),
    Spell::new(73, Effect::Drain, 1, Entity::new(2, 2, 0)),
    Spell::new(113, Effect::Shield, 6, Entity::new(0, 0, 7)),
    Spell::new(173, Effect::Poison, 6, NO_ACTION),
    Spell::new(229, Effect::Recharge, 5, NO_ACTION),
];

#[derive(Clone)]
struct GameState {
    player: Player,
    boss: Boss,
    mana_spent: u32,
    instant_damage: u8, // Treated as an effect of duration 1
    shield_left: u8,
    poison_left: u8,
    recharge_left: u8,
}

enum PlayerTurnResult {
    GameEnded(TurnWinner),
    PossiblePlays(Vec<GameState>),
}

impl GameState {
    /// Returns true if player died
    // Only used on hard difficulty
    fn bleed_player(&mut self) -> bool {
        self.player.hp -= 1;
        return self.player.hp == 0;
    }

    /// Returns true if boss was defeated
    fn apply_effect(&mut self) -> bool {
        if (self.instant_damage > 0) {
            let instant_damage = self.instant_damage as u16;
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

    fn player_turn(&mut self, lowest_mana: u32) -> PlayerTurnResult {
        if (self.apply_effect()) {
            return PlayerTurnResult::GameEnded(TurnWinner::Player);
        }
        let possible_plays = SPELLS
            .iter()
            .filter(|spell| {
                return ((spell.cost as u32) < self.player.mana)
                    // Pruning based on lowest_mana is key to solving this
                    // in a reasonable amount of time 
                    && ((self.mana_spent + (spell.cost as u32)) < lowest_mana)
                    && match spell.effect {
                        // Cannot cast a spell that's still in effect
                        Effect::Shield => self.shield_left == 0,
                        Effect::Poison => self.poison_left == 0,
                        Effect::Recharge => self.recharge_left == 0,
                        _ => true,
                    };
            })
            .map(|spell| {
                let mut new_state = self.clone();
                new_state.mana_spent += spell.cost as u32;
                new_state.player.mana -= spell.cost as u32;
                new_state.player.hp += spell.action.hp;
                new_state.player.armor += spell.action.armor;
                match spell.effect {
                    Effect::Shield => new_state.shield_left = spell.duration,
                    Effect::Poison => new_state.poison_left = spell.duration,
                    Effect::Recharge => new_state.recharge_left = spell.duration,
                    _ => new_state.instant_damage = spell.action.mana as u8,
                }
                new_state
            })
            .collect::<Vec<GameState>>();
        return if (possible_plays.is_empty()) {
            PlayerTurnResult::GameEnded(TurnWinner::Boss)
        } else {
            PlayerTurnResult::PossiblePlays(possible_plays)
        };
    }

    fn boss_turn(&mut self) -> TurnWinner {
        if (self.apply_effect()) {
            return TurnWinner::Player;
        }
        let actual_damage = if (self.boss.damage <= self.player.armor) {
            1
        } else {
            self.boss.damage - self.player.armor
        };
        debug_assert!(actual_damage > 0, "There is always some damage");
        return if (actual_damage >= self.player.hp) {
            self.player.hp = 0;
            TurnWinner::Boss
        } else {
            self.player.hp -= actual_damage;
            TurnWinner::Tie
        };
    }
}

fn parse(filename: &str) -> Boss {
    let mut lines_it = aoc::file::read_lines(filename);
    let mut parse_line = || {
        let line = lines_it.next().unwrap().unwrap();
        let (_, value_str) = line.split_once(": ").unwrap();
        let value = value_str.parse::<u16>().unwrap();
        return value;
    };
    return Boss {
        hp: parse_line(),
        damage: parse_line(),
    };
}

fn solve_case<const FULL_INPUT: bool, const HARD_MODE: bool>(boss_orig: &Boss) -> u32 {
    let mut initial_state = GameState {
        player: Player {
            hp: if (FULL_INPUT) { 50 } else { 10 },
            mana: if (FULL_INPUT) { 500 } else { 250 },
            armor: 0,
        },
        boss: boss_orig.clone(),
        mana_spent: 0,
        instant_damage: 0,
        shield_left: 0,
        poison_left: 0,
        recharge_left: 0,
    };
    if (HARD_MODE) {
        if (!FULL_INPUT) {
            // Examples cannot be solved on hard mode, make it slightly easier again
            initial_state.player.hp += 5;
        }
        initial_state.bleed_player();
    }
    let mut lowest_mana = u32::MAX;
    let mut game_stack = match initial_state.player_turn(lowest_mana) {
        PlayerTurnResult::PossiblePlays(possible_plays) => possible_plays,
        PlayerTurnResult::GameEnded(_) => unreachable!("Game shouldn't have ended on first turn"),
    };
    drop(initial_state);
    while (!game_stack.is_empty()) {
        let mut current_game = game_stack.pop().unwrap();
        match current_game.boss_turn() {
            TurnWinner::Player => {
                lowest_mana = lowest_mana.min(current_game.mana_spent);
                continue;
            }
            TurnWinner::Boss => continue,
            TurnWinner::Tie => debug_assert!(
                current_game.player.hp > 0,
                "A tie must leave the player with some health"
            ),
        };
        if (HARD_MODE && current_game.bleed_player()) {
            continue;
        }
        match current_game.player_turn(lowest_mana) {
            PlayerTurnResult::GameEnded(TurnWinner::Player) => {
                lowest_mana = lowest_mana.min(current_game.mana_spent);
                continue;
            }
            PlayerTurnResult::GameEnded(TurnWinner::Boss) => continue,
            PlayerTurnResult::GameEnded(TurnWinner::Tie) => {
                unreachable!("Player turn cannot end in a tie, must get possible new plays")
            }
            PlayerTurnResult::PossiblePlays(possible_plays) => {
                game_stack.extend(possible_plays);
            }
        }
    }
    lowest_mana
}

fn main() {
    println!("Part 1");
    let example = parse("day22.example");
    assert_eq!(226, solve_case::<false, false>(&example));
    let example2 = parse("day22.example2");
    assert_eq!(641, solve_case::<false, false>(&example2));
    let input = parse("day22.input");
    assert_eq!(953, solve_case::<true, false>(&input));

    println!("Part 2");
    assert_eq!(226, solve_case::<false, true>(&example));
    assert_eq!(588, solve_case::<false, true>(&example2));
    assert_eq!(1289, solve_case::<true, true>(&input));
}
