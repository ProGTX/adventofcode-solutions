use std::collections::HashMap;

pub struct NameToId {
    current_id: usize,
    name_ids: HashMap<String, usize>,
}
impl NameToId {
    pub fn new() -> Self {
        Self {
            current_id: 0,
            name_ids: HashMap::new(),
        }
    }

    pub fn intern(&mut self, name: &str) -> usize {
        if let Some(id) = self.name_ids.get(name) {
            *id
        } else {
            let id = self.current_id.clone();
            self.name_ids.insert(name.to_string(), id);
            self.current_id += 1;
            id
        }
    }

    pub fn get(&self, name: &str) -> Option<&usize> {
        self.name_ids.get(name)
    }

    pub fn expect(&self, name: &str) -> usize {
        *self.get(name).expect(&format!("name '{}' not found", name))
    }

    pub fn new_len(&self, old_len: usize) -> usize {
        self.current_id.max(old_len)
    }
}
