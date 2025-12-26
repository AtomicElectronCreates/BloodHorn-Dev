#![no_std]

/// Tiny no_std key=value parser for boot config-style files.
/// Lines like `key = value` or `key=value`. Comments starting with `#`.

pub struct Line<'a> {
    pub key: &'a str,
    pub value: &'a str,
}

pub fn parse_line(line: &str) -> Option<Line<'_>> {
    let trimmed = trim_ws(line);
    if trimmed.is_empty() || trimmed.as_bytes()[0] == b'#' {
        return None;
    }
    let bytes = trimmed.as_bytes();
    let mut split = None;
    for (i, &b) in bytes.iter().enumerate() {
        if b == b'=' {
            split = Some(i);
            break;
        }
    }
    let idx = split?;
    let (k, v) = trimmed.split_at(idx);
    let v = &v[1..];
    let key = trim_ws(k);
    let value = trim_ws(v);
    if key.is_empty() { return None; }
    Some(Line { key, value })
}

fn trim_ws(s: &str) -> &str {
    let bytes = s.as_bytes();
    let mut start = 0;
    let mut end = bytes.len();
    while start < end && is_ws(bytes[start]) { start += 1; }
    while end > start && is_ws(bytes[end - 1]) { end -= 1; }
    &s[start..end]
}

fn is_ws(b: u8) -> bool {
    matches!(b, b' ' | b'\t' | b'\n' | b'\r')
}
