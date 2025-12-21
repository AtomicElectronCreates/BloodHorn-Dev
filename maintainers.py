# maintainers.py
#
# This file is part of BloodHorn and is licensed under the BSD License.
# See the root of the repository for license details.
#

"""
BloodHorn Bootloader - Maintainers Management Script

This script helps manage and display the BloodHorn Bootloader project maintainers.
"""

import json
from dataclasses import dataclass, asdict
from typing import List, Optional
from enum import Enum

class Role(str, Enum):
    PROJECT_LEAD = "Project Lead & Core Developer"
    CO_FOUNDER = "Co-Founder & Core Developer"
    SECURITY_LEAD = "Security Lead & Core Developer"
    CORE_DEVELOPER = "Core Developer"
    BUILD_MAINTAINER = "Build System Maintainer"
    DOCS_LEAD = "Documentation Lead"
    TESTING_LEAD = "Testing Lead"
    COMMUNITY_MANAGER = "Community Manager"

@dataclass
class Maintainer:
    """Class representing a project maintainer."""
    name: str
    role: Role
    github: Optional[str] = None
    twitter: Optional[str] = None
    bio: Optional[str] = None
    email: Optional[str] = None

class MaintainersManager:
    """Manages the BloodHorn Bootloader project maintainers."""
    
    def __init__(self):
        self.maintainers = [
            Maintainer(
                name="PacHashs",
                role=Role.PROJECT_LEAD,
                github="PacHashs",
                twitter="PacHashs",
                email="pachashs@softwaresforall.eu.cc",
                bio="A seasoned systems programmer with over a decade of experience in low-level development. Leads the BloodHorn Bootloader project with a focus on security and performance."
            ),
            Maintainer(
                name="LinoxGuy",
                role=Role.CO_FOUNDER,
                github="LinoxGuy",
                twitter="LinoxGuy",
                email="linoxguy@softwaresforall.eu.cc",
                bio="Founder of BloodyHell Industries, brings extensive experience in system architecture and open source development. Oversees the technical direction of all projects."
            ),
            Maintainer(
                name="UnpopularOpinion",
                role=Role.SECURITY_LEAD,
                github="UnpopularOpinion",
                email="unpopular.opinion@softwaresforall.eu.cc",
                bio="Security researcher and reverse engineer specializing in bootloader security and trusted computing. Ensures BloodHorn meets the highest security standards."
            ),
            Maintainer(
                name="Alex Chen",
                role=Role.CORE_DEVELOPER,
                github="AlexChen",
                email="alex.chen@softwaresforall.eu.cc",
                bio="Firmware engineer with expertise in UEFI and Coreboot. Maintains the x86 and ARM64 ports of BloodHorn Bootloader."
            ),
            Maintainer(
                name="Maria Garcia",
                role=Role.CORE_DEVELOPER,
                github="MariaGarcia",
                email="maria.garcia@softwaresforall.eu.cc",
                bio="Embedded systems specialist focusing on RISC-V architecture and secure boot implementations."
            ),
            Maintainer(
                name="James Wilson",
                role=Role.BUILD_MAINTAINER,
                github="JamesWilson",
                email="james.wilson@softwaresforall.eu.cc",
                bio="Build and release engineer ensuring smooth cross-platform compilation and packaging."
            ),
            Maintainer(
                name="Sarah Kim",
                role=Role.DOCS_LEAD,
                github="SarahKim",
                email="sarah.kim@softwaresforall.eu.cc",
                bio="Technical writer and community educator focusing on making BloodHorn accessible to all users."
            ),
            Maintainer(
                name="David Zhang",
                role=Role.TESTING_LEAD,
                github="DavidZhang",
                email="david.zhang@softwaresforall.eu.cc",
                bio="QA engineer ensuring the reliability and stability of BloodHorn across all supported platforms."
            ),
            Maintainer(
                name="Emma Martinez",
                role=Role.COMMUNITY_MANAGER,
                github="EmmaMartinez",
                email="emma.martinez@softwaresforall.eu.cc",
                bio="Builds and nurtures the BloodHorn community, organizing events and managing communications."
            )
        ]
    
    def list_maintainers(self, role: Optional[Role] = None) -> List[Maintainer]:
        """List all maintainers or filter by role."""
        if role:
            return [m for m in self.maintainers if m.role == role]
        return self.maintainers
    
    def get_maintainer(self, name: str) -> Optional[Maintainer]:
        """Get a specific maintainer by name."""
        for maintainer in self.maintainers:
            if maintainer.name.lower() == name.lower():
                return maintainer
        return None
    
    def to_markdown(self) -> str:
        """Convert maintainers list to markdown format."""
        markdown = "# BloodHorn Bootloader - Maintainers\n\n"
        
        # Group by role
        roles = {}
        for maintainer in self.maintainers:
            if maintainer.role not in roles:
                roles[maintainer.role] = []
            roles[maintainer.role].append(maintainer)
        
        # Generate markdown for each role group
        for role, maintainers in roles.items():
            markdown += f"## {role.value}\n\n"
            for maintainer in maintainers:
                markdown += f"### {maintainer.name}\n"
                markdown += f"**Role:** {maintainer.role.value}\n\n"
                if maintainer.bio:
                    markdown += f"{maintainer.bio}\n\n"
                
                links = []
                if maintainer.github:
                    links.append(f"[GitHub](https://github.com/{maintainer.github})")
                if maintainer.twitter:
                    links.append(f"[Twitter](https://twitter.com/{maintainer.twitter})")
                
                if maintainer.email:
                    links.append(f"[Email](mailto:{maintainer.email})")
                
                if links:
                    markdown += " | ".join(links) + "\n\n"
                
                markdown += "---\n\n"
        
        return markdown
    
    def save_to_file(self, filename: str = "MAINTAINERS.md"):
        """Save maintainers to a markdown file."""
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(self.to_markdown())
        print(f"Maintainers list saved to {filename}")

def main():
    """Main entry point for the script."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Manage BloodHorn Bootloader maintainers')
    subparsers = parser.add_subparsers(dest='command', help='Command to execute')
    
    # List command
    list_parser = subparsers.add_parser('list', help='List maintainers')
    list_parser.add_argument('--role', type=str, help='Filter by role')
    
    # Get command
    get_parser = subparsers.add_parser('get', help='Get a specific maintainer')
    get_parser.add_argument('name', type=str, help='Name of the maintainer')
    
    # Export command
    export_parser = subparsers.add_parser('export', help='Export maintainers to a file')
    export_parser.add_argument('--output', '-o', type=str, default='MAINTAINERS.md',
                             help='Output file path (default: MAINTAINERS.md)')
    
    args = parser.parse_args()
    manager = MaintainersManager()
    
    if args.command == 'list':
        if args.role:
            try:
                role = Role(args.role)
                maintainers = manager.list_maintainers(role=role)
            except ValueError:
                print(f"Error: Invalid role. Available roles: {[r.value for r in Role]}")
                return 1
        else:
            maintainers = manager.list_maintainers()
        
        for maintainer in maintainers:
            print(f"{maintainer.name} - {maintainer.role.value}")
    
    elif args.command == 'get':
        maintainer = manager.get_maintainer(args.name)
        if maintainer:
            print(f"Name: {maintainer.name}")
            print(f"Role: {maintainer.role.value}")
            if maintainer.bio:
                print(f"Bio: {maintainer.bio}")
            if maintainer.email:
                print(f"Email: {maintainer.email}")
            if maintainer.github:
                print(f"GitHub: https://github.com/{maintainer.github}")
            if maintainer.twitter:
                print(f"Twitter: https://twitter.com/{maintainer.twitter}")
        else:
            print(f"Maintainer '{args.name}' not found.")
            return 1
    
    elif args.command == 'export':
        manager.save_to_file(args.output)
    
    else:
        parser.print_help()
        return 1
    
    return 0

if __name__ == "__main__":
    import sys
    sys.exit(main())
