#![no_std]

/// Minimal string utilities suitable for no_std boot environments.

/// Split `s` on the first occurrence of `delim`. Returns (left, right_opt).
pub fn split_once<'a>(s: &'a str, delim: char) -> (&'a str, Option<&'a str>) {
    let bytes = s.as_bytes();
    let d = delim as u8;
    for (i, &b) in bytes.iter().enumerate() {
        if b == d {
            let left = &s[..i];
            let right = if i + 1 < s.len() { Some(&s[i + 1..]) } else { Some("") };
            return (left, right);
        }
    }
    (s, None)
}

/// Check if `s` starts with ASCII prefix `p` in a case-insensitive way.
pub fn starts_with_ci(s: &str, p: &str) -> bool {
    if p.len() > s.len() { return false; }
    let s_bytes = s.as_bytes();
    let p_bytes = p.as_bytes();
    for i in 0..p_bytes.len() {
        let a = s_bytes[i].to_ascii_lowercase();
        let b = p_bytes[i].to_ascii_lowercase();
        if a != b { return false; }
    }
    true
}
