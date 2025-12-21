# code_stats.py
#
# This file is part of BloodHorn and is licensed under the BSD License.
# See the root of the repository for license details.
#

"""
Code Statistics Counter

This script counts the number of files and lines of code in a project.
It supports various programming languages and can be customized to include/exclude
specific directories and file types.
"""

import os
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Set
import time

# File extensions to include in the count
CODE_EXTENSIONS = {
    # C/C++
    '.c', '.h', '.cpp', '.hpp', '.cc', '.hh', '.cxx', '.hxx', '.inl',
    # Python
    '.py',
    # Assembly
    '.asm', '.s', '.S', '.nasm',
    # Scripts
    '.sh', '.bat', '.cmd', '.ps1',
    # Makefiles and build scripts
    'Makefile', 'makefile', 'GNUmakefile',
    # Configuration
    '.dsc', '.dec', '.inf', '.fdf', '.uni', '.hii', '.asl', '.asi',
    # Documentation
    '.md', '.rst', '.txt', '.dox',
    # Other
    '.edk2', '.vfr'
}

# Directories to exclude (relative to project root)
EXCLUDE_DIRS = {
    '.git',
    '.github',
    '.vscode',
    'Build',
    'Conf',
    'Debug',
    'Release',
    'x64',
    'x86',
    'arm',
    'aarch64',
    'riscv64',
    'loongarch64',
    'venv',
    '__pycache__',
    'node_modules',
    'bin',
    'obj',
    'pkg'
}


def is_code_file(file_path: Path) -> bool:
    """Check if a file should be included in the code count."""
    # Check file extension
    if file_path.suffix.lower() in CODE_EXTENSIONS:
        return True
    
    # Check for Makefiles
    if file_path.name in CODE_EXTENSIONS:
        return True
    
    return False


def count_lines(file_path: Path) -> Tuple[int, int, int]:
    """Count lines in a file, including blank lines and comments."""
    total_lines = 0
    blank_lines = 0
    comment_lines = 0
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                line = line.strip()
                total_lines += 1
                
                if not line:
                    blank_lines += 1
                elif line.startswith(('//', '#', ';', '--', '/*', '*', '*/', '\'"""')):
                    comment_lines += 1
    except (IOError, UnicodeDecodeError):
        # Skip files that can't be read as text
        return 0, 0, 0
    
    return total_lines, blank_lines, comment_lines


def scan_directory(root_dir: Path) -> Dict[str, Dict]:
    """Scan a directory recursively and collect code statistics."""
    stats = {
        'files': 0,
        'lines': 0,
        'blank_lines': 0,
        'comment_lines': 0,
        'by_extension': {}
    }
    
    for item in root_dir.rglob('*'):
        # Skip excluded directories
        if any(part in EXCLUDE_DIRS for part in item.parts):
            continue
            
        if item.is_file() and is_code_file(item):
            ext = item.suffix.lower()
            if ext not in stats['by_extension']:
                stats['by_extension'][ext] = {
                    'files': 0,
                    'lines': 0,
                    'blank_lines': 0,
                    'comment_lines': 0
                }
            
            lines, blank, comments = count_lines(item)
            stats['files'] += 1
            stats['lines'] += lines
            stats['blank_lines'] += blank
            stats['comment_lines'] += comments
            
            stats['by_extension'][ext]['files'] += 1
            stats['by_extension'][ext]['lines'] += lines
            stats['by_extension'][ext]['blank_lines'] += blank
            stats['by_extension'][ext]['comment_lines'] += comments
    
    return stats


def format_number(number: int) -> str:
    """Format a number with thousands separators."""
    return f"{number:,}"


def print_stats(stats: Dict, duration: float):
    """Print the collected statistics in a readable format."""
    print("\n" + "=" * 80)
    print(f"{'Code Statistics':^80}")
    print("=" * 80)
    print(f"{'Total files:':<20} {format_number(stats['files'])}")
    print(f"{'Total lines:':<20} {format_number(stats['lines'])}")
    print(f"{'Blank lines:':<20} {format_number(stats['blank_lines'])} "
          f"({stats['blank_lines'] / stats['lines'] * 100:.1f}%)")
    print(f"{'Comment lines:':<20} {format_number(stats['comment_lines'])} "
          f"({stats['comment_lines'] / stats['lines'] * 100:.1f}%)")
    code_lines = stats['lines'] - stats['blank_lines'] - stats['comment_lines']
    print(f"{'Code lines:':<20} {format_number(code_lines)} "
          f"({code_lines / stats['lines'] * 100:.1f}%)")
    
    print("\n" + "-" * 80)
    print(f"{'By file type:':<10} {'Files':>10} {'Lines':>15} {'Blank':>10} {'Comments':>10} {'Code':>10}")
    print("-" * 80)
    
    # Sort extensions by line count (descending)
    sorted_exts = sorted(
        stats['by_extension'].items(),
        key=lambda x: x[1]['lines'],
        reverse=True
    )
    
    for ext, ext_stats in sorted_exts:
        ext_code = ext_stats['lines'] - ext_stats['blank_lines'] - ext_stats['comment_lines']
        print(
            f"{ext or 'Makefile':<10} "
            f"{ext_stats['files']:>10,} "
            f"{ext_stats['lines']:>15,} "
            f"{ext_stats['blank_lines']:>10,} "
            f"{ext_stats['comment_lines']:>10,} "
            f"{ext_code:>10,}"
        )
    
    print("=" * 80)
    print(f"Scan completed in {duration:.2f} seconds")
    print("=" * 80)


def main():
    """Main function."""
    if len(sys.argv) > 1:
        root_dir = Path(sys.argv[1]).resolve()
    else:
        root_dir = Path('.').resolve()
    
    if not root_dir.exists() or not root_dir.is_dir():
        print(f"Error: Directory not found: {root_dir}")
        sys.exit(1)
    
    print(f"Scanning directory: {root_dir}")
    print("This may take a while for large projects...\n")
    
    start_time = time.time()
    stats = scan_directory(root_dir)
    duration = time.time() - start_time
    
    print_stats(stats, duration)


if __name__ == "__main__":
    main()
