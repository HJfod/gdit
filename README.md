# GDit - The Automatic Megacollab Merging Tool

## Installation

### Development version

1. Download the project
2. Unzip in location of choice
3. Navigate to folder in command line
4. Run `./run.bat c`
5. You're done! :)

## TODO

 * Add `r` (Release) flag to run.bat (compiles the exe and puts it and all runtimes in a /release folder)
 * Fix that insanely odd bug with importing where it duplicates like 3 levels every time but it's always the same three levels and there's no correlation???
 * Add commits
 * Add merging
 * Add automatic commits / merging
 * Add server support
 * Add ability to merge from server

## Commands

Command template:

```
./gdit.exe command option <argument> [optional]
```

#### init

```
init <level>
```

 * Initializes a new gdit from the level. Replace spaces in the level name with underscores.

#### get

```
get [<gdit>]
```

 * Get the base level from a gdit. Send this file to your megacollab participants!

#### <path-to-.gditl>

```
<path-to-.gditl>
```

 * Import the file as a gdit part and start working on it.

#### setup [<setting> [<value>]]

```
setup [<setting> [<value>]]
```

 * Get / Set a setting.