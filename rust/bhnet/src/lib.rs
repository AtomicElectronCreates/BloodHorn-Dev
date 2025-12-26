#![no_std]

/// Simple network helpers for early boot: MAC, IPv4, and checksum.

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub struct Mac(pub [u8; 6]);

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub struct Ipv4(pub [u8; 4]);

/// RFC 1071 style 16-bit checksum.
pub fn checksum16(mut sum: u32, bytes: &[u8]) -> u16 {
    let mut i = 0;
    while i + 1 < bytes.len() {
        let word = ((bytes[i] as u32) << 8) | (bytes[i + 1] as u32);
        sum = sum.wrapping_add(word);
        i += 2;
    }
    if i < bytes.len() {
        let word = (bytes[i] as u32) << 8;
        sum = sum.wrapping_add(word);
    }
    while (sum >> 16) != 0 {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    !(sum as u16)
}
