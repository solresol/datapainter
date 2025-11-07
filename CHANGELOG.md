# Changelog

All notable changes to DataPainter will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive documentation suite (man page, architecture, building, testing, release process)
- Build scripts for Linux, macOS, Windows, and Haiku
- GitHub Actions workflows for CI and release automation
- Integration test suite using pytest and pyte

### Changed
- Enhanced CI workflow to include Python integration tests

### Fixed
- Tab navigation now works through all UI fields and buttons
- TUI rendering stability improved for integration tests

## [0.1.0] - 2024-11-07

### Added
- Initial release of DataPainter
- Interactive TUI for creating and editing 2D labeled datasets
- SQLite-backed data storage with metadata
- Viewport with zoom, pan, and coordinate transforms
- Point creation, deletion, and editing
- Undo/redo system with unsaved changes tracking
- Table view mode for tabular data editing
- CSV export functionality
- Random point generation with customizable distributions
- Tab navigation through UI elements
- Comprehensive CLI with 40+ options
- Complete test suite: 487 unit tests, 61 integration tests
- Full keyboard shortcut system
- Help overlay (press `?`)
- Support for multiple points at same screen position
- Validation of points within valid ranges
- Commit/discard changes workflow
- Database operations: create, delete, list tables
- Metadata management for tables

### Technical Features
- C++17 codebase with modern practices
- Google Test framework for unit testing
- pytest + pyte for TUI integration testing
- Terminal abstraction for cross-platform support
- Clean separation: Data, Logic, and UI layers
- RAII-based resource management
- Smart pointer usage throughout
- Comprehensive error handling

### Platforms
- Linux (Ubuntu, Debian, Fedora, Arch tested)
- macOS (12.x+ tested)
- Windows (experimental)

## Release Types

### Version Number Format

`MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD]`

- **MAJOR**: Breaking changes
- **MINOR**: New features (backward-compatible)
- **PATCH**: Bug fixes (backward-compatible)
- **PRERELEASE**: Alpha, beta, rc tags
- **BUILD**: Build metadata

### Change Categories

Each release documents changes in these categories:

- **Added**: New features
- **Changed**: Changes to existing functionality
- **Deprecated**: Features marked for removal
- **Removed**: Features removed in this version
- **Fixed**: Bug fixes
- **Security**: Security vulnerability fixes

## Links

- [GitHub Repository](https://github.com/solresol/datapainter)
- [Issue Tracker](https://github.com/solresol/datapainter/issues)
- [Release Notes](https://github.com/solresol/datapainter/releases)
