use rustc_hash::FxHashMap;

pub struct NameToId {
    current_id: usize,
    name_ids: FxHashMap<String, usize>,
}
impl NameToId {
    pub fn new() -> Self {
        Self::from(0)
    }

    pub fn from(start: usize) -> Self {
        Self {
            current_id: start,
            name_ids: FxHashMap::default(),
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

/// Writes n's decimal digits at the front of buf, returning how many it used.
///
/// Removes the need for `format!` which would allocate and free a String.
pub fn write_u32(buf: &mut [u8], mut n: u32) -> usize {
    let mut digits = [0u8; 10];
    let mut first = digits.len();
    loop {
        first -= 1;
        digits[first] = b'0' + (n % 10) as u8;
        n /= 10;
        if n == 0 {
            break;
        }
    }
    let len = digits.len() - first;
    buf[..len].copy_from_slice(&digits[first..]);
    len
}
