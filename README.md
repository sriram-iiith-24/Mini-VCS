# AOS Assignment-4 [Mini Version Control System]

A simplified Git-like version control system implementation supporting basic repository level operations.

## Features

- Repository initialization
- File content hashing and storage 
- Object content viewing
- Directory tree management
- File staging
- Commit creation and history
- State restoration via checkout

## Build Instructions

### Compile using make
make -B


## Dependencies

- OpenSSL library (for SHA-1)
- ZSTD library (for compression)

## Commands

### Initialize Repository
* `./mygit init` ->
Creates new repository in current directory with .mygit structure.

### Hash Object
#### Calculate hash only
* `./mygit hash-object [file-name]`

#### Calculate and store object
* `./mygit hash-object -w [file-name]`
Computes SHA-1 hash of file content. -w flag stores the object.

### Cat File
#### View content
* `./mygit cat-file -p [hash]`

#### View type
* `./mygit cat-file -t [hash]`

#### View size  
* `./mygit cat-file -s [hash]`
Displays object information based on flag.

### Write Tree
* `./mygit write-tree`
Creates tree object from current directory state.

### List Tree
#### Detailed view
* `./mygit ls-tree [tree-hash]`

#### Names only
* `./mygit ls-tree --name-only [tree-hash]`
Shows contents of a tree object.

### Add Files
#### Add specific file
* `./mygit add [file]`

#### Add directory
* `./mygit add [directory]`

#### Add all files
* `./mygit add .`
Stages files for commit.

### Commit Changes
* `./mygit commit -m "commit message"`
Creates commit with staged changes.

### View History
* `./mygit log`
Shows commit history in reverse chronological order.

### Checkout
* `./mygit checkout [commit-hash]`
Restores repository state to specified commit.

## Implementation Details

### Object Storage
- All objects stored in .mygit/objects
- SHA-1 hash based directory structure
- ZSTD compression used
- Header contains type and size

### Object Types
- blob: File contents
- tree: Directory listings
- commit: Snapshots with metadata

### File Modes
- 100644: Regular file
- 100755: Executable file
- 40000: Directory
