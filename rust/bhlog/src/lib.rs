#![no_std]

use core::fmt::{self, Write};

/// Log level compatible with typical bootloader messages.
#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Level {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
}

impl Level {
    fn as_str(self) -> &'static str {
        match self {
            Level::Trace => "TRACE",
            Level::Debug => "DEBUG",
            Level::Info  => "INFO",
            Level::Warn  => "WARN",
            Level::Error => "ERROR",
        }
    }
}

/// Logger trait used by the rest of the Rust-side code.
pub trait Logger {
    fn log(&mut self, level: Level, msg: &str);

    fn info(&mut self, msg: &str)  { self.log(Level::Info, msg); }
    fn warn(&mut self, msg: &str)  { self.log(Level::Warn, msg); }
    fn error(&mut self, msg: &str) { self.log(Level::Error, msg); }
}

/// Logger that writes into any `core::fmt::Write` sink.
pub struct WriterLogger<W: Write> {
    sink: W,
}

impl<W: Write> WriterLogger<W> {
    pub fn new(sink: W) -> Self {
        Self { sink }
    }
}

impl<W: Write> Logger for WriterLogger<W> {
    fn log(&mut self, level: Level, msg: &str) {
        let _ = write!(self.sink, "[{}] {}\n", level.as_str(), msg);
    }
}

/// A fixed-size in-memory ring buffer logger useful for early boot capture.
pub struct BufferLogger<const N: usize> {
    buf: [u8; N],
    start: usize,
    len: usize,
}

impl<const N: usize> BufferLogger<N> {
    pub const fn new() -> Self {
        Self { buf: [0; N], start: 0, len: 0 }
    }

    pub fn as_bytes(&self) -> &[u8] {
        if self.len <= N && self.start + self.len <= N {
            &self.buf[self.start..self.start + self.len]
        } else {
            &self.buf[..self.len.min(N)]
        }
    }
}

impl<const N: usize> Write for BufferLogger<N> {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        for &b in bytes {
            if N == 0 { return Ok(()); }
            let idx = (self.start + self.len) % N;
            self.buf[idx] = b;
            if self.len < N {
                self.len += 1;
            } else {
                self.start = (self.start + 1) % N;
            }
        }
        Ok(())
    }
}

impl<const N: usize> Logger for BufferLogger<N> {
    fn log(&mut self, level: Level, msg: &str) {
        let _ = write!(self, "[{}] {}\n", level.as_str(), msg);
    }
}
